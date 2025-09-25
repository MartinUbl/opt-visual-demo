/**
 * OptVisualDemo
 *
 * Copyright (c) 2025-present, Martin Ubl
 * Distributed under the MIT license
 */

#include "Logistic2D.h"

#include "../Core/DrawProxy.h"
#include "../Core/Optimizer.h"
#include "../Core/Helpers.h"

#include "../Optimizers/GeneticAlgorithm.h"

#include <cmath>

bool Logistic2D::On_Init() {
	mData_Points_A.clear();
	mData_Points_B.clear();
	mName = "Logistic regression 2D";
	mDescription = "Logistic regression on given points.";
	return true;
}

bool Logistic2D::On_Cleanup() {
	mData_Points_A.clear();
	mData_Points_B.clear();
	if (mOptimization_Thread && mOptimization_Thread->joinable()) {
		mOptimization_Thread->join();
	}
	return true;
}

bool Logistic2D::On_Update(float delta_time) {
	return true;
}

void Logistic2D::Reset_Data() {
	Experiment::Reset_Data();
	mData_Points_A.clear();
	mData_Points_B.clear();
}

bool Logistic2D::On_Render() {

	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !Is_Mouse_In_UI_Area()) {
		mData_Points_A.push_back(GetMousePosition());
	}
	if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && !Is_Mouse_In_UI_Area()) {
		mData_Points_B.push_back(GetMousePosition());
	}

	for (const auto& point : mData_Points_A) {
		DrawCircleV(point, 3, RED);
	}
	for (const auto& point : mData_Points_B) {
		DrawCircleV(point, 3, GREEN);
	}

	return Experiment::On_Render();
}

void Logistic2D::Draw_Candidate(const std::vector<double>& candidate, bool best) {
	// candidate is [w0, w1, w2], we need to draw the line w0 + w1*x + w2*y = 0
	const double w0 = candidate[0];
	const double w1 = candidate[1];
	const double w2 = candidate[2];

	if (std::fabs(w2) > std::numeric_limits<double>::epsilon()) {
		// y = (-w0 - w1*x) / w2
		const double x1 = 0.0;
		const double y1 = (-w0 - w1 * x1) / w2;
		const double x2 = static_cast<double>(GetScreenWidth());
		const double y2 = (-w0 - w1 * x2) / w2;

		DrawLine(static_cast<int>(x1), static_cast<int>(y1), static_cast<int>(x2), static_cast<int>(y2), best ? BLUE : LIGHTGRAY);
	} else if (std::fabs(w1) > std::numeric_limits<double>::epsilon()) {
		const double x = -w0 / w1;
		DrawLine(static_cast<int>(x), 0, static_cast<int>(x), GetScreenHeight(), best ? BLUE : LIGHTGRAY);
	}
}

bool Logistic2D::Check_Can_Optimize() {
	return mData_Points_A.size() >= 1 && mData_Points_B.size() >= 1;
}

bool Logistic2D::Draw_Cannot_Optimize_Reason(int hintPositionX, int hintPositionY) {
	if (mData_Points_A.size() == 0 || mData_Points_B.size() == 0) {
		DrawProxy::Text("Add at least 1 point of each class to start optimization", hintPositionX, hintPositionY, DARKGRAY, NAppFont::RegularText);
		return true;
	}

	return false;
}

void Logistic2D::Fill_Optimizer_Setup(TOptimizer_Setup& setup) {
	setup.maxIterations = 50000;
	setup.populationSize = 100;

	setup.lowerBounds = { -1000.0, -1000.0, -1000.0 };
	setup.upperBounds = { 1000.0, 1000.0, 1000.0 };
	setup.sensitivity = { 10.0, 10.0, 10.0 };
	setup.initialGuess = { 0.0, 0.0, 0.0 };
}

double Logistic2D::Objective_Function(const std::vector<double>& parameters) {
	auto h = [parameters](double x, double y) {
		const double z = parameters[0] + parameters[1] * x + parameters[2] * y;
		return 1.0 / (1.0 + std::exp(-z));
	};

	double logLikelihood = 0.0;
	for (size_t i = 0; i < mData_Points_A.size(); ++i) {
		const double xi = mData_Points_A[i].x, yi = mData_Points_A[i].y;
		const double hi = h(xi, yi);
		logLikelihood += -std::log(hi + 1e-9);
	}

	for (size_t i = 0; i < mData_Points_B.size(); ++i) {
		const double xi = mData_Points_B[i].x, yi = mData_Points_B[i].y;
		const double hi = h(xi, yi);
		logLikelihood += -std::log(1.0 - hi + 1e-9);
	}

	return logLikelihood;
}
