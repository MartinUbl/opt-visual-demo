/**
 * OptVisualDemo
 *
 * Copyright (c) 2025-present, Martin Ubl
 * Distributed under the MIT license
 */

#include "DrawProxy.h"

#include "raylib.h"

std::map<NAppFont, TFont_Ptr> DrawProxy::mFonts;

void DrawProxy::InitFontsIfNeeded() {
	if (mFonts.empty()) {
		mFonts[NAppFont::Title] = std::make_shared<TFont>("assets/OpenSans.ttf", 32);
		mFonts[NAppFont::Subtitle] = std::make_shared<TFont>("assets/OpenSans.ttf", 24);
		mFonts[NAppFont::RegularText] = std::make_shared<TFont>("assets/OpenSans.ttf", 16);
	}
}

void DrawProxy::Text(const std::string& text, int x, int y, Color color, NAppFont appFont, int spacing) {
	InitFontsIfNeeded();

	auto it = mFonts.find(appFont);
	if (it != mFonts.end()) {
		DrawTextEx(*(it->second), text.c_str(), { (float)x, (float)y }, (float)it->second->GetBaseSize(), (float)spacing, color);
	}
}

void DrawProxy::MeasureText(const std::string& text, int& width, int& height, NAppFont appFont, int spacing) {
	InitFontsIfNeeded();
	auto it = mFonts.find(appFont);
	if (it != mFonts.end()) {
		Vector2 size = MeasureTextEx(*(it->second), text.c_str(), (float)it->second->GetBaseSize(), (float)spacing);
		width = (int)size.x;
		height = (int)size.y;
	}
	else {
		width = 0;
		height = 0;
	}
}
