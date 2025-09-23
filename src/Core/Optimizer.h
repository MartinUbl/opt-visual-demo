/**
 * OptVisualDemo
 *
 * Copyright (c) 2025-present, Martin Ubl
 * Distributed under the MIT license
 */

#pragma once

#include <vector>
#include <functional>
#include <random>

#include "../registration.h"

// callback stage for optimizers; may be used as a bitmask (when does the callback want to be called)
enum class NCallback_Stage {
	Before = 1 << 0,
	After = 1 << 1,
	Both = Before | After
};

// action that the callback may return
enum class NAction {
	Continue,	// continue optimization
	Abort		// abort optimization
};

using TObjective_Fnc = std::function<double(const std::vector<double>& parameters)>;
using TCallback_Fnc = std::function<NAction(NCallback_Stage, size_t, double, const std::vector<std::vector<double>>&)>;

/**
 * Setup structure for optimizers
 */
struct TOptimizer_Setup {
	size_t maxIterations = 100; // maximum number of iterations
	size_t populationSize = 50; // for population-based optimizers
	TObjective_Fnc objectiveFunction; // objective function to minimize
	TCallback_Fnc callbackFunction = nullptr; // optional callback function

	std::vector<double> lowerBounds; // lower bounds for each parameter
	std::vector<double> upperBounds; // upper bounds for each parameter
	std::vector<double> initialGuess; // initial guess for each parameter

	std::vector<double> sensitivity; // sensitivity for each parameter; this is basically the dispersion for a parameter mutation
};

/**
 * Base class for optimizers
 */
class Optimizer {
	protected:
		// prepared for population-based optimizers

		// cureent population
		std::vector<std::vector<double>> mPopulation;
		// objective values for the current population
		std::vector<double> mObjectiveValues;
		// next population (to be swapped with the current one once the iteration is done)
		std::vector<std::vector<double>> mPopulation_Next;

	protected:
		// random generator
		std::random_device mRandDev;
		std::mt19937 mRandGen{ mRandDev() };

	public:
		Optimizer() = default;
		virtual ~Optimizer() = default;

		// perform the optimization according to the setup, returns the best parameters and the best metric found
		virtual void Optimize(const TOptimizer_Setup& setup, std::vector<double>& bestParameters, double& bestMetric) = 0;
};
