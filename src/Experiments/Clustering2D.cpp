/**
 * OptVisualDemo
 *
 * Copyright (c) 2025-present, Martin Ubl
 * Distributed under the MIT license
 */

#include "Clustering2D.h"

#include "../Core/DrawProxy.h"
#include "../Core/Optimizer.h"
#include "../Core/Helpers.h"

#include "../Optimizers/GeneticAlgorithm.h"

#include <cmath>
#include <array>

bool Clustering2D::On_Init() {
	mData_Points.clear();
	mName = "Clustering 2D";
	mDescription = "K-means-based clustering.";
	return true;
}

bool Clustering2D::On_Cleanup() {
	mData_Points.clear();
	if (mOptimization_Thread && mOptimization_Thread->joinable()) {
		mOptimization_Thread->join();
	}
	return true;
}

bool Clustering2D::On_Update(float delta_time) {
	return true;
}

void Clustering2D::Reset_Data() {
	Experiment::Reset_Data();
	mData_Points.clear();
}

bool Clustering2D::On_Render() {

	TSimple_Input inputA(400, 70, 100, 30, "Number of centroids:", mInputState_Num_Centroids, NAppFont::RegularText, NInput_Mask::Numeric, 4);

	if (mIs_Optimizing) {
		mInputState_Num_Centroids.isActive = false;
	}

	inputA.Render();

	try {
		int val = std::stoi(mInputState_Num_Centroids.text);
		mNum_Centroids = std::max(2, std::min(15, val));

		if (val < 2) {
			mInputState_Num_Centroids.text = "2";
		}
		else if (val > 15) {
			mInputState_Num_Centroids.text = "15";
		}
	}
	catch (...) {
		mNum_Centroids = 100;
		if (mInputState_Num_Centroids.text.empty() && !mInputState_Num_Centroids.isActive) {
			mInputState_Num_Centroids.text = "3";
		}
	}

	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !Is_Mouse_In_UI_Area()) {
		mData_Points.push_back(GetMousePosition());
	}

	std::array<Color, 15> pointColors = { RED, GREEN, BLUE, ORANGE, PURPLE, YELLOW, PINK, SKYBLUE, VIOLET, LIME, GOLD, DARKGREEN, DARKBLUE, BROWN, MAROON };

	for (const auto& point : mData_Points) {
		// select closest centroid to determine color
		Color pointColor = LIGHTGRAY;
		if (mCandidates.size() > 0) {
			std::unique_lock<std::mutex> lock(mCandidates_Mutex, std::defer_lock);
			if (mIs_Optimizing) {
				lock.lock();
			}
			if (!mCandidates.empty()) {
				const auto& bestCandidate = mCandidates[0];
				double minDistSq = std::numeric_limits<double>::infinity();
				int closestCentroidIdx = -1;
				for (int i = 0; i < mNum_Centroids; ++i) {
					double cx = bestCandidate[2 * i];
					double cy = bestCandidate[2 * i + 1];
					double dx = point.x - cx;
					double dy = point.y - cy;
					double distSq = dx * dx + dy * dy;
					if (distSq < minDistSq) {
						minDistSq = distSq;
						closestCentroidIdx = i;
					}
				}
				if (closestCentroidIdx >= 0 && closestCentroidIdx < static_cast<int>(pointColors.size())) {
					pointColor = pointColors[closestCentroidIdx];
				}
			}
		}
		
		DrawCircleV(point, 3, pointColor);
	}

	return Experiment::On_Render();
}

void Clustering2D::Draw_Candidate(const std::vector<double>& candidate, bool best) {
	if (candidate.size() == 2*mNum_Centroids) {
		for (int i = 0; i < mNum_Centroids; ++i) {
			const float cx = static_cast<float>(candidate[2 * i]);
			const float cy = static_cast<float>(candidate[2 * i + 1]);
			if (best) {
				// best candidate
				DrawCircleV({ cx, cy }, 8.0f, GREEN);
				DrawCircleV({ cx, cy }, 5.0f, DARKGREEN);

				// draw a circle around each centroid showing its influence area (distance to the nearest other centroid)
				float minDist = std::numeric_limits<float>::infinity();
				for (int j = 0; j < mNum_Centroids; ++j) {
					if (i != j) {
						const float ox = static_cast<float>(candidate[2 * j]);
						const float oy = static_cast<float>(candidate[2 * j + 1]);
						const float dist = std::sqrtf((cx - ox) * (cx - ox) + (cy - oy) * (cy - oy));
						if (dist < minDist) {
							minDist = dist;
						}
					}
				}
				DrawCircleLines(static_cast<int>(cx), static_cast<int>(cy), minDist / 2.0f, DARKGREEN);
			}
			else {
				// other candidates
				DrawCircleV({ cx, cy }, 5.0f, LIGHTGRAY);
			}
		}
	}
}

bool Clustering2D::Check_Can_Optimize() {
	return mData_Points.size() >= mNum_Centroids;
}

bool Clustering2D::Draw_Cannot_Optimize_Reason(int hintPositionX, int hintPositionY) {
	if (mData_Points.size() < mNum_Centroids) {
		DrawProxy::Text("Add at least 2 points to start optimization", hintPositionX, hintPositionY, DARKGRAY, NAppFont::RegularText);
		return true;
	}

	return false;
}

void Clustering2D::Fill_Optimizer_Setup(TOptimizer_Setup& setup) {
	setup.maxIterations = 2000;
	setup.populationSize = 100;

	setup.lowerBounds.resize(2 * mNum_Centroids, 0.0);
	setup.upperBounds.resize(2 * mNum_Centroids);
	setup.sensitivity.resize(2 * mNum_Centroids, 10.0);
	setup.initialGuess.resize(2 * mNum_Centroids, 0.0);
	for (int i = 0; i < mNum_Centroids; ++i) {
		setup.upperBounds[2 * i] = static_cast<double>(GetScreenWidth());
		setup.upperBounds[2 * i + 1] = static_cast<double>(GetScreenHeight());
	}
}

double Clustering2D::Objective_Function(const std::vector<double>& parameters) {
	if (parameters.size() != 2*mNum_Centroids) {
		throw std::invalid_argument("Expected 2*N parameters (coordinates)");
	}

	// k-means clustering objective: sum of squared distances from each point to the nearest centroid
	double totalError = 0.0;
	for (const auto& point : mData_Points) {
		double minDistSq = std::numeric_limits<double>::infinity();
		for (int i = 0; i < mNum_Centroids; ++i) {
			double cx = parameters[2 * i];
			double cy = parameters[2 * i + 1];
			double dx = point.x - cx;
			double dy = point.y - cy;
			double distSq = dx * dx + dy * dy;
			if (distSq < minDistSq) {
				minDistSq = distSq;
			}
		}
		totalError += minDistSq;
	}
	return totalError;
}
