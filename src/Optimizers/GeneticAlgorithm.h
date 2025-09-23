/**
 * OptVisualDemo
 *
 * Copyright (c) 2025-present, Martin Ubl
 * Distributed under the MIT license
 */

#pragma once

#include "../Core/Optimizer.h"

#include <random>

class GeneticAlgorithm : public Optimizer {
	private:
		double mMutationRate = 0.01; // Probability of mutation
		double mCrossoverRate = 0.7; // Probability of crossover

		std::vector<std::normal_distribution<double>> mMutation_Distributions;

		std::vector<double> mBest;
		double mBestMetric = std::numeric_limits<double>::infinity();

		size_t Select_Random_Parent(const TOptimizer_Setup& setup, size_t topCount);

		void Crossover(const TOptimizer_Setup& setup, size_t parentA, size_t parentB, size_t targetIdx);
		void Mutate(const TOptimizer_Setup& setup, size_t targetIdx);

		void Generate_Random_Individual(const TOptimizer_Setup& setup, size_t popNextIdx);

		void Evaluate_Population(const TOptimizer_Setup& setup);

		void Apply_Population_Next();

	public:
		GeneticAlgorithm(double mutationRate = 0.01, double crossoverRate = 0.7)
			: mMutationRate(mutationRate), mCrossoverRate(crossoverRate) {}

		void Optimize(const TOptimizer_Setup& setup, std::vector<double>& bestParameters, double& bestMetric) override;
};
