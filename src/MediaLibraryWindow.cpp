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
#include "ClassId.h"
#include "StdString.h"
#include "App.h"
#include "UiText.h"
#include "UiConfiguration.h"
#include "Widget.h"
#include "Color.h"
#include "Image.h"
#include "Sprite.h"
#include "Json.h"
#include "Panel.h"
#include "Label.h"
#include "TextFlow.h"
#include "ProgressBar.h"
#include "StatsWindow.h"
#include "HyperlinkWindow.h"
#include "MediaLibraryWindow.h"

MediaLibraryWindow::MediaLibraryWindow (float windowWidth, float windowHeight)
: Panel ()
, displayState (MediaLibraryWindow::Waiting)
, iconImage (NULL)
, statusLabel (NULL)
, dividerPanel (NULL)
, statsWindow (NULL)
, linkIconImage (NULL)
, linkWindow (NULL)
, textFlow (NULL)
, progressBar (NULL)
, progressBarLabel (NULL)
, startButton (NULL)
, stopButton (NULL)
{
	classId = ClassId::MediaLibraryWindow;

	setPadding (UiConfiguration::instance->paddingSize, UiConfiguration::instance->paddingSize);
	setCornerRadius (UiConfiguration::instance->cornerRadius);
	setFillBg (true, UiConfiguration::instance->mediumBackgroundColor);
	setFixedSize (true, windowWidth, windowHeight);

	iconImage = (Image *) addWidget (new Image (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::LargeMediaIconSprite)));
	statusLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::HeadlineFont, UiConfiguration::instance->primaryTextColor));

	dividerPanel = (Panel *) addWidget (new Panel ());
	dividerPanel->setFillBg (true, UiConfiguration::instance->dividerColor);
	dividerPanel->setFixedSize (true, 1.0f, UiConfiguration::instance->headlineDividerLineWidth);
	dividerPanel->isPanelSizeClipEnabled = true;

	statsWindow = (StatsWindow *) addWidget (new StatsWindow ());
	statsWindow->isEmptyItemValueIgnored = true;
	statsWindow->isVisible = false;

	linkIconImage = (Image *) addWidget (new Image (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::WebLinkIconSprite)));
	linkIconImage->isVisible = false;

	linkWindow = (HyperlinkWindow *) addWidget (new HyperlinkWindow (StdString (""), StdString (""), UiConfiguration::CaptionFont));
	linkWindow->isVisible = false;

	textFlow = (TextFlow *) addWidget (new TextFlow (windowWidth - (widthPadding * 2.0f), UiConfiguration::BodyFont));
	textFlow->isVisible = false;

	progressBar = (ProgressBar *) addWidget (new ProgressBar (windowWidth * 0.5f, UiConfiguration::instance->progressBarHeight));
	progressBar->isVisible = false;
	progressBarLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::BodyFont, UiConfiguration::instance->primaryTextColor));
	progressBarLabel->isVisible = false;

	startButton = (Button *) addWidget (new Button (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::OkButtonSprite)));
	startButton->zLevel = 1;
	startButton->setRaised (true, UiConfiguration::instance->raisedButtonBackgroundColor);
	startButton->mouseClickCallback = Widget::EventCallbackContext (MediaLibraryWindow::startButtonClicked, this);
	startButton->setMouseHoverTooltip (UiText::instance->getText (UiTextString::StartButtonTooltip));
	startButton->isVisible = false;

	stopButton = (Button *) addWidget (new Button (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ExitButtonSprite)));
	stopButton->zLevel = 1;
	stopButton->setRaised (true, UiConfiguration::instance->raisedButtonBackgroundColor);
	stopButton->mouseClickCallback = Widget::EventCallbackContext (MediaLibraryWindow::stopButtonClicked, this);
	stopButton->setMouseHoverTooltip (UiText::instance->getText (UiTextString::StopButtonTooltip));
	stopButton->isVisible = false;

	refreshLayout ();
}

MediaLibraryWindow::~MediaLibraryWindow () {

}

bool MediaLibraryWindow::isWidgetType (Widget *widget) {
	return (widget && (widget->classId == ClassId::MediaLibraryWindow));
}

MediaLibraryWindow *MediaLibraryWindow::castWidget (Widget *widget) {
	return (MediaLibraryWindow::isWidgetType (widget) ? (MediaLibraryWindow *) widget : NULL);
}

