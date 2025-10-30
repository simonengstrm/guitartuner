#include "audio_engine.h"

#include <chrono>
#include <iostream>
#include <semaphore>
#include <string>
#include <thread>

#include "portaudio.h"

bool AudioEngine::init(std::string deviceNameHint) {
  PaError err = Pa_Initialize();
  if (err != paNoError) {
    std::cout << "Could not initialize audio engine: " << Pa_GetErrorText(err)
              << std::endl;
    return false;
  }

  if (deviceIndex = AudioEngine::findDevice(deviceNameHint); deviceIndex == paNoDevice) {
    std::cout << "Could not find audio device" << std::endl;
    return false;
  }

  initialized = true;
  return true;
}

int AudioEngine::findDevice(std::string deviceNameHint) {
  int deviceCount = Pa_GetDeviceCount();
  for (int i = 0; i < deviceCount; i++) {
    const PaDeviceInfo *deviceInfo = Pa_GetDeviceInfo(i);
    std::string deviceName(deviceInfo->name);
    if (deviceName.contains(deviceNameHint) && deviceInfo->maxInputChannels > 0)
      return i;
  }

  return paNoDevice;
}

bool AudioEngine::openStream() {
  if (inStream) {
    return true;
  }
  const PaDeviceInfo *deviceInfo = Pa_GetDeviceInfo(deviceIndex);

  memset(buffer.data(), 0, buffer.size() * sizeof(SAMPLE));

  inStreamParameters.device = deviceIndex;
  inStreamParameters.channelCount = deviceInfo->maxInputChannels;
  inStreamParameters.sampleFormat = PA_SAMPLE_TYPE;
  inStreamParameters.suggestedLatency = deviceInfo->defaultLowInputLatency;
  inStreamParameters.hostApiSpecificStreamInfo = NULL;

  PaError err = Pa_OpenStream(&inStream, &inStreamParameters, NULL,
                              deviceInfo->defaultSampleRate, SAMPLES_PER_FFT,
                              paClipOff, &AudioEngine::paRecordCallback, this);
  if (err != paNoError) {
    std::cout << Pa_GetErrorText(err);
    return false;
  }

  return true;
}

bool AudioEngine::start() {
  return inStream && Pa_StartStream(inStream) == paNoError;
}

bool AudioEngine::stop() {
  return inStream && Pa_StopStream(inStream) == paNoError;
}

bool AudioEngine::isActive() {
  return inStream && Pa_IsStreamActive(inStream) == 1;
}

AudioEngine::~AudioEngine() {
  if (inStream) {
    Pa_StopStream(inStream);
    Pa_CloseStream(inStream);
  }
  if (initialized) {
    Pa_Terminate();
  }
}

int AudioEngine::paRecordCallback(const void *inputBuffer, void *outputBuffer,
                                  unsigned long framesPerBuffer,
                                  const PaStreamCallbackTimeInfo *timeInfo,
                                  PaStreamCallbackFlags statusFlags,
                                  void *userData) {
  AudioEngine *self = (AudioEngine *)userData;
  const SAMPLE *rptr = (const SAMPLE *)inputBuffer;
  SAMPLE *wptr = &self->buffer[0];

  for (unsigned long i = 0; i < SAMPLES_PER_FFT; i++) {
    rptr++;             // Skip first channel
    *wptr++ = *rptr++;  // Copy input to output
  }

  return paComplete;
}
