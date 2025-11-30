#pragma once
#include <portaudio.h>

#include <string>

struct NoteInfo {
  std::string name;
  int octave;
  float cents;
  float noteFreq;
  int midi;
};

float signalToFreq(const float* buffer, unsigned long bufferSize, int sampleRate);

NoteInfo freqToNote(float frequency);