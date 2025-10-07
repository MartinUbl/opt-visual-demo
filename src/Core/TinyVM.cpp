#include "TinyVM.h"

namespace TinyVM {
	Machine::Machine(const MachineFeatures& features)
		: mFeatures(features) {
		mRegisters.resize(mFeatures.registersCount, 0.0);
		mExecution_Cost = 0.0;
	}

	void Machine::Prepare_Instructions() {
		// prepare instructions
		mInstruction_Handlers.clear();
		// NOP is always available
		mInstruction_Handlers.emplace_back(std::make_unique<NopInstruction>());
		if (mFeatures.basicArithmetics) {
			mInstruction_Handlers.emplace_back(std::make_unique<AddInstruction>());
			mInstruction_Handlers.emplace_back(std::make_unique<SubInstruction>());
			mInstruction_Handlers.emplace_back(std::make_unique<MulInstruction>());
			mInstruction_Handlers.emplace_back(std::make_unique<DivInstruction>());
		}
	}

	void Machine::Prepare_Sources(const std::vector<double>& input, const std::vector<double>& memory, std::vector<double>& output) {
		mSources.clear();
		// inputs
		if (input.size() != mFeatures.inputCount) {
			return;
		}

		size_t i = 0;
		for (const auto& in : input) {
			mSources.emplace_back(std::make_unique<ReadOnlySource>(in, 0.0, 1.0, "In" + std::to_string(i++)));
		}

		i = 0;
		// registers
		for (auto& reg : mRegisters) {
			mSources.emplace_back(std::make_unique<ReadWriteSource>(reg, "R" + std::to_string(i++)));
		}
		// constants
		if (memory.size() < mFeatures.constantsCount + mFeatures.instructionsCount) {
			return;
		}

		const double constOffset = (mFeatures.constantsMin + mFeatures.constantsMax) / 2.0;
		const double constScale = (mFeatures.constantsMax - mFeatures.constantsMin) / 2.0;

		for (size_t i = 0; i < mFeatures.constantsCount; i++) {
			mSources.emplace_back(std::make_unique<ReadOnlySource>(memory[i], constOffset, constScale, "#" + std::to_string(memory[i] * constScale + constOffset)));
		}
		// outputs
		for (size_t i = 0; i < mFeatures.outputCount; i++) {
			mSources.emplace_back(std::make_unique<ReadWriteSource>(output[i], "Out" + std::to_string(i)));
		}
	}

	// memory = constants + instructions
	std::vector<double> Machine::Run(const std::vector<double>& input, const std::vector<double>& memory) {

		std::vector<double> outputs;
		outputs.resize(mFeatures.outputCount, 0.0);

		// reset registers
		std::fill(mRegisters.begin(), mRegisters.end(), 0.0);

		Prepare_Sources(input, memory, outputs);
		Prepare_Instructions();

		mExecution_Cost = 0.0;

		// execute instructions
		for (size_t i = 0; i < mFeatures.instructionsCount; i++) {
			double encoded = memory[mFeatures.constantsCount + i];
			if (encoded < 0.0 || encoded > 1.0) {
				// invalid instruction encoding
				continue;
			}
			auto [instructionIndex, operand] = Decode_Instruction(encoded);
			if (instructionIndex < mInstruction_Handlers.size()) {
				mInstruction_Handlers[instructionIndex]->Execute(*this, operand);
				mExecution_Cost += mInstruction_Handlers[instructionIndex]->Get_Cost();
			}
		}

		return outputs;
	}

	std::vector<std::string> Machine::Transcribe(const std::vector<double>& memory) {
		std::vector<std::string> result;
		if (memory.size() < mFeatures.constantsCount + mFeatures.instructionsCount) {
			return result;
		}

		std::vector<double> outputs;
		outputs.resize(mFeatures.outputCount, 0.0);

		Prepare_Sources(std::vector<double>(mFeatures.inputCount, 0.0), memory, outputs);
		Prepare_Instructions();

		mExecution_Cost = 0.0;

		for (size_t i = 0; i < mFeatures.instructionsCount; i++) {
			double encoded = memory[mFeatures.constantsCount + i];
			if (encoded < 0.0 || encoded > 1.0) {
				result.push_back("???");
				continue;
			}
			auto [instructionIndex, operand] = Decode_Instruction(encoded);
			if (instructionIndex < mInstruction_Handlers.size()) {
				auto line = mInstruction_Handlers[instructionIndex]->Transcribe(*this, operand);
				if (!line.empty() && line != "NOP") {
					result.push_back(line);
				}
				mExecution_Cost += mInstruction_Handlers[instructionIndex]->Get_Cost();
			}
			else {
				result.push_back("???");
			}
		}
		return result;
	}

	double Machine::Read(const size_t index) const {
		if (index < mSources.size()) {
			return mSources[index]->Get();
		}
		return 0.0;
	}

	void Machine::Write(const size_t index, double value) {
		if (index < mSources.size()) {
			mSources[index]->Set(value);
		}
	}

