#pragma once
#include <portaudio.h>

#include <array>
#include <complex>
#include <string>

constexpr unsigned long paddedSize = 16384;
using FFTData = std::array<std::complex<float>, paddedSize>;

float signalToFreq(float* buffer, unsigned long bufferSize, int sampleRate);

void hannWindow(float* buffer, unsigned long bufferSize);

void fft(const float* buffer, unsigned long bufferSize, FFTData& output);

float findPeakFrequency(const std::array<std::complex<float>, paddedSize>& fftData, int sampleRate);

void harmonicProductSpectrum(FFTData& fftData, unsigned long factor);

float findMaxAmplitude(const float* buffer, unsigned long bufferSize);

struct NoteInfo {
  std::string name;
  int octave;
  float cents;
  float noteFreq;
  int midi;
};
NoteInfo freqToNote(float frequency);