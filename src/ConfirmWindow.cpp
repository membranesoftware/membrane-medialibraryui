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
#include "ConfirmWindow.h"

ConfirmWindow::ConfirmWindow (float windowWidth, float windowHeight)
: Panel ()
, isConfirmed (false)
, textFlow (NULL)
, confirmButton (NULL)
, cancelButton (NULL)
{
	classId = ClassId::ConfirmWindow;

	setPadding (UiConfiguration::instance->paddingSize, UiConfiguration::instance->paddingSize);
	setFillBg (true, UiConfiguration::instance->lightPrimaryColor);
	setFixedSize (true, windowWidth, windowHeight);

	textFlow = (TextFlow *) addWidget (new TextFlow (windowWidth - (widthPadding * 2.0f), UiConfiguration::BodyFont));
	textFlow->setTextColor (UiConfiguration::instance->inverseTextColor);

	confirmButton = (Button *) addWidget (new Button (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::OkButtonSprite)));
	confirmButton->zLevel = 1;
	confirmButton->setRaised (true, UiConfiguration::instance->raisedButtonBackgroundColor);
	confirmButton->mouseClickCallback = Widget::EventCallbackContext (ConfirmWindow::confirmButtonClicked, this);
	confirmButton->setMouseHoverTooltip (UiText::instance->getText (UiTextString::Confirm).capitalized ());

	cancelButton = (Button *) addWidget (new Button (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ExitButtonSprite)));
	cancelButton->zLevel = 1;
	cancelButton->setRaised (true, UiConfiguration::instance->raisedButtonBackgroundColor);
	cancelButton->mouseClickCallback = Widget::EventCallbackContext (ConfirmWindow::cancelButtonClicked, this);
	cancelButton->setMouseHoverTooltip (UiText::instance->getText (UiTextString::Cancel).capitalized ());
}

ConfirmWindow::~ConfirmWindow () {

}

bool ConfirmWindow::isWidgetType (Widget *widget) {
	return (widget && (widget->classId == ClassId::ConfirmWindow));
}

ConfirmWindow *ConfirmWindow::castWidget (Widget *widget) {
	return (ConfirmWindow::isWidgetType (widget) ? (ConfirmWindow *) widget : NULL);
}

void ConfirmWindow::refreshLayout () {
	float x, y, x2, y2;

	x = widthPadding;
	y = heightPadding;
	x2 = 0.0f;
	y2 = 0.0f;

	y += UiConfiguration::instance->marginSize;
	textFlow->flowDown (x, &y, &x2, &y2);
	cancelButton->flowRight (&x, y, &x2, &y2);
	confirmButton->flowRight (&x, y, &x2, &y2);

	y = height - heightPadding - cancelButton->height;
	x = width - widthPadding;
	confirmButton->flowLeft (&x, y);
	cancelButton->flowLeft (&x, y);
}

void ConfirmWindow::setText (const StdString &text) {
	textFlow->setText (text);
	refreshLayout ();
}

void ConfirmWindow::setButtonTooltips (const StdString &confirmTooltip, const StdString &cancelTooltip) {
	confirmButton->setMouseHoverTooltip (confirmTooltip);
	cancelButton->setMouseHoverTooltip (cancelTooltip);
}

void ConfirmWindow::confirmButtonClicked (void *windowPtr, Widget *widgetPtr) {
	ConfirmWindow *window;

	window = (ConfirmWindow *) windowPtr;
	window->isConfirmed = true;
	window->eventCallback (window->buttonClickCallback);
}

void ConfirmWindow::cancelButtonClicked (void *windowPtr, Widget *widgetPtr) {
	ConfirmWindow *window;

	window = (ConfirmWindow *) windowPtr;
	window->isConfirmed = false;
	window->eventCallback (window->buttonClickCallback);
}
