#pragma once

#include <mutex>
#include <vector>

#include "freq_analysis.h"

constexpr unsigned GUI_WIDTH{800};
constexpr unsigned GUI_HEIGHT{600};

class GUI {
 public:
  GUI(unsigned long sampleRate) : sampleRate(sampleRate) {}
  ~GUI() = default;

  void initialize();

  void mainLoop();

  void setNewSpectrumData(const FFTData& newSpectrum) {
    std::lock_guard<std::mutex> lock(spectrumMutex);
    currentSpectrum = newSpectrum;
  }

 private:
  void UpdateSpectrogramData();
  void DrawSpectrogram(float min_f, float max_f);

  FFTData currentSpectrum{};
  std::mutex spectrumMutex;
  unsigned long sampleRate;

  // Scrolling spectrogram visualization data
  std::vector<std::vector<float>> spectrogramHistory;
  static constexpr size_t maxHistorySize = 300;
};
