#include "gui.h"

#include <algorithm>

#include "raylib.h"

void GUI::initialize() {
  InitWindow(GUI_WIDTH, GUI_HEIGHT, "Visualizer");
  SetTargetFPS(60);
}

void GUI::UpdateSpectrogramData() {
  std::vector<float> magnitudes;
  std::lock_guard<std::mutex> lock(spectrumMutex);

  const size_t fftSize = currentSpectrum.size();
  const size_t halfSize = fftSize / 2;
  magnitudes.resize(halfSize);

  for (unsigned long i = 0; i < halfSize; ++i) {
    float magnitude = std::abs(currentSpectrum[i]);
    float dbMagnitude = 20.0f * log10f(magnitude + 1e-6f);
    magnitudes[i] = dbMagnitude;
  }

  if (spectrogramHistory.size() >= maxHistorySize) {
    spectrogramHistory.erase(spectrogramHistory.begin());
  }

  spectrogramHistory.push_back(magnitudes);
}

void DrawGridLines(float min_f, float max_f) {
  std::vector<float> octaveFreqs = {55.0f, 110.0f, 220.0f, 440.0f, 880.0f, 1760.0f, 3520.0f};
  const float logMin = log10f(min_f);
  const float logMax = log10f(max_f);

  for (float freq : octaveFreqs) {
    if (freq < min_f || freq > max_f) continue;

    float yNorm = (log10f(freq) - logMin) / (logMax - logMin);
    float y = GUI_HEIGHT - (yNorm * GUI_HEIGHT);

    DrawLine(0, (int)y, GUI_WIDTH, (int)y, BROWN);

    // Optional label
    DrawText(TextFormat("%.0f Hz", freq), 5, (int)y - 14, 14, GRAY);
  }
}

void GUI::DrawSpectrogram(float min_f, float max_f) {
  const float logMin = log10f(min_f);
  const float logMax = log10f(max_f);

  const float xStep = (float)(GUI_WIDTH) / (float)maxHistorySize;
  for (size_t t = 0; t < spectrogramHistory.size(); ++t) {
    const auto& row = spectrogramHistory[t];
    float x = t * xStep;

    const size_t halfSize = row.size();
    const float fftSize = halfSize * 2.0f;

    for (size_t bin = 1; bin < halfSize - 1; ++bin) {
      float freq = bin * sampleRate / fftSize;
      float nextFreq = (bin + 1) * sampleRate / fftSize;

      if (freq < min_f || freq > max_f) continue;

      // ---- Log mapping ----
      float yNorm = (log10f(freq) - logMin) / (logMax - logMin);
      float yNextNorm = (log10f(nextFreq) - logMin) / (logMax - logMin);

      if (yNorm < 0.0f || yNorm > 1.0f) continue;

      float y = GUI_HEIGHT - (yNorm * GUI_HEIGHT);
      float yNext = GUI_HEIGHT - (yNextNorm * GUI_HEIGHT);

      float rectHeight = std::abs(yNext - y);
      if (rectHeight < 1.0f) rectHeight = 1.0f;

      // ---- dB scaling ----
      float db = row[bin];
      float intensity = (db + 100.0f) / 100.0f;
      intensity = std::clamp(intensity, 0.0f, 1.0f);

      Color c = ColorFromHSV(240.0f - intensity * 240.0f, 1.0f, intensity);

      DrawRectangle((int)x, (int)yNext, (int)xStep + 1, (int)rectHeight + 1, c);
    }
  }
}

void GUI::mainLoop() {
  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(BLACK);

    constexpr float min_f = 70.0f;
    constexpr float max_f = 4000.0f;

    UpdateSpectrogramData();

    DrawSpectrogram(min_f, max_f);
    DrawGridLines(min_f, max_f);

    EndDrawing();
  }
}