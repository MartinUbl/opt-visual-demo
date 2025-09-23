/**
 * OptVisualDemo
 *
 * Copyright (c) 2025-present, Martin Ubl
 * Distributed under the MIT license
 */

#include "Fourier2D.h"

#include <numbers>
#include <thread>

#include "../Core/DrawProxy.h"
#include "../Core/Optimizer.h"
#include "../Core/Helpers.h"

#include "../Optimizers/GeneticAlgorithm.h"

namespace {

	bool gen_audioDeviceInitialized = false;

	constexpr size_t playbackTimeSecs = 4;
	constexpr unsigned int sampleRate = 44100;
	constexpr double sampleRateFloat = static_cast<double>(sampleRate);

	size_t gen_numHarmonics = 3;
	std::vector<double> gen_bestCandidate;
	double maxAmplitude = 1.0;

	AudioStream stream;
	int gen_time = 0;

	std::vector<short> gen_outBuffer;

	void generateSamples(void* buffer, unsigned int frames) {

		short* samples = (short*)buffer;

		for (unsigned int i = 0; i < frames; i++) {
			if (gen_time < (int)gen_outBuffer.size()) {
				samples[i] = gen_outBuffer[gen_time];
			}
			else {
				samples[i] = 0;
			}
			gen_time++;
		}
	}
}

void Fourier2D::Play_Sound() {

	if (mCandidates.size() == 0) {
		return;
	}

	if (gen_time != 0) {
		// already playing
		return;
	}

	gen_numHarmonics = mNum_Harmonics;
	gen_bestCandidate = mCandidates[0];
	maxAmplitude = 0.0;
	for (size_t n = 0; n < mNum_Harmonics; n++) {
		const double an = gen_bestCandidate[n * 3 + 0];
		const double bn = gen_bestCandidate[n * 3 + 1];
		maxAmplitude += std::abs(an) + std::abs(bn);
	}

	gen_outBuffer.resize(sampleRate * playbackTimeSecs);

	for (unsigned int i = 0; i < sampleRate * playbackTimeSecs; i++) {
		double wavepoint = 0.0;
		for (size_t n = 0; n < gen_numHarmonics; n++) {
			if (gen_bestCandidate.size() > 0) {
				const double an = gen_bestCandidate[n * 3 + 0];
				const double bn = gen_bestCandidate[n * 3 + 1];
				const double wn = gen_bestCandidate[n * 3 + 2] * 150.0;
				wavepoint += an * std::cos(wn * ((gen_time / sampleRateFloat) * 2.0 * std::numbers::pi)) + bn * std::sin(wn * ((gen_time / sampleRateFloat) * 2.0 * std::numbers::pi));
			}
		}
		wavepoint /= maxAmplitude; // normalize to -1..1
		gen_outBuffer[i] = (short)(wavepoint * 32767);
		gen_time++;
	}

	gen_time = 0;

	if (!gen_audioDeviceInitialized) {
		gen_audioDeviceInitialized = true;
		// make sure the audio device is initialized only once
		// we will close it when the sound playback is done
		// raylib does not support re-initialization of the audio device
		InitAudioDevice();
	}

	std::thread thr([]() {
		// play sound using raylib and callback, that samples the best candidate
		stream = LoadAudioStream(sampleRate, 16, 1);
		SetAudioStreamBufferSizeDefault(4096);
		SetAudioStreamCallback(stream, generateSamples);
		PlayAudioStream(stream);

		std::this_thread::sleep_for(std::chrono::seconds(playbackTimeSecs));
		gen_time = 0;
		UnloadAudioStream(stream);
	});
	thr.detach();
}

Vector2 Fourier2D::From_Screen_To_Cartesian(const Vector2& screenPoint) const {
	// X is from 0 to 4*PI (left to right)
	// Y is from -10 to 10 (bottom to top) with 0 in the middle of the screen
	const float x = (screenPoint.x / GetScreenWidth()) * static_cast<float>(4.0 * std::numbers::pi);
	const float y = ((GetScreenHeight() / 2.0f) - screenPoint.y) / (GetScreenHeight() / 20.0f);
	return { x, y };
}

Vector2 Fourier2D::From_Cartesian_To_Screen(const Vector2& cartesianPoint) const {
	const float x = (cartesianPoint.x / static_cast<float>(4.0 * std::numbers::pi)) * GetScreenWidth();
	const float y = (GetScreenHeight() / 2.0f) - (cartesianPoint.y * (GetScreenHeight() / 20.0f));
	return { x, y };
}

bool Fourier2D::On_Init() {
	mData_Points.clear();
	mName = "Fourier 2D";
	mDescription = "A simple harmonics (N = " + std::to_string(mNum_Harmonics) + ") model fitting.";
	return true;
}