void MediaLibraryWindow::refreshLayout () {
	float x, y, x0, y0, x2, y2;

	x = widthPadding;
	y = heightPadding;
	x0 = x;
	y0 = y;
	x2 = 0.0f;
	y2 = 0.0f;

	iconImage->flowRight (&x, y, &x2, &y2);
	statusLabel->flowDown (x, &y, &x2, &y2);
	iconImage->centerVertical (y0, y2);
	statusLabel->centerVertical (y0, y2);

	x = x0;
	y = y2 + UiConfiguration::instance->marginSize;
	x2 = 0.0f;
	dividerPanel->setFixedSize (true, width, UiConfiguration::instance->headlineDividerLineWidth);
	dividerPanel->flowDown (0.0f, &y, &x2, &y2);

	x = x0;
	y0 = y;
	y = y2 + UiConfiguration::instance->marginSize;
	switch (displayState) {
		case MediaLibraryWindow::Uninstalled: {
			textFlow->flowDown (x, &y, &x2, &y2);
			y += UiConfiguration::instance->marginSize;
			y0 = y;
			linkIconImage->flowRight (&x, y, &x2, &y2);
			linkWindow->flowDown (x, &y, &x2, &y2);
			x = width - widthPadding - UiConfiguration::instance->marginSize;
			linkWindow->flowLeft (&x);
			linkIconImage->flowLeft (&x);
			linkWindow->centerVertical (y0, y2);
			linkIconImage->centerVertical (y0, y2);
			break;
		}
		case MediaLibraryWindow::Waiting:
		case MediaLibraryWindow::Contacting:
		case MediaLibraryWindow::ContactingStartedApplication:
		case MediaLibraryWindow::WaitingStopApplication: {
			x = width - widthPadding;
			y = height - heightPadding;
			y -= (UiConfiguration::instance->marginSize * 2.0f);
			progressBar->flowUp (x - progressBar->width, &y);
			progressBarLabel->flowUp (x - progressBarLabel->width, &y);
			break;
		}
		case MediaLibraryWindow::Stopped:
		case MediaLibraryWindow::Running:
		case MediaLibraryWindow::StartError: {
			textFlow->flowDown (x, &y, &x2, &y2);
			if (linkWindow->isVisible) {
				y0 = y;
				linkIconImage->flowRight (&x, y, &x2, &y2);
				linkWindow->flowDown (x, &y, &x2, &y2);
				x = width - widthPadding - UiConfiguration::instance->marginSize;
				linkWindow->flowLeft (&x);
				linkIconImage->flowLeft (&x);
				linkWindow->centerVertical (y0, y2);
				linkIconImage->centerVertical (y0, y2);
			}

			x = width - widthPadding;
			y = height - heightPadding - UiConfiguration::instance->marginSize;
			startButton->flowLeft (&x, y - startButton->height);
			stopButton->flowLeft (&x, y - stopButton->height);

			x = widthPadding;
			y = startButton->position.y + ((startButton->height - statsWindow->height) / 2.0f);
			statsWindow->position.assign (x, y);
			break;
		}
	}
}

