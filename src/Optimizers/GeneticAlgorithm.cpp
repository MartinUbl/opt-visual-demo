/**
 * OptVisualDemo
 *
 * Copyright (c) 2025-present, Martin Ubl
 * Distributed under the MIT license
 */

#include "GeneticAlgorithm.h"

#include <random>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <limits>

size_t GeneticAlgorithm::Select_Random_Parent(const TOptimizer_Setup& setup, size_t topCount) {
	std::uniform_int_distribution<> dis(0, (int)topCount - 1);
	return dis(mRandGen);
}

void GeneticAlgorithm::Crossover(const TOptimizer_Setup& setup, size_t parentA, size_t parentB, size_t targetIdx) {
	std::uniform_real_distribution<> dis(0.0, 1.0);
	if (dis(mRandGen) < mCrossoverRate) {
		// Single-point crossover
		size_t paramCount = mPopulation[parentA].size();
		std::uniform_int_distribution<> crossPointDis(1, (int)paramCount - 1);
		size_t crossPoint = crossPointDis(mRandGen);
		for (size_t i = 0; i < paramCount; ++i) {
			if (i < crossPoint) {
				mPopulation_Next[targetIdx][i] = mPopulation[parentA][i];
			}
			else {
				mPopulation_Next[targetIdx][i] = mPopulation[parentB][i];
			}
		}
	}
	else {
		// No crossover, copy parent A
		mPopulation_Next[targetIdx] = mPopulation[parentA];
	}
}

void GeneticAlgorithm::Mutate(const TOptimizer_Setup& setup, size_t targetIdx) {
	std::uniform_real_distribution<> dis(0.0, 1.0);
	for (size_t i = 0; i < mPopulation_Next[targetIdx].size(); ++i) {
		if (dis(mRandGen) < mMutationRate) {

			const double mutAmount = mMutation_Distributions[i](mRandGen);
			mPopulation_Next[targetIdx][i] += mutAmount;

			// Ensure within bounds
			if (mPopulation_Next[targetIdx][i] < setup.lowerBounds[i]) {
				mPopulation_Next[targetIdx][i] = setup.lowerBounds[i];
			}
			else if (mPopulation_Next[targetIdx][i] > setup.upperBounds[i]) {
				mPopulation_Next[targetIdx][i] = setup.upperBounds[i];
			}
		}
	}
}

void GeneticAlgorithm::Generate_Random_Individual(const TOptimizer_Setup& setup, size_t popNextIdx) {
	std::uniform_real_distribution<> dis(0.0, 1.0);
	for (size_t i = 0; i < setup.lowerBounds.size(); ++i) {
		mPopulation_Next[popNextIdx][i] = setup.lowerBounds[i] + dis(mRandGen) * (setup.upperBounds[i] - setup.lowerBounds[i]);
	}
}

void GeneticAlgorithm::Evaluate_Population(const TOptimizer_Setup& setup) {
	for (size_t i = 0; i < mPopulation.size(); ++i) {
		mObjectiveValues[i] = setup.objectiveFunction(mPopulation[i]);
	}
}

void GeneticAlgorithm::Apply_Population_Next() {
	mPopulation = mPopulation_Next;
}

