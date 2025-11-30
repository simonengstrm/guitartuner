#include "freq_analysis.h"

#include <algorithm>
#include <array>
#include <complex>
#include <iostream>
#include <numbers>

constexpr unsigned long paddedSize = 16384;

void fft(std::array<std::complex<float>, paddedSize>& data) {
  const int n = data.size();
  for (int i = 1, j = 0; i < n; i++) {
    int bit = n >> 1;
    for (; j & bit; bit >>= 1) {
      j ^= bit;
    }

    j ^= bit;
    if (i < j) {
      std::swap(data[i], data[j]);
    }
  }

  for (int len = 2; len <= n; len <<= 1) {
    float angle = -2 * std::numbers::pi / len;
    std::complex<float> wlen(cos(angle), sin(angle));

    for (int i = 0; i < n; i += len) {
      std::complex<float> w(1);
      for (int j = 0; j < len / 2; j++) {
        std::complex<float> u = data[i + j];
        std::complex<float> v = data[i + j + len / 2] * w;
        data[i + j] = u + v;
        data[i + j + len / 2] = u - v;
        w *= wlen;
      }
    }
  }
}

size_t harmonicProductSpectrum(std::array<std::complex<float>, paddedSize>& fftData) {
  // inplace HPS
  const unsigned long originalSize = fftData.size();
  const unsigned long hpsSize = originalSize / 8;  // Reduce to 1/8th size
  for (unsigned long i = 0; i < hpsSize; ++i) {
    for (unsigned int harmonic = 2; harmonic <= 8; ++harmonic) {
      unsigned long index = i * harmonic;
      if (index < originalSize) {
        fftData[i] *= fftData[index];
      }
    }
  }
  return hpsSize;
}

float findPeakFrequency(const std::array<std::complex<float>, paddedSize>& fftData, int sampleRate,
                        size_t hpsSize) {
  const unsigned long bufferSize = fftData.size();
  float maxMagnitude = 0.0f;
  unsigned long peakIndex = 0;

  for (unsigned long i = 0; i < hpsSize; ++i) {
    float magnitude = std::abs(fftData[i]);
    if (magnitude > maxMagnitude) {
      maxMagnitude = magnitude;
      peakIndex = i;
    }
  }

  // Neighboring magnitudes
  float magL = (peakIndex > 0) ? std::abs(fftData[peakIndex - 1]) : 0.0f;
  float magC = std::abs(fftData[peakIndex]);
  float magR = (peakIndex < bufferSize / 2 - 1) ? std::abs(fftData[peakIndex + 1]) : 0.0f;

  // Interpolation to find a more accurate peak
  float delta = 0.5f * (magL - magR) / (magL - 2 * magC + magR);

  float frequency = (peakIndex + delta) * sampleRate / hpsSize;

  return frequency;
}

float signalToFreq(const float* buffer, unsigned long bufferSize, int sampleRate) {
  // Only perform FFT if the amplitude is above a threshold (noise gate)
  float maxAmplitude = 0.0f;
  for (unsigned long i = 0; i < bufferSize; ++i) {
    maxAmplitude = std::max(maxAmplitude, std::abs(buffer[i]));
  }

  if (maxAmplitude < 0.01f) {  // Threshold to avoid noise
    return 0.0f;
  }

  std::array<std::complex<float>, paddedSize> fftInput{};
  for (unsigned long i = 0; i < paddedSize; ++i) {
    if (i < bufferSize) {
      fftInput[i] = std::complex<float>(buffer[i], 0.0f);
    } else {
      fftInput[i] = std::complex<float>(0.0f, 0.0f);  // Zero padding
    }
  }

  fft(fftInput);
  // const auto hpsSize = harmonicProductSpectrum(fftInput);
  float frequency = findPeakFrequency(fftInput, sampleRate, fftInput.size());

  return frequency;
}

NoteInfo freqToNote(float f) {
  constexpr std::array<std::string_view, 12> NAMES = {"C",  "C#", "D",  "D#", "E",  "F",
                                                      "F#", "G",  "G#", "A",  "A#", "B"};

  if (f <= 0.0f) {
    return {"", 0, 0.0f, 0.0f, -1};
  }

  float m = 69.0f + 12.0f * std::log2(f / 440.0f);
  if (!std::isfinite(m)) {
    return {"", 0, 0.0f, 0.0f, -1};
  }

  int midi = (int)std::lround(m);
  midi = std::clamp(midi, 0, 127);  // MIDI standard range

  int idx = midi % 12;
  int oct = midi / 12 - 1;

  float noteFreq = 440.0f * std::pow(2.0f, (midi - 69) / 12.0f);
  float cents = 1200.0f * std::log2(f / noteFreq);

  return {std::string(NAMES[idx]), oct, cents, noteFreq, midi};
}