void MediaLibraryWindow::setDisplayState (MediaLibraryWindow::DisplayState state) {
	displayState = state;
	statsWindow->setItem (UiText::instance->getText (UiTextString::Version).capitalized (), applicationVersion);
	switch (displayState) {
		case MediaLibraryWindow::Waiting: {
			statusLabel->setText (UiText::instance->getText (UiTextString::ScanningApplication).capitalized ());
			progressBarLabel->setText (UiText::instance->getText (UiTextString::MediaLibraryWaitingText));
			progressBar->setIndeterminate (true);
			statsWindow->isVisible = false;
			linkIconImage->isVisible = false;
			linkWindow->isVisible = false;
			textFlow->isVisible = false;
			progressBar->isVisible = true;
			progressBarLabel->isVisible = true;
			startButton->isVisible = false;
			stopButton->isVisible = false;
			break;
		}
		case MediaLibraryWindow::Contacting: {
			statusLabel->setText (UiText::instance->getText (UiTextString::Installed).capitalized ());
			progressBarLabel->setText (UiText::instance->getText (UiTextString::MediaLibraryContactingText));
			progressBar->setIndeterminate (true);
			statsWindow->isVisible = false;
			linkIconImage->isVisible = false;
			linkWindow->isVisible = false;
			textFlow->isVisible = false;
			progressBar->isVisible = true;
			progressBarLabel->isVisible = true;
			startButton->isVisible = false;
			stopButton->isVisible = false;
			break;
		}
		case MediaLibraryWindow::ContactingStartedApplication: {
			statusLabel->setText (UiText::instance->getText (UiTextString::Starting).capitalized ());
			progressBarLabel->setText (UiText::instance->getText (UiTextString::MediaLibraryContactingStartedApplicationText));
			progressBar->setIndeterminate (true);
			statsWindow->isVisible = false;
			linkIconImage->isVisible = false;
			linkWindow->isVisible = false;
			textFlow->isVisible = false;
			progressBar->isVisible = true;
			progressBarLabel->isVisible = true;
			startButton->isVisible = false;
			stopButton->isVisible = false;
			break;
		}
		case MediaLibraryWindow::WaitingStopApplication: {
			statusLabel->setText (UiText::instance->getText (UiTextString::Stopping).capitalized ());
			progressBarLabel->setText (UiText::instance->getText (UiTextString::MediaLibraryWaitingStopApplicationText));
			progressBar->setIndeterminate (true);
			statsWindow->isVisible = false;
			linkIconImage->isVisible = false;
			linkWindow->isVisible = false;
			textFlow->isVisible = false;
			progressBar->isVisible = true;
			progressBarLabel->isVisible = true;
			startButton->isVisible = false;
			stopButton->isVisible = false;
			break;
		}
		case MediaLibraryWindow::StartError: {
			statusLabel->setText (UiText::instance->getText (UiTextString::StartupError).capitalized ());
			textFlow->setText (UiText::instance->getText (UiTextString::MediaLibraryStartErrorText));
			statsWindow->isVisible = true;
			linkIconImage->isVisible = false;
			linkWindow->isVisible = false;
			textFlow->isVisible = true;
			progressBar->isVisible = false;
			progressBarLabel->isVisible = false;
			startButton->setDisabled (false);
			startButton->isVisible = true;
			stopButton->setDisabled (true);
			stopButton->isVisible = true;
			break;
		}
		case MediaLibraryWindow::Uninstalled: {
			statusLabel->setText (UiText::instance->getText (UiTextString::NotInstalled).capitalized ());
			textFlow->setText (UiText::instance->getText (UiTextString::MediaLibraryUninstalledText));
			linkWindow->setLink (UiText::instance->getText (UiTextString::DownloadLinkText), StdString::createSprintf ("%smembrane-media-library", App::ServerUrl.c_str ()));
			statsWindow->isVisible = false;
			linkIconImage->isVisible = true;
			linkWindow->isVisible = true;
			textFlow->isVisible = true;
			progressBar->isVisible = false;
			progressBarLabel->isVisible = false;
			startButton->isVisible = false;
			stopButton->isVisible = false;
			break;
		}
		case MediaLibraryWindow::Stopped: {
			statusLabel->setText (UiText::instance->getText (UiTextString::NotRunning).capitalized ());
			textFlow->setText (UiText::instance->getText (UiTextString::MediaLibraryStoppedText));
			statsWindow->isVisible = true;
			linkIconImage->isVisible = false;
			linkWindow->isVisible = false;
			textFlow->isVisible = true;
			progressBar->isVisible = false;
			progressBarLabel->isVisible = false;
			startButton->setDisabled (false);
			startButton->isVisible = true;
			stopButton->setDisabled (true);
			stopButton->isVisible = true;
			break;
		}
		case MediaLibraryWindow::Running: {
			statusLabel->setText (UiText::instance->getText (UiTextString::Running).capitalized ());
			textFlow->setText (UiText::instance->getText (UiTextString::MediaLibraryRunningText));
			linkWindow->setLink (UiText::instance->getText (UiTextString::DownloadLinkText), StdString::createSprintf ("%smembrane-control", App::ServerUrl.c_str ()));
			statsWindow->isVisible = true;
			linkIconImage->isVisible = true;
			linkWindow->isVisible = true;
			textFlow->isVisible = true;
			progressBar->isVisible = false;
			progressBarLabel->isVisible = false;
			startButton->setDisabled (true);
			startButton->isVisible = true;
			stopButton->setDisabled (false);
			stopButton->isVisible = true;
			break;
		}
	}

	refreshLayout ();
}

void MediaLibraryWindow::startButtonClicked (void *windowPtr, Widget *widgetPtr) {
	((MediaLibraryWindow *) windowPtr)->eventCallback (((MediaLibraryWindow *) windowPtr)->startClickCallback);
}

void MediaLibraryWindow::stopButtonClicked (void *windowPtr, Widget *widgetPtr) {
	((MediaLibraryWindow *) windowPtr)->eventCallback (((MediaLibraryWindow *) windowPtr)->stopClickCallback);
}
