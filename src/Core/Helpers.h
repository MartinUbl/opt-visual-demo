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
