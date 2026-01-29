#include "audio_engine.h"

#include <chrono>
#include <iostream>
#include <string>

#include "pa_ringbuffer.h"
#include "portaudio.h"

bool AudioEngine::init(std::string deviceNameHint) {
  PaError err = Pa_Initialize();
  if (err != paNoError) {
    std::cout << "Could not initialize audio engine: " << Pa_GetErrorText(err) << std::endl;
    return false;
  }

  if (deviceIndex = AudioEngine::findDevice(deviceNameHint); deviceIndex == paNoDevice) {
    std::cout << "Could not find audio device" << std::endl;
    return false;
  }

  if (PaUtil_InitializeRingBuffer(&ringBuffer, sizeof(SAMPLE), RING_BUFFER_SIZE,
                                  ringBufferData.data()) < 0) {
    std::cout << "Could not initialize ring buffer" << std::endl;
    return false;
  }

  initialized = true;
  return true;
}

int AudioEngine::findDevice(std::string deviceNameHint) {
  int deviceCount = Pa_GetDeviceCount();
  for (int i = 0; i < deviceCount; i++) {
    const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(i);
    std::string deviceName(deviceInfo->name);
    if (deviceName.contains(deviceNameHint) && deviceInfo->maxInputChannels > 0) return i;
  }

  return paNoDevice;
}

bool AudioEngine::openStream() {
  if (inStream) {
    return true;
  }
  const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(deviceIndex);

  inStreamParameters.device = deviceIndex;
  inStreamParameters.channelCount = deviceInfo->maxInputChannels;
  inStreamParameters.sampleFormat = PA_SAMPLE_TYPE;
  inStreamParameters.suggestedLatency = deviceInfo->defaultLowInputLatency;
  inStreamParameters.hostApiSpecificStreamInfo = NULL;
  std::cout << "default sample rate: " << deviceInfo->defaultSampleRate << std::endl;

  PaError err = Pa_OpenStream(&inStream, &inStreamParameters, NULL, deviceInfo->defaultSampleRate,
                              SAMPLES_PER_FFT, paClipOff, &AudioEngine::paRecordCallback, this);
  if (err != paNoError) {
    std::cout << Pa_GetErrorText(err);
    return false;
  }

  return true;
}

bool AudioEngine::start() {
  audioThread = std::jthread([this]() {
    while (audioThread.get_stop_token().stop_requested() == false) {
      std::array<SAMPLE, SAMPLES_PER_FFT> tempBuffer;
      if (PaUtil_GetRingBufferReadAvailable(&ringBuffer) >= SAMPLES_PER_FFT) {
        PaUtil_ReadRingBuffer(&ringBuffer, tempBuffer.data(), SAMPLES_PER_FFT * sizeof(SAMPLE));
        if (audioCallback) {
          audioCallback(tempBuffer, SAMPLES_PER_FFT,
                        static_cast<int>(getDeviceInfo()->defaultSampleRate));
        }
      } else {
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
      }
    }
  });

  return inStream && Pa_StartStream(inStream) == paNoError;
}

bool AudioEngine::stop() {
  if (audioThread.joinable()) {
    audioThread.request_stop();
    audioThread.join();
  }
  return inStream && Pa_StopStream(inStream) == paNoError;
}

bool AudioEngine::isActive() { return inStream && Pa_IsStreamActive(inStream) == 1; }

// This function is kind of tailored to my focusrite scarlett 2i2 which has 2 input channels
// where channel 0 is mic input and channel 1 is instrument input, so i skip channel 0
int AudioEngine::paRecordCallback(const void* inputBuffer, [[maybe_unused]] void* outputBuffer,
                                  unsigned long framesPerBuffer,
                                  [[maybe_unused]] const PaStreamCallbackTimeInfo* timeInfo,
                                  [[maybe_unused]] PaStreamCallbackFlags statusFlags,
                                  void* userData) {
  AudioEngine* self = static_cast<AudioEngine*>(userData);
  const SAMPLE* rptr = static_cast<const SAMPLE*>(inputBuffer);
  // Temporary buffer to store correct channel samples before writing to ring buffer
  std::vector<SAMPLE> tempBuffer(framesPerBuffer);
  SAMPLE* wptr = tempBuffer.data();
  for (unsigned long i = 0; i < SAMPLES_PER_FFT; i++) {
    rptr++;             // Skip first channel
    *wptr++ = *rptr++;  // Copy input to output
  }

  PaUtil_WriteRingBuffer(self->getRingBuffer(), tempBuffer.data(), framesPerBuffer);

  return paContinue;
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
