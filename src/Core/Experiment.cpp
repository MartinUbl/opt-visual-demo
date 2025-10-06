/**
 * OptVisualDemo
 *
 * Copyright (c) 2025-present, Martin Ubl
 * Distributed under the MIT license
 */

#include "Experiment.h"
#include "Application.h"

#include "DrawProxy.h"

#include "Helpers.h"

#include "Optimizer.h"
#include "../Optimizers/GeneticAlgorithm.h"

bool Experiment::Is_Mouse_In_UI_Area() const {
	auto pos = GetMousePosition();

	// title bar
	if (pos.y < 50) {
		return true;
	}
	// buttons on the top right
	if (pos.x > GetScreenWidth() - 110 && pos.y < 200) {
		return true;
	}

	return false;
}

bool Experiment::On_Render() {
	// draw candidates
	{
		std::lock_guard<std::mutex> lock(mCandidates_Mutex);
		if (mCandidates.size() > 0) {
			for (size_t i = 1; i < mCandidates.size(); ++i) {
				Draw_Candidate(mCandidates[i], false);
			}

			// draw the best candidate on top
			Draw_Candidate(mCandidates[0], true);
		}
	}

	TSimple_Button btnClear(GetScreenWidth() - 10 - 100, 50, 100, 30, "Clear");
	if (btnClear.Render()) {
		Reset_Data();
	}

	TSimple_Button btnOptimize(GetScreenWidth() - 10 - 100, 90, 100, 30, "Optimize fast");
	if (btnOptimize.Render()) {
		Start_Optimization(TExperiment_Optimize_Mode::Fast);
	}

	TSimple_Button btnOptimizeMedium(GetScreenWidth() - 10 - 100, 130, 100, 30, "Optimize medium");
	if (btnOptimizeMedium.Render()) {
		Start_Optimization(TExperiment_Optimize_Mode::Medium);
	}

	TSimple_Button btnOptimizeSlow(GetScreenWidth() - 10 - 100, 170, 100, 30, "Optimize slow");
	if (btnOptimizeSlow.Render()) {
		Start_Optimization(TExperiment_Optimize_Mode::Slow);
	}

	if (mIs_Optimizing) {
		DrawProxy::Text("Optimizing... Iteration: " + std::to_string(mOpt_Iteration) + " Best Metric: " + std::to_string(mOpt_BestMetric), 10, GetScreenHeight() - 30, DARKGRAY, NAppFont::RegularText);
	}
	else {
		if (!Draw_Cannot_Optimize_Reason(10, GetScreenHeight() - 30)) {
			if (mCandidates.size() > 0) {
				DrawProxy::Text("Optimization complete. Best Metric: " + std::to_string(mOpt_BestMetric), 10, GetScreenHeight() - 30, DARKGRAY, NAppFont::RegularText);
			}
			else {
				DrawProxy::Text("Click 'Optimize' to start", 10, GetScreenHeight() - 30, DARKGRAY, NAppFont::RegularText);
			}
		}
	}

	// title bar
	DrawRectangle(0, 0, GetScreenWidth(), 40, LIGHTGRAY);

	// back button
	TSimple_Button btnBack(0, 0, 40, 40, "<", NAppFont::Title);
	if (btnBack.Render()) {
		Reset_Data();
		Application::Instance().Request_Stage_Change(NStage::Menu);
	}

	// experiment name and description
	DrawProxy::Text(mName, 48, 3, DARKGRAY, NAppFont::Title);
	DrawProxy::Text(mDescription, 10, 45, DARKGRAY, NAppFont::Subtitle);

	return true;
}

void Experiment::Reset_Data() {
	mIs_Optimizing = false;
	if (mOptimization_Thread && mOptimization_Thread->joinable()) {
		mOptimization_Thread->join();
	}
	mCandidates.clear();
	mOpt_Iteration = 0;
	mOpt_BestMetric = std::numeric_limits<double>::infinity();
}

void Experiment::Start_Optimization(TExperiment_Optimize_Mode mode) {
	if (mIs_Optimizing) {
		return;
	}

	if (!Check_Can_Optimize()) {
		return;
	}

	if (mOptimization_Thread && mOptimization_Thread->joinable()) {
		mOptimization_Thread->join();
	}

	mIs_Optimizing = true;
	mOptimization_Thread = std::make_unique<std::thread>([this, mode]() {
		// Prepare optimizer
		TOptimizer_Setup setup;

		setup.objectiveFunction = [this](const std::vector<double>& params) {
			return this->Objective_Function(params);
		};

		setup.callbackFunction = [this, mode](NCallback_Stage stage, size_t iteration, double bestMetric, const std::vector<std::vector<double>>& population) {
			// can be used to visualize the optimization process
			{
				std::lock_guard<std::mutex> lock(mCandidates_Mutex);
				mCandidates = population;
				mOpt_Iteration = iteration;
				mOpt_BestMetric = bestMetric;
			}
			// sleep a bit to allow visualization
			if (mode == TExperiment_Optimize_Mode::Stepped) {
				// in stepped mode, wait until the user clicks to proceed
				/*while (mIs_Optimizing) {
					if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
						break;
					}
					std::this_thread::sleep_for(std::chrono::milliseconds(10));
				}*/
			}
			else if (mode == TExperiment_Optimize_Mode::Slow) {
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
			else if (mode == TExperiment_Optimize_Mode::Medium) {
				std::this_thread::sleep_for(std::chrono::milliseconds(30));
			}

			Cache_Best_Candidate(bestMetric, population);

			if (!mIs_Optimizing) {
				return NAction::Abort;
			}

			return NAction::Continue;
		};

		Fill_Optimizer_Setup(setup);

		std::vector<double> bestParameters;
		double bestMetric = 0;

		// TODO: differentiate between different optimizers
		GeneticAlgorithm ga(0.05, 0.85);
		ga.Optimize(setup, bestParameters, bestMetric);

		mCandidates[0] = bestParameters;
		mOpt_BestMetric = bestMetric;

		mIs_Optimizing = false;
	});
}
