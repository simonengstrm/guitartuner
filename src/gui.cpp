#include "gui.h"

#include <iomanip>
#include <iostream>

void GUI::initialize() {
  InitWindow(800, 600, "Visualizer");
  SetTargetFPS(60);
}

void GUI::mainLoop() {
  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(BLACK);

    {
      std::lock_guard<std::mutex> lock(spectrumMutex);
      const unsigned long sampleRate = 192000;
      float peakFrequency = findPeakFrequency(spectrum, sampleRate);
      NoteInfo note = freqToNote(peakFrequency);

      // Draw frequency spectrum up to 5kHz
      const unsigned long min_f = 20;
      const unsigned long max_f = 5000;
      const unsigned long min_index =
          static_cast<unsigned long>(static_cast<float>(min_f) / static_cast<float>(sampleRate) *
                                     static_cast<float>(spectrum.size()));
      const unsigned long max_index =
          static_cast<unsigned long>(static_cast<float>(max_f) / static_cast<float>(sampleRate) *
                                     static_cast<float>(spectrum.size()));
      const float width = 800.0f / static_cast<float>(max_index - min_index);
      for (unsigned long i = min_index; i < max_index; ++i) {
        float magnitude = std::abs(spectrum[i]);
        float dbMagnitude = 20.0f * log10f(magnitude + 1e-6f);  // Convert to dB scale
        float height = (dbMagnitude + 100.0f) * (600.0f / 100.0f);
        if (height < 0.0f) height = 0.0f;
        if (height > 600.0f) height = 600.0f;
        DrawRectangle(static_cast<int>((i - min_index) * width), 600 - static_cast<int>(height),
                      static_cast<int>(width), static_cast<int>(height), GREEN);
      }
    }

    EndDrawing();
  }
}
