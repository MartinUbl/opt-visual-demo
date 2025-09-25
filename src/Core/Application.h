/**
 * OptVisualDemo
 *
 * Copyright (c) 2025-present, Martin Ubl
 * Distributed under the MIT license
 */

#pragma once

#include "ObjectAccessor.h"
#include "Stage.h"
#include "Experiment.h"

constexpr int Window_Width = 1200;
constexpr int Window_Height = 800;

/**
 * Main application class, maintains the main loop and manages stages
 */
class Application : public IObject {
	private:
		NStage mCurrent_Stage = NStage::None;
		NStage mRequested_Stage = NStage::None;
		std::unique_ptr<Stage> mStage = nullptr;

		NExperiment mRequested_Experiment = NExperiment::None;

	public:
		Application() = default;
		~Application() override = default;

		// Initialize the application (parse args, setup, etc.)
		bool Init(int argc, char** argv);
		// Run the main application loop
		int Run();

		// Singleton instance accessor
		static Application& Instance() {
			return CObjectAccessor::get<Application>();
		}

		// get current stage
		NStage Current_Stage() const {
			return mCurrent_Stage;
		}

		// request a stage change in the next frame
		void Request_Stage_Change(NStage new_stage) {
			mRequested_Stage = new_stage;
		}

		// request a new experiment to run (effective on next stage change to Experiment)
		void Request_Experiment(NExperiment new_experiment) {
			mRequested_Experiment = new_experiment;
		}
};
