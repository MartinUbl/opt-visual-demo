/**
 * OptVisualDemo
 *
 * Copyright (c) 2025-present, Martin Ubl
 * Distributed under the MIT license
 */

#include "registration.h"

#include "Experiments/LinearModel2D.h"
#include "Experiments/CircleModel2D.h"
#include "Experiments/Fourier2D.h"
#include "Experiments/Triangle2D.h"
#include "Experiments/Logistic2D.h"
#include "Experiments/Clustering2D.h"
#include "Experiments/BallDrop2D.h"

#include <map>

const std::map<NExperiment, ExperimentDefinition> ExperimentFactories = {
	{ NExperiment::LinearModel2D, { "Linear Model 2D", []() { return std::make_unique<LinearModel2D>(); } }},
	{ NExperiment::CircleModel2D, { "Circle Model 2D", []() { return std::make_unique<CircleModel2D>(); } }},
	{ NExperiment::Fourier2D_N2, { "Fourier Model 2D (N = 2)", []() { return std::make_unique<Fourier2D>(2); } }},
	{ NExperiment::Fourier2D_N5, { "Fourier Model 2D (N = 5)", []() { return std::make_unique<Fourier2D>(5); } }},
	{ NExperiment::Triangle2D, { "Triangle 2D", []() { return std::make_unique<Triangle2D>(); } }},
	{ NExperiment::Logistic2D, { "Logistic Regression 2D", []() { return std::make_unique<Logistic2D>(); } }},
	{ NExperiment::Clustering2D, { "Clustering 2D", []() { return std::make_unique<Clustering2D>(); } }},
	{ NExperiment::BallDrop2D, { "Ball Drop 2D", []() { return std::make_unique<BallDrop2D>(); } }},
};
