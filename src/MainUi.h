/*
* Copyright 2018-2021 Membrane Software <author@membranesoftware.com> https://membranesoftware.com
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
// UI that shows the main display

#ifndef MAIN_UI_H
#define MAIN_UI_H

#include "WidgetHandle.h"
#include "MediaLibraryWindow.h"
#include "Ui.h"

class MainUi : public Ui {
public:
	MainUi ();
	~MainUi ();

protected:
	// Load subclass-specific resources and return a result value
	int doLoad ();

	// Unload subclass-specific resources
	void doUnload ();

	// Update subclass-specific interface state as appropriate for an elapsed millisecond time period
	void doUpdate (int msElapsed);

	// Execute subclass-specific actions appropriate for a received window close event and return a boolean value indicating if the event was consumed and should no longer be processed
	bool doProcessWindowCloseEvent ();

private:
	// Find the server application
	static void findApplication (void *uiPtr);

	// Wait for the application process to exit
	static void waitApplication (void *uiPtr);

	// Callback functions
	static void httpRequestComplete (void *uiPtr, const StdString &targetUrl, int statusCode, SharedBuffer *responseData);
	static void startClicked (void *uiPtr, Widget *widgetPtr);
	static void stopClicked (void *uiPtr, Widget *widgetPtr);
	static void confirmWindowButtonClicked (void *uiPtr, Widget *widgetPtr);

	// Set the Ui stage
	void setStage (int targetStage, int targetStageClock = 0, int targetStageCount = 0);

	// Invoke a command on the localhost agent
	int invokeCommand (Json *command);

	int stage;
	int stageClock;
	int stageCount;
	MediaLibraryWindow *mediaLibraryWindow;
	WidgetHandle confirmWindow;
	WidgetHandle darkenPanel;
	void *processData;
};

#endif
