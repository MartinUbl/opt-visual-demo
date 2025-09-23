/**
 * OptVisualDemo
 *
 * Copyright (c) 2025-present, Martin Ubl
 * Distributed under the MIT license
 */

#pragma once

#include <string>
#include <map>
#include <memory>
#include "raylib.h"

 // application fonts
enum class NAppFont {
	Title,
	Subtitle,
	RegularText
};

/**
 * class wrapping a raylib Font with loading/unloading (RAII-style)
 */
class TFont {
	private:
		Font font;

	public:
		TFont(const char* fontFileName) {
			font = LoadFont(fontFileName);
		}

		TFont(const char* fontFileName, int fontSize) {
			font = LoadFontEx(fontFileName, fontSize, 0, 0);
		}

		virtual ~TFont() {
			UnloadFont(font);
		}

		operator Font& () {
			return font;
		}

		int GetBaseSize() const {
			return font.baseSize;
		}
};

using TFont_Ptr = std::shared_ptr<TFont>;

/**
 * Drawing proxy for text rendering, maintains fonts
 */
class DrawProxy {
	public:
		static void Text(const std::string& text, int x, int y, Color color, NAppFont appFont = NAppFont::RegularText, int spacing = 0);
		static void MeasureText(const std::string& text, int& width, int& height, NAppFont appFont = NAppFont::RegularText, int spacing = 0);

	private:
		static std::map<NAppFont, TFont_Ptr> mFonts;

		static void InitFontsIfNeeded();
};
