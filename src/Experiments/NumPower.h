/**
 * OptVisualDemo
 *
 * Copyright (c) 2025-present, Martin Ubl
 * Distributed under the MIT license
 */

#pragma once

#include "../Core/Experiment.h"

#include <vector>
#include "raylib.h"

class NumPower : public Experiment {
	public:
		NumPower() = default;
		virtual ~NumPower() = default;

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
