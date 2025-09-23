/**
 * OptVisualDemo
 *
 * Copyright (c) 2025-present, Martin Ubl
 * Distributed under the MIT license
 */

#include "CircleModel2D.h"

#include "../Core/DrawProxy.h"
#include "../Core/Optimizer.h"
#include "../Core/Helpers.h"

#include "../Optimizers/GeneticAlgorithm.h"

bool CircleModel2D::On_Init() {
	mData_Points.clear();
	mName = "Circle Model 2D";
	mDescription = "A simple circular model fitting.";
	return true;
}

bool CircleModel2D::On_Cleanup() {
	mData_Points.clear();
	if (mOptimization_Thread && mOptimization_Thread->joinable()) {
		mOptimization_Thread->join();
	}
	return true;
}

bool CircleModel2D::On_Update(float delta_time) {
	return true;
}

void CircleModel2D::Reset_Data() {
	Experiment::Reset_Data();
	mData_Points.clear();
}

bool CircleModel2D::On_Render() {

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

		DrawProxy::Text("C[" + std::to_string(mCandidates[0][0]) + "; " + std::to_string(mCandidates[0][1]) + "], r = " + std::to_string(mCandidates[0][2]), 10, GetScreenHeight() - 50, DARKGRAY, NAppFont::RegularText);
	}

	return Experiment::On_Render();
}

void CircleModel2D::Draw_Candidate(const std::vector<double>& candidate, bool best) {
	if (candidate.size() == 3) {
		double x = candidate[0];
		double y = candidate[1];
		double radius = candidate[2];

		Color col = best ? BLUE : LIGHTGRAY;
		DrawCircleLines((int)x, (int)y, (float)radius, col);
	}
}

bool CircleModel2D::Check_Can_Optimize() {
	return mData_Points.size() >= 3;
}

bool CircleModel2D::Draw_Cannot_Optimize_Reason(int hintPositionX, int hintPositionY) {
	if (mData_Points.size() < 3) {
		DrawProxy::Text("Add at least 3 points to start optimization", hintPositionX, hintPositionY, DARKGRAY, NAppFont::RegularText);
		return true;
	}

	return false;
}

void CircleModel2D::Fill_Optimizer_Setup(TOptimizer_Setup& setup) {
	setup.maxIterations = 80000;
	setup.populationSize = 50;

	setup.lowerBounds = { -1000.0, -1000.0, 1.0 };
	setup.upperBounds = { 1000.0, 1000.0, 10000.0 };
	setup.sensitivity = { 500.0, 500.0, 500.0 };

	setup.initialGuess = { 0.0, 0.0, 1.0 };
}

double CircleModel2D::Objective_Function(const std::vector<double>& parameters) {
	if (parameters.size() != 3) {
		throw std::invalid_argument("Expected 3 parameters: x, y and radius");
	}

	const double centerX = parameters[0];
	const double centerY = parameters[1];
	const double radius = parameters[2];
	double totalError = 0.0;
	for (const auto& point : mData_Points) {
		const double dx = point.x - centerX;
		const double dy = point.y - centerY;
		const double dist = std::sqrt(dx * dx + dy * dy);
		const double error = dist - radius;
		totalError += error * error; // squared error
	}
	return totalError / mData_Points.size();
}
