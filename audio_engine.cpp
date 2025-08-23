#include <chrono>
#include <string>
#include <iostream>
#include <thread>
#include <semaphore>

#include "portaudio.h"

struct AudioEngine {

    bool initialized = false;
    PaStream* inStream = nullptr;
    PaStreamParameters inStreamParameters {};
    float* buffer = nullptr;

    // Function for handling buffer full
    bool init() {
        PaError err = Pa_Initialize();
        if (err != paNoError) {
            std::cout << "Could not initialize audio engine: " << Pa_GetErrorText(err) << std::endl;
            return false;
        }
        
        initialized = true;
        return true;
    }

    int findDevice(std::string deviceNameHint) {
        int deviceCount = Pa_GetDeviceCount();
        for (int i = 0; i < deviceCount; i++) {
            const PaDeviceInfo *deviceInfo = Pa_GetDeviceInfo(i);
            std::string deviceName(deviceInfo->name);
            if (deviceName.contains(deviceNameHint) && deviceInfo->maxInputChannels > 0) return i;
        }
        
        return paNoDevice;
    }

    bool openStream(int deviceIndex, unsigned long framesPerBuffer) {
        if (inStream) {
            return true;
        }

        buffer = (float*) malloc(framesPerBuffer * sizeof(float));

        const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(deviceIndex);
        inStreamParameters.device = deviceIndex;
        inStreamParameters.channelCount = deviceInfo->maxInputChannels;
        inStreamParameters.sampleFormat = paFloat32;
        inStreamParameters.suggestedLatency = deviceInfo->defaultLowInputLatency;
        inStreamParameters.hostApiSpecificStreamInfo = nullptr;

        PaError err = Pa_OpenStream(&inStream, &inStreamParameters, nullptr, deviceInfo->defaultSampleRate, framesPerBuffer, paClipOff, 
            &AudioEngine::paRecordCallback, this);
        if (err != paNoError) {
            std::cout << Pa_GetErrorText(err);
            return false;
        }

        return true;
    }

    bool start() {
        return inStream && Pa_StartStream(inStream) == paNoError;
    }

    bool stop() {
        return inStream && Pa_StopStream(inStream) == paNoError;
    }

    ~AudioEngine() {
        if (inStream) {
            Pa_StopStream(inStream);
            Pa_CloseStream(inStream);
        }
        if (initialized) {
            Pa_Terminate();
        }

        free(buffer);
    }

private:
    static int paRecordCallback(const void *inputBuffer,
                                void *outputBuffer, 
                                unsigned long framesPerBuffer,
                                const PaStreamCallbackTimeInfo *timeInfo, 
                                PaStreamCallbackFlags statusFlags, 
                                void *userData) {

        AudioEngine* self = (AudioEngine*) userData;
        const float *in = (float*) inputBuffer;

        // Extract data from channel 2;
        for (int i = 0; i < framesPerBuffer; i++) {
            self->buffer[i] = in[i * 2 + 1];
        }
        
        return paComplete;
    }

};