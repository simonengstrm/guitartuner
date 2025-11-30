#include <csignal>
#include <iomanip>
#include <iostream>
#include <thread>

#include "audio_engine.h"
#include "freq_analysis.h"

using std::cout;

bool shutdown{false};

void signalHandler(int signum) { shutdown = true; }

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

int main() {
  signal(SIGINT, signalHandler);
  signal(SIGTERM, signalHandler);

  AudioEngine engine{};
  if (!engine.init("Scarlett")) {
    std::cout << "Could not initialize audio engine" << std::endl;
    return -1;
  }

  auto fftCallback = [](const std::array<SAMPLE, SAMPLES_PER_FFT> buffer,
                        unsigned long framesPerBuffer, int sampleRate) {
    float freq = signalToFreq(buffer.data(), framesPerBuffer, sampleRate);
    NoteInfo note = freqToNote(freq);
    printTuner(note, freq);
  };

  if (!engine.openStream()) {
    std::cout << "Could not open audio stream" << std::endl;
    return -1;
  }

  engine.setAudioCallback(fftCallback);

  if (!engine.start()) {
    std::cout << "Could not start audio stream" << std::endl;
    return -1;
  }

  while (engine.isActive() && !shutdown) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  if (!engine.stop()) {
    std::cout << "Could not stop audio stream" << std::endl;
    return -1;
  }
}
