/**
 * OptVisualDemo
 *
 * Copyright (c) 2025-present, Martin Ubl
 * Distributed under the MIT license
 */

#pragma once

#include "../Core/Experiment.h"

#include <vector>
#include <mutex>
#include "raylib.h"

class BallDrop2D : public Experiment {
	private:
		std::vector<Vector2> mObstacles;

		bool mDragging_Obstacles = false;

		std::mutex mBest_Positions_Mutex;
		std::vector<Vector2> mBest_Positions;

	protected:
		void Cache_Best_Candidate(double bestMetric, const std::vector<std::vector<double>>& population) override;

	public:
		BallDrop2D() = default;
		virtual ~BallDrop2D() = default;

		bool On_Init() override;
		bool On_Cleanup() override;
		bool On_Update(float delta_time) override;
		bool On_Render() override;

		void Draw_Candidate(const std::vector<double>& candidate, bool best = false) override;
		double Objective_Function(const std::vector<double>& parameters) override;
		void Fill_Optimizer_Setup(TOptimizer_Setup& setup) override;
		bool Check_Can_Optimize() override;
		void Reset_Data() override;
		bool Draw_Cannot_Optimize_Reason(int hintPositionX, int hintPositionY) override;
};
