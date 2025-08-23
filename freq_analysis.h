#pragma once
#include <string>
#include <portaudio.h>

#define M_PI 3.14159265358979323846

struct NoteInfo {
    std::string name;
    int octave;
    float cents;
    float noteFreq;
    int midi;
};

float freqAnalysis(const float *buffer, unsigned long bufferSize, int sampleRate);

NoteInfo freqToNote(float frequency);