void GeneticAlgorithm::Optimize(const TOptimizer_Setup& setup, std::vector<double>& bestParameters, double& bestMetric) {
	if (setup.populationSize < 2 || setup.lowerBounds.size() != setup.upperBounds.size() || setup.objectiveFunction == nullptr) {
		throw std::invalid_argument("Invalid optimizer setup");
	}

	// Prepare mutation distributions; sensitivity is not mandatory, fall back to 10.0
	mMutation_Distributions.clear();
	for (size_t i = 0; i < setup.lowerBounds.size(); ++i) {
		double sensitivity = 10.0;
		if (i < setup.sensitivity.size() && setup.sensitivity[i] > 0.0) {
			sensitivity = setup.sensitivity[i];
		}
		mMutation_Distributions.emplace_back(0.0, sensitivity);
	}

	const size_t paramCount = setup.lowerBounds.size();
	mPopulation.resize(setup.populationSize, std::vector<double>(paramCount));
	mPopulation_Next.resize(setup.populationSize, std::vector<double>(paramCount));

	mObjectiveValues.resize(setup.populationSize);
	// Initialize population
	for (size_t i = 0; i < setup.populationSize; ++i) {
		Generate_Random_Individual(setup, i);
	}

	Apply_Population_Next();

	if (!setup.initialGuess.empty() && setup.initialGuess.size() == paramCount) {
		// Replace the first individual with the initial guess
		mPopulation[0] = setup.initialGuess;
	}

	Evaluate_Population(setup);
	bestMetric = std::numeric_limits<double>::infinity();
	bestParameters.clear();
	if (setup.callbackFunction) {
		if (setup.callbackFunction(NCallback_Stage::Before, 0, 0, mPopulation) == NAction::Abort) {
			return;
		}
	}

	for (size_t iter = 0; iter < setup.maxIterations; ++iter) {

		// Sort population by objective values (ascending)
		std::vector<size_t> indices(setup.populationSize);
		
		for (size_t i = 0; i < setup.populationSize; ++i) {
			indices[i] = i;
		}

		std::sort(indices.begin(), indices.end(), [this](size_t a, size_t b) {
			return mObjectiveValues[a] < mObjectiveValues[b];
		});

		// Update best found solution
		if (mObjectiveValues[indices[0]] < bestMetric) {
			bestMetric = mObjectiveValues[indices[0]];
			bestParameters = mPopulation[indices[0]];

			if (bestMetric < mBestMetric) {
				mBestMetric = bestMetric;
				mBest = bestParameters;
			}
		}

		size_t topCount = setup.populationSize / 2;
		// Create next generation
		for (size_t i = 0; i < setup.populationSize; ++i) {
			size_t parentA = Select_Random_Parent(setup, topCount);
			size_t parentB = Select_Random_Parent(setup, topCount);
			Crossover(setup, indices[parentA], indices[parentB], i);
			Mutate(setup, i);
			// With small probability, generate a completely random individual
			std::uniform_real_distribution<> dis(0.0, 1.0);
			if (dis(mRandGen) < 0.05) {
				Generate_Random_Individual(setup, i);
			}
		}

		Apply_Population_Next();
		Evaluate_Population(setup);

		// sort the population vector by the objective values
		std::sort(indices.begin(), indices.end(), [this](size_t a, size_t b) {
			return mObjectiveValues[a] < mObjectiveValues[b];
		});

		// select the worst parameters and replace them with the mBest
		mPopulation[*indices.rbegin()] = mBest;
		mObjectiveValues[*indices.rbegin()] = setup.objectiveFunction(mBest); // re-evaluate the best as the data may have changed

		// sort the population vector by the objective values
		std::sort(indices.begin(), indices.end(), [this](size_t a, size_t b) {
			return mObjectiveValues[a] < mObjectiveValues[b];
		});

		// sort the mPopulation vector by the objective values
		std::vector<std::vector<double>> sortedPopulation(setup.populationSize);
		for (size_t i = 0; i < setup.populationSize; ++i) {
			sortedPopulation[i] = mPopulation[indices[i]];
		}

		if (setup.callbackFunction) {
			if (setup.callbackFunction(NCallback_Stage::After, iter, mObjectiveValues[indices[0]], sortedPopulation) == NAction::Abort) {
				break;
			}
		}
	}

	// sort population one last time to get the best solution
	std::vector<size_t> indices(setup.populationSize);
	for (size_t i = 0; i < setup.populationSize; ++i) {
		indices[i] = i;
	}
	std::sort(indices.begin(), indices.end(), [this](size_t a, size_t b) {
		return mObjectiveValues[a] < mObjectiveValues[b];
	});
	bestMetric = mObjectiveValues[indices[0]];
	bestParameters = mPopulation[indices[0]];
}
