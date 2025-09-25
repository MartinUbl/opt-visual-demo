/**
 * OptVisualDemo
 *
 * Copyright (c) 2025-present, Martin Ubl
 * Distributed under the MIT license
 */

#pragma once

#include <vector>
#include <map>
#include <thread>
#include <memory>
#include <mutex>

#include "Optimizer.h"

#include "../registration.h"

struct TOptimizer_Setup;

enum class TExperiment_Optimize_Mode {
	Fast,
	Medium,
	Slow,
	Stepped
};

/**
 * Base class for experiments
 */
class Experiment {
	protected:
		// name - should be set by derived class
		std::string mName = "Experiment";
		// description - should be set by derived class
		std::string mDescription = "";

		// determines whether the mouse pointer is in the UI area (so clicks should not, e.g., add data points)
		bool Is_Mouse_In_UI_Area() const;

	protected:
		bool mIs_Optimizing = false;
		std::unique_ptr<std::thread> mOptimization_Thread;

		std::mutex mCandidates_Mutex;
		std::vector<std::vector<double>> mCandidates;

		size_t mOpt_Iteration = 0;
		double mOpt_BestMetric = std::numeric_limits<double>::infinity();

	public:
		Experiment() = default;
		virtual ~Experiment() = default;

		// starts the optimization in a separate thread
		void Start_Optimization(TExperiment_Optimize_Mode mode);

	public:
		// initialize experiment
		virtual bool On_Init() { return true; };
		
		// cleanup after experiment
		virtual bool On_Cleanup() { return true; };
		
		// update experiment state, if there is some time-dependent logic
		virtual bool On_Update(float delta_time) { return true; };
		
		// render the experiment
		virtual bool On_Render();

		// draw a candidate solution (best=true for the best candidate)
		virtual void Draw_Candidate(const std::vector<double>& candidate, bool best = false) { };

		// objective function to minimize
		virtual double Objective_Function(const std::vector<double>& parameters) { return 0; }

		// fill the optimizer setup structure with parameters specific to this experiment
		virtual void Fill_Optimizer_Setup(TOptimizer_Setup& setup) { };

		// check whether the experiment is ready to be optimized (e.g., enough data points, etc.)
		virtual bool Check_Can_Optimize() { return true; };

		// draw the reason why optimization cannot be started, return true if something was drawn
		virtual bool Draw_Cannot_Optimize_Reason(int hintPositionX, int hintPositionY) { return false; };

		// reset experiment data (e.g., data points)
		virtual void Reset_Data();
};
