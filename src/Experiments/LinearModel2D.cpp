/**
 * OptVisualDemo
 *
 * Copyright (c) 2025-present, Martin Ubl
 * Distributed under the MIT license
 */

#include "LinearModel2D.h"

#include "../Core/DrawProxy.h"
#include "../Core/Optimizer.h"
#include "../Core/Helpers.h"

#include "../Optimizers/GeneticAlgorithm.h"

bool LinearModel2D::On_Init() {
	mData_Points.clear();
	mName = "Linear Model 2D";
	mDescription = "A simple linear model fitting.";
	return true;
}

bool LinearModel2D::On_Cleanup() {
	mData_Points.clear();
	if (mOptimization_Thread && mOptimization_Thread->joinable()) {
		mOptimization_Thread->join();
	}
	return true;
}

bool LinearModel2D::On_Update(float delta_time) {
	return true;
}

void LinearModel2D::Reset_Data() {
	Experiment::Reset_Data();
	mData_Points.clear();
}

bool LinearModel2D::On_Render() {

	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !Is_Mouse_In_UI_Area()) {
		mData_Points.push_back(GetMousePosition());
	}

	for (const auto& point : mData_Points) {
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
	}

	return Experiment::On_Render();
}

void LinearModel2D::Draw_Candidate(const std::vector<double>& candidate, bool best) {
	if (candidate.size() == 2) {
		double slope = candidate[0];
		double intercept = candidate[1];
		// Draw line for this candidate
		float x1 = 0;
		float y1 = (float)(slope * x1 + intercept);
		float x2 = (float)GetScreenWidth();
		float y2 = (float)(slope * x2 + intercept);

		if (best) {
			// best candidate
			DrawLineEx({ x1, y1 }, { x2, y2 }, 2.0f, GREEN);
		}
		else {
			// other candidates
			DrawLine((int)x1, (int)y1, (int)x2, (int)y2, PINK);
		}
	}
}

bool LinearModel2D::Check_Can_Optimize() {
	return mData_Points.size() >= 2;
}

bool LinearModel2D::Draw_Cannot_Optimize_Reason(int hintPositionX, int hintPositionY) {
	if (mData_Points.size() < 2) {
		DrawProxy::Text("Add at least 2 points to start optimization", hintPositionX, hintPositionY, DARKGRAY, NAppFont::RegularText);
		return true;
	}

	return false;
}

void LinearModel2D::Fill_Optimizer_Setup(TOptimizer_Setup& setup) {
	setup.maxIterations = 20000;
	setup.populationSize = 100;

	setup.lowerBounds = { -100000.0, -100000.0 };
	setup.upperBounds = { 100000.0, 100000.0 };
	setup.sensitivity = { 10.0, 100.0 };
	setup.initialGuess = { 0.0, 0.0 };
}

double LinearModel2D::Objective_Function(const std::vector<double>& parameters) {
	if (parameters.size() != 2) {
		throw std::invalid_argument("Expected 2 parameters: slope and intercept");
	}
	double slope = parameters[0];
	double intercept = parameters[1];
	double totalError = 0.0;
	for (const auto& point : mData_Points) {
		double predictedY = slope * point.x + intercept;
		double error = point.y - predictedY;
		totalError += error * error;
	}
	return totalError / mData_Points.size();
}
