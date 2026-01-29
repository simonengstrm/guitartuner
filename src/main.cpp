#include <raylib.h>

#include <algorithm>
#include <cmath>
#include <csignal>
#include <iomanip>
#include <iostream>
#include <thread>

#include "audio_engine.h"
#include "freq_analysis.h"
#include "gui.h"

using std::cout;

bool shutdown{false};

void signalHandler([[maybe_unused]] int signum) { shutdown = true; }

void printTuner(const NoteInfo& info, float freq) {
  int totalWidth = 50;
  std::string bar(totalWidth, '-');

  cout << bar << "\t                     \r";

  if (info.midi != -1) {
    int centerPosition = totalWidth / 2;
    int centsOffset = static_cast<int>(info.cents / 100.0f * (totalWidth / 2.0));
    int markerPosition = centerPosition + centsOffset;
    if (markerPosition < 0) markerPosition = 0;
    if (markerPosition >= totalWidth) markerPosition = totalWidth - 1;

    bar[totalWidth / 2] = '|';
    bar[markerPosition] = '*';
    cout << bar << "\t" << info.name << info.octave << "\t" << std::fixed << std::setprecision(0)
         << freq << "\r";
  }

  std::cout << std::flush;
}

int main(int argc, char* argv[]) {
  signal(SIGINT, signalHandler);
  signal(SIGTERM, signalHandler);

  GUI gui{};
  gui.initialize();

  AudioEngine engine{};
  std::string deviceName = argc > 1 ? argv[1] : "Scarlett";
  if (!engine.init(deviceName)) {
    std::cout << "Could not initialize audio engine\n";
    return -1;
  }

  auto tunerCallback = [&](const std::array<SAMPLE, SAMPLES_PER_FFT> buffer,
                           unsigned long bufferSize, [[maybe_unused]] int sampleRate) {
    // if (findMaxAmplitude(buffer.data(), bufferSize) < 0.01f) {  // Threshold to avoid noise
    //   return;
    // }

    std::array<SAMPLE, SAMPLES_PER_FFT> copiedBuffer{};
    std::copy(buffer.begin(), buffer.end(), copiedBuffer.begin());

    hannWindow(copiedBuffer.data(), copiedBuffer.size());
    FFTData fftOutput{};
    fft(copiedBuffer.data(), copiedBuffer.size(), fftOutput);
    gui.setNewSpectrumData(fftOutput);
    // float frequency = findPeakFrequency(fftOutput, sampleRate);
    // NoteInfo note = freqToNote(frequency);
    // printTuner(note, frequency);
  };

  if (!engine.openStream()) {
    std::cout << "Could not open audio stream\n";
    return -1;
  }

  engine.setAudioCallback(tunerCallback);

  if (!engine.start()) {
    std::cout << "Could not start audio stream\n";
    return -1;
  }

  gui.mainLoop();

  if (!engine.stop()) {
    std::cout << "Could not stop audio stream\n";
    return -1;
  }
}
