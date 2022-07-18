/*
* Copyright 2018-2022 Membrane Software <author@membranesoftware.com> https://membranesoftware.com
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice,
* this list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright notice,
* this list of conditions and the following disclaimer in the documentation
* and/or other materials provided with the distribution.
*
* 3. Neither the name of the copyright holder nor the names of its contributors
* may be used to endorse or promote products derived from this software without
* specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*/
#include "Config.h"
#include <stdlib.h>
#include "StdString.h"
#include "Log.h"
#include "App.h"
#include "Network.h"
#include "OsUtil.h"
#include "SystemInterface.h"
#include "HashMap.h"
#include "Label.h"
#include "Image.h"
#include "Ui.h"
#include "Panel.h"
#include "MediaLibraryWindow.h"
#include "ConfirmWindow.h"
#include "MainUi.h"

// Stage values
enum {
	NoStage = 0,
	Starting = 1,
	WaitingFindApplication = 2,
	FindApplicationComplete = 3,
	WaitingGetStatus1 = 4,
	ContactingStartedApplication = 5,
	WaitingGetStatus2 = 6,
	WaitingStopApplication = 7
};
const int ContactingStartedApplicationPeriod = 2400; // ms
const int ContactingStartedApplicationMaxTries = 30;
const int WaitingStopApplicationDelay = 1200;

MainUi::MainUi ()
: Ui ()
, stage (0)
, stageClock (0)
, stageCount (0)
, mediaLibraryWindow (NULL)
, processData (NULL)
{

}

MainUi::~MainUi () {
}

OsUtil::Result MainUi::doLoad () {
	Panel *bg, *bar;
	Image *image;
	Label *label;
	float y;

	bg = (Panel *) addWidget (new Panel ());
	bg->setFillBg (true, Color (0.08f, 0.08f, 0.08f));
	bg->setFixedSize (true, (float) App::instance->windowWidth, (float) App::instance->windowHeight);
	bg->zLevel = -1;

	bar = (Panel *) addWidget (new Panel ());
	bar->setFillBg (true, UiConfiguration::instance->darkPrimaryColor);
	bar->setPadding (UiConfiguration::instance->paddingSize, UiConfiguration::instance->paddingSize);

	image = (Image *) bar->addWidget (new Image (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::AppLogoSprite)));
	image->setDrawColor (true, UiConfiguration::instance->mediumSecondaryColor);

	label = (Label *) bar->addWidget (new Label (UiText::instance->getText (UiTextString::MembraneMediaLibrary), UiConfiguration::TitleFont, UiConfiguration::instance->inverseTextColor));
	label->zLevel = 1;

	bar->setLayout (Panel::HorizontalVcenteredLayout, (float) App::instance->windowWidth);
	bar->setFixedSize (true, (float) App::instance->windowWidth, bar->height);

	y = bar->height + UiConfiguration::instance->paddingSize;

	mediaLibraryWindow = new MediaLibraryWindow (((float) App::instance->windowWidth) * 0.9f, ((float) App::instance->windowHeight) - y - UiConfiguration::instance->paddingSize);
	mediaLibraryWindow->startClickCallback = Widget::EventCallbackContext (MainUi::startClicked, this);
	mediaLibraryWindow->stopClickCallback = Widget::EventCallbackContext (MainUi::stopClicked, this);
	addWidget (mediaLibraryWindow, (((float) App::instance->windowWidth) - mediaLibraryWindow->width) / 2.0f, y);

	mediaLibraryWindow->setDisplayState (MediaLibraryWindow::Waiting);
	setStage (Starting);

	return (OsUtil::Success);
}

void MainUi::doUnload () {
	setStage (NoStage);
	mediaLibraryWindow = NULL;
	confirmWindow.destroyAndClear ();
	darkenPanel.destroyAndClear ();
	if (processData) {
		OsUtil::terminateProcess (processData);
	}
}

