/**
 * OptVisualDemo
 *
 * Copyright (c) 2025-present, Martin Ubl
 * Distributed under the MIT license
 */

#pragma once

#include <memory>
#include "Core/Experiment.h"

// available experiments
enum class NExperiment {
	None,
	LinearModel2D,
	CircleModel2D,
	Fourier2D_N2,
	Fourier2D_N5,
	Triangle2D,
	Logistic2D,
	Clustering2D,
};

// available optimizers
enum class NOptimizer {
	None,
	GeneticAlgorithm_Simple,
};

class Experiment;

struct ExperimentDefinition {
	std::string name;
	std::function<std::unique_ptr<Experiment>()> factory;
};

extern const std::map<NExperiment, ExperimentDefinition> ExperimentFactories;
