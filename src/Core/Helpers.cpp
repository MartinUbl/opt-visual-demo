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

bool TSimple_Input::Render() {
	bool retval = false;
	// label
	int labelWidth = 0;
	int labelHeight = 0;
	DrawProxy::MeasureText(mLabel, labelWidth, labelHeight, mAppFont);
	DrawProxy::Text(mLabel, (int)mRect.x, (int)(mRect.y - labelHeight - 5), DARKGRAY, mAppFont);
	// box
	Color col = LIGHTGRAY;
	if (mState.isActive) {
		col = GREEN;
	}
	if (CheckCollisionPointRec(GetMousePosition(), mRect)) {
		if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
			mState.isActive = true;
		}
	}
	else {
		if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
			mState.isActive = false;
		}
	}
	if (mState.isActive) {

		if (mMaxLength == -1 || mState.text.size() < static_cast<size_t>(mMaxLength)) {
			if (mInMask == NInput_Mask::Numeric) {
				for (int key = 0; key <= 9; key++) {
					if (IsKeyPressed(KEY_ZERO + key)) {
						mState.text += static_cast<char>('0' + key);
						retval = true;
					}

					if (IsKeyPressed(KEY_KP_0 + key)) {
						mState.text += static_cast<char>('0' + key);
						retval = true;
					}
				}
			}
			else {
				for (int key = 32; key < 128; key++) {
					if (IsKeyPressed(key)) {
						mState.text += static_cast<char>(key);
						retval = true;
					}
				}
			}
		}

		if (IsKeyPressed(KEY_BACKSPACE) && mState.text.size() > 0) {
			mState.text.pop_back();
			retval = true;
		}
	}
	DrawRectangleRec(mRect, col);
	int textWidth = 0;
	int textHeight = 0;
	DrawProxy::MeasureText(mState.text + "_", textWidth, textHeight, mAppFont);
	DrawProxy::Text(mState.text + (mState.isActive ? "_" : ""), (int)(mRect.x + 5), (int)(mRect.y + (mRect.height - textHeight) / 2), DARKGRAY, mAppFont);
	return retval;
}
