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
// Panel that contains elements representing a Membrane Media Library server on a card view

#ifndef MEDIA_LIBRARY_WINDOW_H
#define MEDIA_LIBRARY_WINDOW_H

#include "StdString.h"
#include "Widget.h"
#include "Image.h"
#include "Label.h"
#include "TextFlow.h"
#include "Button.h"
#include "ProgressBar.h"
#include "HyperlinkWindow.h"
#include "StatsWindow.h"
#include "Panel.h"

class MediaLibraryWindow : public Panel {
public:
	enum DisplayState {
		Waiting = 0,
		Contacting = 1,
		Uninstalled = 2,
		Stopped = 3,
		Running = 4,
		ContactingStartedApplication = 5,
		StartError = 6,
		WaitingStopApplication = 7
	};

	MediaLibraryWindow (float windowWidth, float windowHeight);
	virtual ~MediaLibraryWindow ();

	// Read-write data members
	Widget::EventCallbackContext startClickCallback;
	Widget::EventCallbackContext stopClickCallback;
	StdString applicationVersion;

	// Read-only data members
	MediaLibraryWindow::DisplayState displayState;

	// Set the display state of the window
	void setDisplayState (MediaLibraryWindow::DisplayState state);

	// Return a boolean value indicating if the provided Widget is a member of this class
	static bool isWidgetType (Widget *widget);

	// Return a typecasted pointer to the provided widget, or NULL if the widget does not appear to be of the correct type
	static MediaLibraryWindow *castWidget (Widget *widget);

protected:
	// Reset the panel's widget layout as appropriate for its content and configuration
	void refreshLayout ();

private:
	// Callback functions
	static void startButtonClicked (void *windowPtr, Widget *widgetPtr);
	static void stopButtonClicked (void *windowPtr, Widget *widgetPtr);

	Image *iconImage;
	Label *statusLabel;
	Panel *dividerPanel;
	StatsWindow *statsWindow;
	Image *linkIconImage;
	HyperlinkWindow *linkWindow;
	TextFlow *textFlow;
	ProgressBar *progressBar;
	Label *progressBarLabel;
	Button *startButton;
	Button *stopButton;
};

#endif
