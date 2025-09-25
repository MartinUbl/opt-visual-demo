/**
 * OptVisualDemo
 *
 * Copyright (c) 2025-present, Martin Ubl
 * Distributed under the MIT license
 */

#include "Triangle2D.h"

#include "../Core/DrawProxy.h"
#include "../Core/Optimizer.h"
#include "../Core/Helpers.h"
#include "../Core/Application.h"

#include "../Optimizers/GeneticAlgorithm.h"

#include <cmath>
#include <format>

bool Triangle2D::On_Init() {
	mName = "Triangle 2D";
	mDescription = "Optimizing a triangle with sides of given lengths.";

	mInputStateA.text = std::to_string(mLength_A);
	mInputStateB.text = std::to_string(mLength_B);
	mInputStateC.text = std::to_string(mLength_C);

	return true;
}

bool Triangle2D::On_Cleanup() {
	if (mOptimization_Thread && mOptimization_Thread->joinable()) {
		mOptimization_Thread->join();
	}
	return true;
}

bool Triangle2D::On_Update(float delta_time) {
	return true;
}

void Triangle2D::Reset_Data() {
	Experiment::Reset_Data();
}

bool Triangle2D::On_Render() {

	TSimple_Input inputA(400, 70, 100, 30, "A:", mInputStateA, NAppFont::RegularText, NInput_Mask::Numeric, 4);
	inputA.Render();

	TSimple_Input inputB(550, 70, 100, 30, "B:", mInputStateB, NAppFont::RegularText, NInput_Mask::Numeric, 4);
	inputB.Render();

	TSimple_Input inputC(700, 70, 100, 30, "C:", mInputStateC, NAppFont::RegularText, NInput_Mask::Numeric, 4);
	inputC.Render();

	try {
		mLength_A = std::max(1, std::min(Window_Width, std::stoi(mInputStateA.text)));
	}
	catch (...) {
		mLength_A = 100;
		if (mInputStateA.text.empty() && !mInputStateA.isActive) {
			mInputStateA.text = "100";
		}
	}

	try {
		mLength_B = std::max(1, std::min(Window_Width, std::stoi(mInputStateB.text)));
	}
	catch (...) {
		mLength_B = 100;
		if (mInputStateB.text.empty() && !mInputStateB.isActive) {
			mInputStateB.text = "100";
		}
	}

	try {
		mLength_C = std::max(1, std::min(Window_Width, std::stoi(mInputStateC.text)));
	}
	catch (...) {
		mLength_C = 100;
		if (mInputStateC.text.empty() && !mInputStateC.isActive) {
			mInputStateC.text = "100";
		}
	}

	/*for (const auto& point : mData_Points) {
		DrawCircleV(point, 3, RED);
	}

	// draw candidates
	if (mCandidates.size() > 0) {

		std::unique_lock<std::mutex> lock(mCandidates_Mutex, std::defer_lock);
		if (mIs_Optimizing) {
			lock.lock();
		}

		const double adjustedSlope = -mCandidates[0][0];
		const double adjustedIntercept = GetScreenHeight() - mCandidates[0][1];

		DrawProxy::Text("y = " + std::to_string(adjustedSlope) + " * x + " + std::to_string(adjustedIntercept), 10, GetScreenHeight() - 50, DARKGRAY, NAppFont::RegularText);
	}*/

	return Experiment::On_Render();
}

void Triangle2D::Draw_Candidate(const std::vector<double>& candidate, bool best) {
	if (candidate.size() == 6) {
		const float x1 = static_cast<float>(candidate[0]);
		const float y1 = static_cast<float>(candidate[1]);
		const float x2 = static_cast<float>(candidate[2]);
		const float y2 = static_cast<float>(candidate[3]);
		const float x3 = static_cast<float>(candidate[4]);
		const float y3 = static_cast<float>(candidate[5]);

		if (best) {
			// best candidate
			DrawLineEx({ x1, y1 }, { x2, y2 }, 2.0f, GREEN);
			DrawLineEx({ x2, y2 }, { x3, y3 }, 2.0f, GREEN);
			DrawLineEx({ x3, y3 }, { x1, y1 }, 2.0f, GREEN);

			const float length_A = std::sqrtf((x3 - x2) * (x3 - x2) + (y3 - y2) * (y3 - y2));
			const float length_B = std::sqrtf((x3 - x1) * (x3 - x1) + (y3 - y1) * (y3 - y1));
			const float length_C = std::sqrtf((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));

			// draw lengths of sides at the middle of each side
			DrawProxy::Text(std::format("{:.1f}", length_C), (int)((x1 + x2) / 2), (int)((y1 + y2) / 2) - 10, DARKGRAY, NAppFont::RegularText);
			DrawProxy::Text(std::format("{:.1f}", length_B), (int)((x1 + x3) / 2), (int)((y1 + y3) / 2) - 10, DARKGRAY, NAppFont::RegularText);
			DrawProxy::Text(std::format("{:.1f}", length_A), (int)((x2 + x3) / 2), (int)((y2 + y3) / 2) - 10, DARKGRAY, NAppFont::RegularText);
		}
		else {
			// other candidates
			DrawLine(static_cast<int>(x1), static_cast<int>(y1), static_cast<int>(x2), static_cast<int>(y2), PINK);
			DrawLine(static_cast<int>(x2), static_cast<int>(y2), static_cast<int>(x3), static_cast<int>(y3), PINK);
			DrawLine(static_cast<int>(x3), static_cast<int>(y3), static_cast<int>(x1), static_cast<int>(y1), PINK);
		}
	}
}

bool Triangle2D::Check_Can_Optimize() {
	return true;
}

bool Triangle2D::Draw_Cannot_Optimize_Reason(int hintPositionX, int hintPositionY) {
	/*if (???) {
		DrawProxy::Text("???", hintPositionX, hintPositionY, DARKGRAY, NAppFont::RegularText);
		return true;
	}*/

	return false;
}

void Triangle2D::Fill_Optimizer_Setup(TOptimizer_Setup& setup) {
	setup.maxIterations = 10000;
	setup.populationSize = 100;

	setup.lowerBounds = { 100, 100, 100, 100, 100, 100 };
	setup.upperBounds = { Window_Width, Window_Height, Window_Width, Window_Height, Window_Width, Window_Height };
	setup.sensitivity = { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 };
	setup.initialGuess = setup.lowerBounds;
}

double Triangle2D::Objective_Function(const std::vector<double>& parameters) {
	if (parameters.size() != 6) {
		throw std::invalid_argument("Expected 6 parameters: x1, y1, x2, y2, x3, y3");
	}

	auto distance = [](float x1, float y1, float x2, float y2) {
		return std::sqrtf((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
	};

	const float x1 = static_cast<float>(parameters[0]);
	const float y1 = static_cast<float>(parameters[1]);
	const float x2 = static_cast<float>(parameters[2]);
	const float y2 = static_cast<float>(parameters[3]);
	const float x3 = static_cast<float>(parameters[4]);
	const float y3 = static_cast<float>(parameters[5]);

	const float a = distance(x2, y2, x3, y3);
	const float b = distance(x1, y1, x3, y3);
	const float c = distance(x1, y1, x2, y2);

	return (a - mLength_A) * (a - mLength_A) + (b - mLength_B) * (b - mLength_B) + (c - mLength_C) * (c - mLength_C);
}
