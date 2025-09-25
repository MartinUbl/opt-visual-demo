/**
 * OptVisualDemo
 *
 * Copyright (c) 2025-present, Martin Ubl
 * Distributed under the MIT license
 */

#pragma once

#include <memory>

#include "raylib.h"
#include <string>
#include "DrawProxy.h"

/**
 * RAII guard for window initialization and closing
 */
class TWindow_Guard {
	public:
		TWindow_Guard(int width, int height, const char* title) {
			InitWindow(width, height, title);
		}
		virtual ~TWindow_Guard() {
			CloseWindow();
		}
};

/**
 * RAII guard for BeginDrawing/EndDrawing
 */
class TDrawing_Guard {
	public:
		TDrawing_Guard() {
			BeginDrawing();
		}
		virtual ~TDrawing_Guard() {
			EndDrawing();
		}
};

/**
 * A simple button UI element
 */
class TSimple_Button {
	public:
		TSimple_Button(int x, int y, int width, int height, const std::string& text, NAppFont appFont = NAppFont::RegularText)
			: mRect{ static_cast<float>(x), static_cast<float>(y), static_cast<float>(width), static_cast<float>(height) }, mText(text), mAppFont{ appFont } {
		}

		// Render the button, returns true if it was clicked
		bool Render();

	private:
		Rectangle mRect;
		std::string mText;
		NAppFont mAppFont = NAppFont::RegularText;
};

enum class NInput_Mask {
	None,
	Numeric
};

struct TSimple_Input_State {
	bool isActive = false;
	std::string text;
};

/**
 * A simple input box UI element
 */
class TSimple_Input {
	public:
		TSimple_Input(int x, int y, int width, int height, const std::string& label, TSimple_Input_State& state, NAppFont appFont = NAppFont::RegularText, NInput_Mask inMask = NInput_Mask::None, int maxLength = -1)
			: mRect{ static_cast<float>(x), static_cast<float>(y), static_cast<float>(width), static_cast<float>(height) }, mLabel(label), mState{ state }, mAppFont{ appFont }, mInMask{ inMask }, mMaxLength{ maxLength } {
		}

		bool Render();

	private:
		Rectangle mRect;
		std::string mLabel;

		NAppFont mAppFont = NAppFont::RegularText;
		NInput_Mask mInMask = NInput_Mask::None;

		int mMaxLength = -1; // -1 = no limit

		TSimple_Input_State& mState;
};
