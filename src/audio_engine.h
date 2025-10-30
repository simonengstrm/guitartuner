#pragma once
#include <array>
#include <functional>
#include <iostream>
#include <memory>

#include "portaudio.h"

constexpr unsigned SAMPLES_PER_FFT{2048};
constexpr unsigned PA_SAMPLE_TYPE{paFloat32};

typedef float SAMPLE;

using engine_buf = std::array<SAMPLE, sizeof(SAMPLE) * SAMPLES_PER_FFT>;

struct AudioEngine {
  bool init(std::string deviceNameHint);

  static int findDevice(std::string deviceNameHint);

  bool openStream();

  bool start();

  bool stop();

  bool isActive();

  const PaDeviceInfo *getDeviceInfo() {
    return Pa_GetDeviceInfo(deviceIndex);
  }

  engine_buf &getBuffer() { return buffer; }

  ~AudioEngine();

 private:
  bool initialized = false;
  PaStream *inStream{nullptr};
  PaStreamParameters inStreamParameters{};
  engine_buf buffer{};
  int deviceIndex = -1;

  static int paRecordCallback(const void *inputBuffer,
                              void *outputBuffer,
                              unsigned long framesPerBuffer,
                              const PaStreamCallbackTimeInfo *timeInfo,
                              PaStreamCallbackFlags statusFlags,
                              void *userData);
};