bool Fourier2D::On_Cleanup() {
	mData_Points.clear();
	if (mOptimization_Thread && mOptimization_Thread->joinable()) {
		mOptimization_Thread->join();
	}
	if (gen_audioDeviceInitialized) {
		// close the audio device if it was initialized
		gen_audioDeviceInitialized = false;
		CloseAudioDevice();
	}
	return true;
}

bool Fourier2D::On_Update(float delta_time) {
	return true;
}

void Fourier2D::Reset_Data() {
	Experiment::Reset_Data();
	mData_Points.clear();
}

bool Fourier2D::On_Render() {

	TSimple_Button playBtn(GetScreenWidth() - 10 - 100, 130, 100, 30, "Play Sound");
	if (playBtn.Render()) {
		Play_Sound();
	}
	else {
		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !Is_Mouse_In_UI_Area()) {
			mData_Points.push_back(From_Screen_To_Cartesian(GetMousePosition()));
		}
	}

	for (const auto& point : mData_Points) {
		DrawCircleV(From_Cartesian_To_Screen(point), 3, RED);
	}

	return Experiment::On_Render();
}

void Fourier2D::Draw_Candidate(const std::vector<double>& candidate, bool best) {
	if (candidate.size() == mNum_Harmonics * 3) {

		Vector2 prevPoint = { 0, 0 };

		for (double x = 0; x <= 4.0 * std::numbers::pi; x += 0.01) {
			double y = 0.0f;
			for (size_t n = 0; n < mNum_Harmonics; n++) {
				const double an = candidate[n * 3 + 0];
				const double bn = candidate[n * 3 + 1];
				const double wn = candidate[n * 3 + 2];
				y += an * std::cos(wn * x) + bn * std::sin(wn * x);
			}
			const Vector2 screenPoint = From_Cartesian_To_Screen({ static_cast<float>(x), static_cast<float>(y) });
			if (x != 0) {
				DrawLineEx(prevPoint, screenPoint, best ? 2.0f : 1.0f, best ? BLUE : LIGHTGRAY);
			}
			prevPoint = screenPoint;
		}
	}
}

bool Fourier2D::Check_Can_Optimize() {
	return mData_Points.size() >= 5;
}

bool Fourier2D::Draw_Cannot_Optimize_Reason(int hintPositionX, int hintPositionY) {
	if (mData_Points.size() < 5) {
		DrawProxy::Text("Add at least 5 points to start optimization", hintPositionX, hintPositionY, DARKGRAY, NAppFont::RegularText);
		return true;
	}

	return false;
}

void Fourier2D::Fill_Optimizer_Setup(TOptimizer_Setup& setup) {
	setup.maxIterations = 80000;
	setup.populationSize = 50;

	setup.lowerBounds.resize(mNum_Harmonics * 3);
	setup.upperBounds.resize(mNum_Harmonics * 3);
	setup.sensitivity.resize(mNum_Harmonics * 3);

	setup.initialGuess.resize(mNum_Harmonics * 3);

	for (size_t n = 0; n < mNum_Harmonics; n++) {
		// an
		setup.lowerBounds[n * 3 + 0] = -10.0;
		setup.upperBounds[n * 3 + 0] = 10.0;
		setup.sensitivity[n * 3 + 0] = 5.0;
		setup.initialGuess[n * 3 + 0] = 0.0;
		// bn
		setup.lowerBounds[n * 3 + 1] = -10.0;
		setup.upperBounds[n * 3 + 1] = 10.0;
		setup.sensitivity[n * 3 + 1] = 5.0;
		setup.initialGuess[n * 3 + 1] = 0.0;
		// wn
		setup.lowerBounds[n * 3 + 2] = 0.1;
		setup.upperBounds[n * 3 + 2] = 10.0;
		setup.sensitivity[n * 3 + 2] = 1.0;
		setup.initialGuess[n * 3 + 2] = (n + 1) * 1.0;
	}
}

double Fourier2D::Objective_Function(const std::vector<double>& parameters) {
	if (parameters.size() != mNum_Harmonics * 3) {
		throw std::invalid_argument("Expected Num_Harmonics * 3 parameters: an, bn, wn for each harmonic");
	}

	double totalError = 0.0;
	for (const auto& point : mData_Points) {
		const float x = point.x;
		const float yTrue = point.y;
		double yPred = 0.0;
		for (size_t n = 0; n < mNum_Harmonics; n++) {
			const double an = parameters[n * 3 + 0];
			const double bn = parameters[n * 3 + 1];
			const double wn = parameters[n * 3 + 2];
			yPred += an * std::cos(wn * x) + bn * std::sin(wn * x);
		}
		const double error = yPred - yTrue;
		totalError += error * error; // squared error
	}
	return totalError / mData_Points.size();
}
