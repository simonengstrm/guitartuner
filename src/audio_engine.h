#pragma once
#include <iostream>

#include "portaudio.h"

constexpr unsigned SAMPLES_PER_FFT{2048};
constexpr unsigned PA_SAMPLE_TYPE{paFloat32};

typedef float SAMPLE;

struct AudioEngine {
  bool initialized = false;
  PaStream *inStream;
  PaStreamParameters inStreamParameters{};
  float *buffer = nullptr;

  bool init();

  int findDevice(std::string deviceNameHint);

  bool openStream(int deviceIndex);

  bool start();

  bool stop();

  ~AudioEngine();

 private:
  static int paRecordCallback(const void *inputBuffer,
                              void *outputBuffer,
                              unsigned long framesPerBuffer,
                              const PaStreamCallbackTimeInfo *timeInfo,
                              PaStreamCallbackFlags statusFlags,
                              void *userData);
};