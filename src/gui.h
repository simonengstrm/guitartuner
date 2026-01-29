#pragma once

#include <mutex>

#include "freq_analysis.h"
#include "raylib.h"

class GUI {
 public:
  GUI() = default;
  ~GUI() = default;

  void initialize();

  void mainLoop();

  void setNewSpectrumData(const FFTData& newSpectrum) {
    std::lock_guard<std::mutex> lock(spectrumMutex);
    spectrum = newSpectrum;
  }

 private:
  FFTData spectrum{};
  std::mutex spectrumMutex;
};