void MainUi::doUpdate (int msElapsed) {
	confirmWindow.compact ();
	if (darkenPanel.widget) {
		darkenPanel.compact ();
		if (! confirmWindow.widget) {
			darkenPanel.destroyAndClear ();
		}
	}

	switch (stage) {
		case Starting: {
			setStage (WaitingFindApplication);
			retain ();
			if (! TaskGroup::instance->run (TaskGroup::RunContext (MainUi::findApplication, this))) {
				Log::debug ("Failed to execute task MainUi::findApplication");
				release ();
			}
			break;
		}
		case FindApplicationComplete: {
			setStage (WaitingGetStatus1);
			invokeCommand (App::instance->createCommand (SystemInterface::Command_GetStatus));
			break;
		}
		case ContactingStartedApplication: {
			if (stageCount > ContactingStartedApplicationMaxTries) {
				mediaLibraryWindow->setDisplayState (MediaLibraryWindow::StartError);
				if (processData) {
					OsUtil::terminateProcess (processData);
				}
				setStage (NoStage);
				break;
			}
			stageClock += msElapsed;
			if (stageClock >= ContactingStartedApplicationPeriod) {
				invokeCommand (App::instance->createCommand (SystemInterface::Command_GetStatus));
				setStage (WaitingGetStatus2, 0, stageCount);
			}
			break;
		}
		case WaitingStopApplication: {
			if (stageClock < WaitingStopApplicationDelay) {
				stageClock += msElapsed;
			}
			if (stageClock >= WaitingStopApplicationDelay) {
				if (! processData) {
					mediaLibraryWindow->setDisplayState (MediaLibraryWindow::Stopped);
					setStage (NoStage);
				}
			}
			break;
		}
	}
}

bool MainUi::doProcessWindowCloseEvent () {
	ConfirmWindow *confirm;
	Panel *panel;

	if (! processData) {
		return (false);
	}
	if (confirmWindow.widget) {
		if (processData) {
			OsUtil::terminateProcess (processData);
		}
		return (false);
	}

	panel = (Panel *) addWidget (new Panel ());
	panel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, 0.0f));
	panel->bgColor.translate (0.0f, 0.0f, 0.0f, UiConfiguration::instance->overlayWindowAlpha, UiConfiguration::instance->backgroundCrossFadeDuration);
	panel->setFixedSize (true, App::instance->rootPanel->width, App::instance->rootPanel->height);
	panel->zLevel = App::instance->rootPanel->maxWidgetZLevel + 1;
	darkenPanel.assign (panel);

	confirm = (ConfirmWindow *) addWidget (new ConfirmWindow (App::instance->rootPanel->width * 0.6f, App::instance->rootPanel->height * 0.6f));
	confirm->buttonClickCallback = Widget::EventCallbackContext (MainUi::confirmWindowButtonClicked, this);
	confirm->setText (UiText::instance->getText (UiTextString::ShutdownConfirmText));
	confirm->setButtonTooltips (UiText::instance->getText (UiTextString::ShutdownConfirmTooltip), UiText::instance->getText (UiTextString::ShutdownCancelTooltip));
	confirm->zLevel = App::instance->rootPanel->maxWidgetZLevel + 2;
	confirm->position.assign ((App::instance->rootPanel->width - confirm->width) / 2.0f, (App::instance->rootPanel->height - confirm->height) / 2.0f);
	confirmWindow.assign (confirm);

	return (true);
}

void MainUi::confirmWindowButtonClicked (void *uiPtr, Widget *widgetPtr) {
	MainUi *ui;
	ConfirmWindow *confirm;

	ui = (MainUi *) uiPtr;
	confirm = (ConfirmWindow *) widgetPtr;
	if (confirm->isConfirmed) {
		if (ui->processData) {
			OsUtil::terminateProcess (ui->processData);
		}
		App::instance->shutdown ();
	}
	else {
		confirm->isDestroyed = true;
	}
}

void MainUi::setStage (int targetStage, int targetStageClock, int targetStageCount) {
	stage = targetStage;
	stageClock = targetStageClock;
	stageCount = targetStageCount;
}

void MainUi::findApplication (void *uiPtr) {
	MainUi *ui;
	HashMap map;
	StdString val;
	OsUtil::Result result;

	ui = (MainUi *) uiPtr;
	if (! ui->isLoaded) {
		ui->release ();
		return;
	}
	result = map.read (OsUtil::getAppendPath (StdString ("conf"), StdString ("build.conf")), true);
	if (result != OsUtil::Success) {
		ui->mediaLibraryWindow->setDisplayState (MediaLibraryWindow::Uninstalled);
	}
	else {
		val = map.find ("Version", "");
		if (val.empty ()) {
			ui->mediaLibraryWindow->setDisplayState (MediaLibraryWindow::Uninstalled);
		}
		else {
			ui->mediaLibraryWindow->applicationVersion.assign (val);
			ui->mediaLibraryWindow->setDisplayState (MediaLibraryWindow::Contacting);
			ui->setStage (FindApplicationComplete);
		}
	}
	ui->release ();
}

int MainUi::invokeCommand (Json *command) {
	if (! command) {
		return (OsUtil::InvalidParamError);
	}
	retain ();
	Network::instance->sendHttpPost (StdString::createSprintf ("%s://%s:%i%s", App::instance->isHttpsEnabled ? "https" : "http", Network::LocalhostAddress.c_str (), SystemInterface::Constant_DefaultTcpPort1, SystemInterface::Constant_DefaultInvokePath), command->toString (), Network::HttpRequestCallbackContext (MainUi::httpRequestComplete, this));
	delete (command);
	return (OsUtil::Success);
}

