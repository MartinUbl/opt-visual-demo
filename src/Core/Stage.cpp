/**
 * OptVisualDemo
 *
 * Copyright (c) 2025-present, Martin Ubl
 * Distributed under the MIT license
 */

#include "Stage.h"
#include "DrawProxy.h"
#include "Helpers.h"

#include "Application.h"

#include "../registration.h"
#include "Experiment.h"

#include "raylib.h"

bool MenuStage::On_Enter() {
	return true;
}

bool MenuStage::On_Leave() {
	return true;
}

bool MenuStage::On_Update(float delta_time) {
	return true;
}

bool MenuStage::On_Render() {

	DrawProxy::Text("Optimizers Demo", 10, 10, DARKGRAY, NAppFont::Title);
	DrawProxy::Text("Select experiment", 10, 40, DARKGRAY, NAppFont::Subtitle);

	const int experimentsPerRow = 4;

	auto r = GetScreenWidth();
	const int rectWidth = r / experimentsPerRow - 10;
	const int rectHeight = 60;

	bool mouseOverAny = false;

	int i = 0;
	for (auto& exp : ExperimentFactories) {
		const int x = 10 + (i % experimentsPerRow) * ((r - 10) / experimentsPerRow);
		const int y = 80 + (i / experimentsPerRow) * (rectHeight + 10);

		Color col = LIGHTGRAY;
		Color textColor = DARKGRAY;
		if (CheckCollisionPointRec(GetMousePosition(), { (float)x, (float)y, (float)rectWidth, (float)rectHeight })) {
			col = DARKGREEN;
			textColor = RAYWHITE;
			mouseOverAny = true;
			if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
				Application::Instance().Request_Experiment(exp.first);
				Application::Instance().Request_Stage_Change(NStage::Experiment);
				mouseOverAny = false;
			}
		}

		DrawRectangle(x, y, rectWidth, rectHeight, col);
		DrawProxy::Text(std::to_string(i + 1) + ": " + exp.second.name, x + 5, y + 7, textColor, NAppFont::RegularText);

		i++;
	}

	if (mouseOverAny) {
		SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
	}
	else {
		SetMouseCursor(MOUSE_CURSOR_DEFAULT);
	}

	return true;
}

bool ExperimentStage::On_Enter() {

	auto itr = ExperimentFactories.find(mExperimentType);
	if (itr != ExperimentFactories.end()) {
		mExperiment = itr->second.factory();
	}

	if (mExperiment) {
		mExperiment->On_Init();
	}
	else {
		// Failed to create experiment, return to menu
		Application::Instance().Request_Stage_Change(NStage::Menu);
	}

	return true;
}

bool ExperimentStage::On_Leave() {
	if (mExperiment) {
		mExperiment->On_Cleanup();
		mExperiment.reset();
	}
	return true;
}

bool ExperimentStage::On_Update(float delta_time) {
	return mExperiment ? mExperiment->On_Update(delta_time) : true;
}

bool ExperimentStage::On_Render() {
	return mExperiment ? mExperiment->On_Render() : true;
}
