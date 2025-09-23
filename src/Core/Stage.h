/**
 * OptVisualDemo
 *
 * Copyright (c) 2025-present, Martin Ubl
 * Distributed under the MIT license
 */

#pragma once

#include <vector>
#include <memory>

#include "Experiment.h"

// available stages
enum class NStage {
	None,
	Menu,
	Experiment
};

/**
 * Base class for application stages
 */
class Stage {
	public:
		Stage() = default;
		virtual ~Stage() = default;

		// called when entering the stage
		virtual bool On_Enter() { return true; };
		// called when leaving the stage
		virtual bool On_Leave() { return true; };

		// update stage logic
		virtual bool On_Update(float delta_time) { return true; };
		// render the stage
		virtual bool On_Render() { return true; }
};

/**
 * Menu stage
 */
class MenuStage : public Stage {
	public:
		MenuStage() = default;
		bool On_Enter() override;
		bool On_Leave() override;
		bool On_Update(float delta_time) override;
		bool On_Render() override;
};

/**
 * Experiment stage
 */
class ExperimentStage : public Stage {
	private:
		NExperiment mExperimentType = NExperiment::None;
		std::unique_ptr<Experiment> mExperiment = nullptr;
	public:
		ExperimentStage(NExperiment exp) : mExperimentType(exp) {}
		bool On_Enter() override;
		bool On_Leave() override;
		bool On_Update(float delta_time) override;
		bool On_Render() override;
};