void MainUi::httpRequestComplete (void *uiPtr, const StdString &targetUrl, int statusCode, SharedBuffer *responseData) {
	MainUi *ui;
	Json *cmd;
	StdString respdata;
	int cmdid;

	ui = (MainUi *) uiPtr;
	if (! ui->isLoaded) {
		ui->release ();
		return;
	}

	switch (ui->stage) {
		case WaitingGetStatus1: {
			if (statusCode == Network::HttpUnauthorizedCode) {
				ui->mediaLibraryWindow->setDisplayState (MediaLibraryWindow::Running);
				break;
			}
			if (!((statusCode == Network::HttpOkCode) && responseData && (responseData->length > 0))) {
				ui->mediaLibraryWindow->setDisplayState (MediaLibraryWindow::Stopped);
				break;
			}
			respdata.assignBuffer (responseData);
			if (! SystemInterface::instance->parseCommand (respdata, &cmd)) {
				ui->mediaLibraryWindow->setDisplayState (MediaLibraryWindow::Stopped);
				break;
			}
			cmdid = SystemInterface::instance->getCommandId (cmd);
			if ((cmdid == SystemInterface::CommandId_AuthorizationRequired) || (cmdid == SystemInterface::CommandId_AgentStatus)) {
				ui->mediaLibraryWindow->setDisplayState (MediaLibraryWindow::Running);
			}
			else {
				ui->mediaLibraryWindow->setDisplayState (MediaLibraryWindow::Stopped);
			}
			delete (cmd);
			break;
		}
		case WaitingGetStatus2: {
			if (statusCode == Network::HttpUnauthorizedCode) {
				ui->mediaLibraryWindow->setDisplayState (MediaLibraryWindow::Running);
				ui->setStage (NoStage);
				break;
			}
			if (!((statusCode == Network::HttpOkCode) && responseData && (responseData->length > 0))) {
				ui->setStage (ContactingStartedApplication, 0, ui->stageCount + 1);
				break;
			}
			respdata.assignBuffer (responseData);
			if (! SystemInterface::instance->parseCommand (respdata, &cmd)) {
				ui->setStage (ContactingStartedApplication, 0, ui->stageCount + 1);
				break;
			}
			cmdid = SystemInterface::instance->getCommandId (cmd);
			if ((cmdid == SystemInterface::CommandId_AuthorizationRequired) || (cmdid == SystemInterface::CommandId_AgentStatus)) {
				ui->mediaLibraryWindow->setDisplayState (MediaLibraryWindow::Running);
			}
			else {
				ui->mediaLibraryWindow->setDisplayState (MediaLibraryWindow::StartError);
			}
			ui->setStage (NoStage);
			delete (cmd);
			break;
		}
	}

	ui->release ();
}

void MainUi::startClicked (void *uiPtr, Widget *widgetPtr) {
	MainUi *ui;
	void *proc;

	ui = (MainUi *) uiPtr;
	if (ui->processData) {
		return;
	}
	if (OsUtil::setEnvValue (StdString ("DATA_DIRECTORY"), OsUtil::getUserDataPath ()) != OsUtil::Success) {
		Log::err ("Failed to set process environment variable");
		return;
	}
	proc = OsUtil::executeProcess (StdString ("node"), StdString ("src/Main.js"));
	if (! proc) {
		Log::err ("Failed to launch server process");
		return;
	}
	ui->processData = proc;
	ui->mediaLibraryWindow->setDisplayState (MediaLibraryWindow::ContactingStartedApplication);
	ui->setStage (ContactingStartedApplication);
	ui->retain ();
	if (! TaskGroup::instance->run (TaskGroup::RunContext (MainUi::waitApplication, ui))) {
		Log::debug ("Failed to execute task MainUi::waitApplication");
		ui->release ();
	}
}

void MainUi::stopClicked (void *uiPtr, Widget *widgetPtr) {
	MainUi *ui;

	ui = (MainUi *) uiPtr;
	if (! ui->processData) {
		return;
	}
	OsUtil::terminateProcess (ui->processData);
	ui->mediaLibraryWindow->setDisplayState (MediaLibraryWindow::WaitingStopApplication);
	ui->setStage (WaitingStopApplication);
}

void MainUi::waitApplication (void *uiPtr) {
	MainUi *ui;

	ui = (MainUi *) uiPtr;
	if (ui->processData) {
		OsUtil::waitProcess (ui->processData);
		OsUtil::freeProcessData (ui->processData);
		ui->processData = NULL;
	}
	ui->release ();
}
