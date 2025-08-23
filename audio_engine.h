#pragma once
#include <iostream>

#include "portaudio.h"

struct AudioEngine {

    bool initialized = false;
    PaStream* inStream;
    PaStreamParameters inStreamParameters {};
    float* buffer = nullptr;

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