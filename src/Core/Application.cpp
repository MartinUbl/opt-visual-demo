/**
 * OptVisualDemo
 *
 * Copyright (c) 2025-present, Martin Ubl
 * Distributed under the MIT license
 */

#include "Application.h"
#include "Helpers.h"
#include "Stage.h"

#include "raylib.h"

bool Application::Init(int argc, char** argv) {
	// for future use
	return true;
}

int Application::Run() {

	// antialiasing
	SetConfigFlags(FLAG_MSAA_4X_HINT);

	TWindow_Guard window(1200, 800, "OptVisualDemo");

	TFont_Ptr fnt = std::make_shared<TFont>("assets/OpenSans.ttf");

	// main loop
	while (!WindowShouldClose())
	{
		// handle stage changes
		if (mRequested_Stage != NStage::None) {
			if (mStage) {
				mStage->On_Leave();
				mStage.reset();
			}

			switch (mRequested_Stage) {
				case NStage::Menu:
					mStage = std::make_unique<MenuStage>();
					break;
				case NStage::Experiment:
					mStage = std::make_unique<ExperimentStage>(mRequested_Experiment);
					break;
				default:
					break;
			}

			if (mStage) {
				mStage->On_Enter();
				mCurrent_Stage = mRequested_Stage;
			}
			else {
				mCurrent_Stage = NStage::None;
			}

			mRequested_Stage = NStage::None;
		}

		TDrawing_Guard draw;

		ClearBackground(RAYWHITE);

		if (mStage) {
			mStage->On_Update(GetFrameTime());
			mStage->On_Render();
		}
	}

	if (mStage) {
		mStage->On_Leave();
		mStage.reset();
	}

	return 0;
}
