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

class Fourier2D : public Experiment {
	private:
		std::vector<Vector2> mData_Points;

		Vector2 From_Screen_To_Cartesian(const Vector2& screenPoint) const;
		Vector2 From_Cartesian_To_Screen(const Vector2& cartesianPoint) const;

		const size_t mNum_Harmonics = 3;

		void Play_Sound();

	public:
		Fourier2D(size_t numHarmonics = 3) : mNum_Harmonics(numHarmonics) {}
		virtual ~Fourier2D() = default;

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
