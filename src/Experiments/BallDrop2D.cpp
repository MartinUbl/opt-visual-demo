/**
 * OptVisualDemo
 *
 * Copyright (c) 2025-present, Martin Ubl
 * Distributed under the MIT license
 */

#include "BallDrop2D.h"

#include "../Core/DrawProxy.h"
#include "../Core/Optimizer.h"
#include "../Core/Helpers.h"

#include "../Optimizers/GeneticAlgorithm.h"

#include "../Core/PhysicsWrapper.h"

#include <numbers>

bool BallDrop2D::On_Init() {
	mObstacles.clear();
	mName = "Ball drop 2D";
	mDescription = "Dropping the ball into a target area as fast as possible";
	return true;
}

bool BallDrop2D::On_Cleanup() {
	mObstacles.clear();
	if (mOptimization_Thread && mOptimization_Thread->joinable()) {
		mOptimization_Thread->join();
	}
	return true;
}

bool BallDrop2D::On_Update(float delta_time) {
	return true;
}

void BallDrop2D::Reset_Data() {
	Experiment::Reset_Data();
	mObstacles.clear();
}

bool BallDrop2D::On_Render() {

	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !Is_Mouse_In_UI_Area()) {
		mObstacles.push_back(GetMousePosition());
		mDragging_Obstacles = true;
	}

	if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
		mDragging_Obstacles = false;
	}

	if (mDragging_Obstacles) {
		auto pos = GetMousePosition();
		if (!mObstacles.empty()) {
			auto& last = mObstacles.back();

			constexpr float minDist = 20.0f;

			if (std::abs(pos.x - last.x) > minDist || std::abs(pos.y - last.y) > minDist) {
				mObstacles.push_back(pos);
			}
		}
	}

	DrawCircle(GetScreenWidth() / 2, 50, 5, BEIGE); // starting point
	DrawRectangle(0, GetScreenHeight() - 10, GetScreenWidth(), 10, DARKGREEN); // target area

	for (const auto& obstacle : mObstacles) {
		DrawRectangle(static_cast<int>(obstacle.x) - 20, static_cast<int>(obstacle.y) - 20, 40, 40, RED);
	}

	return Experiment::On_Render();
}

void BallDrop2D::Draw_Candidate(const std::vector<double>& candidate, bool best) {
	if (candidate.size() == 2) {

		if (best) {

			// draw the candidate as a line from the starting point
			Vector2 start = { GetScreenWidth() / 2.0f, 50.0f };
			Vector2 direction = { static_cast<float>(cos(candidate[0])), static_cast<float>(sin(candidate[0])) };
			Vector2 end = { start.x + direction.x * 20.0f * static_cast<float>(candidate[1]), start.y + direction.y * 20.0f * static_cast<float>(candidate[1]) };
			DrawLineEx(start, end, 2.0f, YELLOW);

			const auto& positions = mBest_Positions;
			std::lock_guard<std::mutex> lock(mBest_Positions_Mutex);
			for (const auto& pos : positions) {
				DrawCircle(static_cast<int>(pos.x), static_cast<int>(pos.y), 3, BLUE);
			}
		}

	}
}

void BallDrop2D::Cache_Best_Candidate(double bestMetric, const std::vector<std::vector<double>>& population) {
	if (!population.empty()) {
		std::lock_guard<std::mutex> lock(mBest_Positions_Mutex);
		mBest_Positions.clear();
		// simulate the best candidate and store the positions
		PhysicsWorld world({ .gravity = { 0, 9.81f } });
		world.Add_Dynamic_Ball_Body(GetScreenWidth() / 2.0f, 50.0f, 5.0f, static_cast<float>(population[0][0]), static_cast<float>(population[0][1]));
		for (const auto& obstacle : mObstacles) {
			world.Add_Static_Rect_Body(obstacle.x, obstacle.y, 40.0f, 40.0f);
		}
		// simulate until the ball reaches the bottom of the screen
		for (size_t i = 0; i < 1000; i++) {
			world.Step(1.0f / 60.0f);
			const auto& positions = world.Get_Ball_Positions();
			if (!positions.empty() && positions.back().y >= GetScreenHeight()) {
				// ball reached the bottom of the screen
				break;
			}
		}
		mBest_Positions = world.Get_Ball_Positions();
	}
}

bool BallDrop2D::Check_Can_Optimize() {
	return true;
}

bool BallDrop2D::Draw_Cannot_Optimize_Reason(int hintPositionX, int hintPositionY) {
	return false;
}

void BallDrop2D::Fill_Optimizer_Setup(TOptimizer_Setup& setup) {
	setup.maxIterations = 20000;
	setup.populationSize = 100;

	setup.lowerBounds = { 0, 0 };
	setup.upperBounds = { std::numbers::pi, 10.0 };
	setup.sensitivity = { 0.2, 0.1 };
	setup.initialGuess = { 0.0, 0.0 };
}

double BallDrop2D::Objective_Function(const std::vector<double>& parameters) {
	if (parameters.size() != 2) {
		throw std::invalid_argument("Expected 2 parameters: slope and intercept");
	}

	// we don't require exact euclidean distance, so we can use Manhattan distance for speed
	auto calcDistance = [](const Vector2& a, const Vector2& b) {
		return (std::abs(a.x - b.x) + std::abs(a.y - b.y));
	};

	PhysicsWorld world({ .gravity = { 0, 9.81f } });

	world.Add_Dynamic_Ball_Body(GetScreenWidth() / 2.0f, 50.0f, 5.0f, static_cast<float>(parameters[0]), static_cast<float>(parameters[1]));
	for (const auto& obstacle : mObstacles) {
		if (obstacle.x < 20 || obstacle.x > GetScreenWidth() - 20 || obstacle.y < 20 || obstacle.y > GetScreenHeight() - 20) {
			continue;
		}
		world.Add_Static_Rect_Body(obstacle.x, obstacle.y, 40.0f, 40.0f);
	}

	// simulate until the ball reaches the bottom of the screen
	for (size_t i = 0; i < 1000; i++) {
		world.Step(1.0f / 60.0f);
		const auto& positions = world.Get_Ball_Positions();
		if (!positions.empty() && positions.back().y >= GetScreenHeight()) {
			// go through the positions, assume they mark a valid trajectory
			// calculate the distance it needed to travel using the positions
			double distance = 0.0;
			for (size_t j = 1; j < positions.size(); j++) {
				distance += calcDistance(positions[j - 1], positions[j]) + 1.0f; // +1 to prefer shorter paths with fewer bounces (not exactly bulletproof, but works well enough)
			}
			return distance;
		}
	}

	const auto& positions = world.Get_Ball_Positions();

	// return the distance from the bottom of the screen as penalty
	if (!positions.empty()) {
		return GetScreenHeight() - positions.back().y + 100000.0; // +100000 to ensure it's worse than any valid solution
	}
	else {
		return 1000000.0;
	}
}
