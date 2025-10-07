/**
 * OptVisualDemo
 *
 * Copyright (c) 2025-present, Martin Ubl
 * Distributed under the MIT license
 */

#include "NumPower.h"

#include "../Core/DrawProxy.h"
#include "../Core/Optimizer.h"
#include "../Core/Helpers.h"
#include "../Core/TinyVM.h"

#include "../Optimizers/GeneticAlgorithm.h"

namespace {
	constexpr size_t Constants_Count = 2;
	constexpr size_t Instruction_Count = 20;

	constexpr size_t Parameters_Count = Constants_Count + Instruction_Count;

	TinyVM::Machine Create_Machine() {
		TinyVM::MachineFeatures features;
		features.inputCount = 1;
		features.outputCount = 1;
		features.registersCount = 4;
		features.constantsCount = Constants_Count;
		features.instructionsCount = Instruction_Count;
		features.basicArithmetics = true;
		return TinyVM::Machine(features);
	}
}

bool NumPower::On_Init() {
	mName = "VM Power approximation";
	mDescription = "Genetic programming to obtain a simple x^2.1 program.";
	return true;
}

bool NumPower::On_Cleanup() {
	if (mOptimization_Thread && mOptimization_Thread->joinable()) {
		mOptimization_Thread->join();
	}
	return true;
}

bool NumPower::On_Update(float delta_time) {
	return true;
}

void NumPower::Reset_Data() {
	Experiment::Reset_Data();
}

bool NumPower::On_Render() {

	// draw disclaimer
	DrawProxy::Text("WARNING: This method rarely finds a good solution; genetic programming is usually done in a slighly different way. Moreover, this almost always gets stuck in a local minimum.", 10, GetScreenHeight() - 48, MAROON, NAppFont::RegularText);

	return Experiment::On_Render();
}

void NumPower::Draw_Candidate(const std::vector<double>& candidate, bool best) {
	if (candidate.size() == Parameters_Count && best) {
		auto machine = Create_Machine();

		auto program = machine.Transcribe(candidate);
		int startX = 10;
		int startY = 90;
		int lineHeight = 20;
		for (size_t i = 0; i < program.size(); i++) {
			Color color = DARKPURPLE;
			DrawProxy::Text(program[i], startX, startY + static_cast<int>(i) * lineHeight, color, NAppFont::RegularText);
		}

		// run for inputs 0 to 10 and show the output
		int outputStartY = startY + static_cast<int>(program.size()) * lineHeight + 20;
		for (int i = 0; i <= 10; i++) {
			std::vector<double> input = { static_cast<double>(i) };
			std::vector<double> memory = candidate; // program is in the memory
			auto output = machine.Run(input, memory);
			std::string outputStr;
			if (output.size() != 1) {
				outputStr = "Invalid output";
			}
			else {
				outputStr = std::to_string(output[0]);
			}
			std::string line = "Input: " + std::to_string(i) + " | Expected: " + std::to_string(std::pow(static_cast<double>(i), 2.1)) + " | Output: " + outputStr;
			Color color = DARKBLUE;
			DrawProxy::Text(line, startX, outputStartY + i * lineHeight, color, NAppFont::RegularText);
		}
	}
}

bool NumPower::Check_Can_Optimize() {
	return true;
}

bool NumPower::Draw_Cannot_Optimize_Reason(int hintPositionX, int hintPositionY) {
	return false;
}

void NumPower::Fill_Optimizer_Setup(TOptimizer_Setup& setup) {
	setup.maxIterations = 20000;
	setup.populationSize = 100;

	setup.lowerBounds = std::vector<double>(Parameters_Count, 0.0); // constants and instructions
	setup.upperBounds = std::vector<double>(Parameters_Count, 1.0); // constants and instructions
	setup.initialGuess = std::vector<double>(Parameters_Count, 0.0); // nulls or NOPs
	setup.sensitivity = std::vector<double>(Parameters_Count, 0.1); // small mutations
}

double NumPower::Objective_Function(const std::vector<double>& parameters) {
	// we expect Parameters_Count parameters
	if (parameters.size() != Parameters_Count) {
		throw std::invalid_argument("Expected " + std::to_string(Parameters_Count) + " parameters for the TinyVM program");
	}

	auto machine = Create_Machine();
	// run the program for inputs 0 to 10 and compare the output to the expected value (input^2)
	double totalError = 0.0;
	for (int i = 0; i <= 10; i++) {
		const std::vector<double> input = { static_cast<double>(i) };
		std::vector<double> memory = parameters; // program is in the memory
		auto output = machine.Run(input, memory);
		if (output.size() != 1) {
			// invalid output
			totalError += 1000.0;
		}
		else {
			const double expected = std::pow(static_cast<double>(i), 2.1);
			const double error = output[0] - expected;
			totalError += error * error; // squared error
		}
	}

	return totalError + machine.Get_Execution_Cost();
}