	std::pair<size_t, double> Machine::Decode_Instruction(double encoded) const {
		// encoded is a number between 0 and 1
		size_t instructionIndex = static_cast<size_t>(encoded * mInstruction_Handlers.size());
		if (instructionIndex >= mInstruction_Handlers.size()) {
			instructionIndex = mInstruction_Handlers.size() - 1;
		}
		double operand = (encoded * mInstruction_Handlers.size()) - instructionIndex;
		return { instructionIndex, operand };
	}

	std::pair<size_t, double> Machine::Decode_Operand(double encoded) const {
		// encoded is a number between 0 and 1
		size_t sourceIndex = static_cast<size_t>(encoded * mSources.size());
		if (sourceIndex >= mSources.size()) {
			sourceIndex = mSources.size() - 1;
		}
		double modifier = (encoded * mSources.size()) - sourceIndex;
		return { sourceIndex, modifier };
	}

	ReadOnlySource::ReadOnlySource(const double& value, double offset, double scale, const std::string& name)
		: DataSource(name), mValue(value), mOffset(offset), mScale(scale) {
	}

	double ReadOnlySource::Get() const {
		return mValue * mScale + mOffset;
	}

	void ReadOnlySource::Set(double value) {
		// does nothing
	}

	ReadWriteSource::ReadWriteSource(double& value, const std::string& name)
		: DataSource(name), mValue(value) {
	}

	double ReadWriteSource::Get() const {
		return mValue;
	}

	void ReadWriteSource::Set(double value) {
		mValue = value;
	}

	void NopInstruction::Execute(Machine& machine, double arg) {
		// does nothing
	}

	std::string NopInstruction::Transcribe(const Machine& machine, double arg) const {
		return "NOP";
	}

	void AddInstruction::Execute(Machine& machine, double arg) {
		auto [srcIndex, srcMod] = machine.Decode_Operand(arg);
		auto [dstIndex, dstMod] = machine.Decode_Operand(srcMod);
		double value = machine.Read(srcIndex);
		double current = machine.Read(dstIndex);
		machine.Write(dstIndex, current + value);
	}

	std::string AddInstruction::Transcribe(const Machine& machine, double arg) const {
		auto [srcIndex, srcMod] = machine.Decode_Operand(arg);
		auto [dstIndex, dstMod] = machine.Decode_Operand(srcMod);
		return "ADD " + machine.Get_Source_Name(dstIndex) + " + " + machine.Get_Source_Name(srcIndex) + " -> " + machine.Get_Source_Name(dstIndex);
	}

	void SubInstruction::Execute(Machine& machine, double arg) {
		auto [srcIndex, srcMod] = machine.Decode_Operand(arg);
		auto [dstIndex, dstMod] = machine.Decode_Operand(srcMod);
		double value = machine.Read(srcIndex);
		double current = machine.Read(dstIndex);
		machine.Write(dstIndex, current - value);
	}

	std::string SubInstruction::Transcribe(const Machine& machine, double arg) const {
		auto [srcIndex, srcMod] = machine.Decode_Operand(arg);
		auto [dstIndex, dstMod] = machine.Decode_Operand(srcMod);
		return "SUB " + machine.Get_Source_Name(dstIndex) + " - " + machine.Get_Source_Name(srcIndex) + " -> " + machine.Get_Source_Name(dstIndex);
	}

	void MulInstruction::Execute(Machine& machine, double arg) {
		auto [srcIndex, srcMod] = machine.Decode_Operand(arg);
		auto [dstIndex, dstMod] = machine.Decode_Operand(srcMod);
		double value = machine.Read(srcIndex);
		double current = machine.Read(dstIndex);
		machine.Write(dstIndex, current * value);
	}

	std::string MulInstruction::Transcribe(const Machine& machine, double arg) const {
		auto [srcIndex, srcMod] = machine.Decode_Operand(arg);
		auto [dstIndex, dstMod] = machine.Decode_Operand(srcMod);
		return "MUL " + machine.Get_Source_Name(dstIndex) + " * " + machine.Get_Source_Name(srcIndex) + " -> " + machine.Get_Source_Name(dstIndex);
	}

	void DivInstruction::Execute(Machine& machine, double arg) {
		auto [srcIndex, srcMod] = machine.Decode_Operand(arg);
		auto [dstIndex, dstMod] = machine.Decode_Operand(srcMod);
		double value = machine.Read(srcIndex);
		if (value == 0.0) {
			return; // avoid division by zero
		}
		double current = machine.Read(dstIndex);
		machine.Write(dstIndex, current / value);
	}

	std::string DivInstruction::Transcribe(const Machine& machine, double arg) const {
		auto [srcIndex, srcMod] = machine.Decode_Operand(arg);
		auto [dstIndex, dstMod] = machine.Decode_Operand(srcMod);
		return "DIV " + machine.Get_Source_Name(dstIndex) + " / " + machine.Get_Source_Name(srcIndex) + " -> " + machine.Get_Source_Name(dstIndex);
	}
}
