/**
 * OptVisualDemo
 *
 * Copyright (c) 2025-present, Martin Ubl
 * Distributed under the MIT license
 */

#include "Helpers.h"
#include "DrawProxy.h"

bool TSimple_Button::Render() {
	bool retval = false;
	Color col = LIGHTGRAY;
	Color textColor = DARKGRAY;
	if (CheckCollisionPointRec(GetMousePosition(), mRect)) {
		col = DARKGREEN;
		textColor = RAYWHITE;
		if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
			retval = true;
		}
	}

	int textWidth = 0;
	int textHeight = 0;

	DrawProxy::MeasureText(mText, textWidth, textHeight, mAppFont);

	DrawRectangleRec(mRect, col);
	DrawProxy::Text(mText, (int)(mRect.x + (mRect.width - textWidth) / 2), (int)(mRect.y + (mRect.height - textHeight) / 2), textColor, mAppFont);
	return retval;
}
