#pragma once

#include <vector>
#include <memory>
#include <string>

namespace TinyVM {

	struct MachineFeatures {
		size_t inputCount = 1;
		size_t outputCount = 1;
		size_t registersCount = 4;
		size_t constantsCount = 0;
		size_t instructionsCount = 20;

		double constantsMin = -10.0;
		double constantsMax = 10.0;

		bool basicArithmetics = true; // + - * /
		// room for more features in the future
	};

	class DataSource {
		protected:
			std::string mName;

		public:
			DataSource(const std::string& name = "DS") : mName(name) {};
			virtual double Get() const { return 1.0; };
			virtual void Set(double value) { };

			const std::string& Get_Name() const { return mName; }
	};

	class ReadOnlySource : public DataSource {
		private:
			const double& mValue;

			const double mOffset = 0.0;
			const double mScale = 1.0;

		public:
			ReadOnlySource(const double& value, double offset = 0.0, double scale = 1.0, const std::string& name = "RO");
			double Get() const override;
			void Set(double value) override; // does nothing
	};

	class ReadWriteSource : public DataSource {
		private:
			double& mValue;

		public:
			ReadWriteSource(double& value, const std::string& name = "RW");
			double Get() const override;
			void Set(double value) override;
	};

	class Machine;

	class Instruction {
		public:
			virtual void Execute(Machine& machine, double arg) = 0;
			virtual std::string Transcribe(const Machine& machine, double arg) const = 0;

			virtual double Get_Cost() const { return 0.0; }
	};

	class NopInstruction : public Instruction {
		public:
			void Execute(Machine& machine, double arg) override;
			std::string Transcribe(const Machine& machine, double arg) const override;

			double Get_Cost() const override { return 0.0; }
	};

	class AddInstruction : public Instruction {
		public:
			void Execute(Machine& machine, double arg) override;
			std::string Transcribe(const Machine& machine, double arg) const override;

			double Get_Cost() const override { return 1.0; }
	};

	class SubInstruction : public Instruction {
		public:
			void Execute(Machine& machine, double arg) override;
			std::string Transcribe(const Machine& machine, double arg) const override;

			double Get_Cost() const override { return 1.0; }
	};

	class MulInstruction : public Instruction {
		public:
			void Execute(Machine& machine, double arg) override;
			std::string Transcribe(const Machine& machine, double arg) const override;

			double Get_Cost() const override { return 1.5; }
	};

	class DivInstruction : public Instruction {
		public:
			void Execute(Machine& machine, double arg) override;
			std::string Transcribe(const Machine& machine, double arg) const override;

			double Get_Cost() const override { return 1.5; }
	};

	class Machine {
		private:
			const MachineFeatures mFeatures;

			std::vector<double> mRegisters;

			std::vector<std::unique_ptr<DataSource>> mSources;
			std::vector<std::unique_ptr<Instruction>> mInstruction_Handlers;

			double mExecution_Cost = 0.0;

		protected:
			void Prepare_Sources(const std::vector<double>& input, const std::vector<double>& memory, std::vector<double>& output);
			void Prepare_Instructions();

		public:
			Machine(const MachineFeatures& features = MachineFeatures());

			// memory = constants + instructions
			std::vector<double> Run(const std::vector<double>& input, const std::vector<double>& memory);
			std::vector<std::string> Transcribe(const std::vector<double>& memory);

			double Read(const size_t index) const;
			void Write(const size_t index, double value);

			std::pair<size_t, double> Decode_Instruction(double encoded) const;
			std::pair<size_t, double> Decode_Operand(double encoded) const;

			const std::string& Get_Source_Name(size_t index) const {
				if (index < mSources.size()) {
					return mSources[index]->Get_Name();
				}
				return mSources[0]->Get_Name(); // should never happen
			}

			double Get_Execution_Cost() const { return mExecution_Cost; }
	};

}
