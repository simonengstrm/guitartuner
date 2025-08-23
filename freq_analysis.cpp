#include <algorithm>
#include <complex>
#include "freq_analysis.h"

void fft(std::complex<float>* data, unsigned long bufferSize) {
    int n = bufferSize;

    for (int i = 1, j = 0; i < n; i++) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) {
            j ^= bit;
        }
        j ^= bit;
        if (i < j) {
            std::swap(data[i], data[j]);
        }
    }

    for (int len = 2; len <= n; len <<= 1) {
        float angle = -2 * M_PI / len;
        std::complex<float> wlen(cos(angle), sin(angle));
        
        for (int i = 0; i < n; i += len) {
            std::complex<float> w(1);
            for (int j = 0; j < len / 2; j++) {
                std::complex<float> u = data[i + j];
                std::complex<float> v = data[i + j + len / 2] * w;
                data[i + j] = u + v;
                data[i + j + len / 2] = u - v;
                w *= wlen;
            }
        }
    }
}

float findPeakFrequency(const std::complex<float>* fftData, unsigned long bufferSize, int sampleRate) {
    float maxMagnitude = 0.0f;
    unsigned long peakIndex = 0;

    for (unsigned long i = 0; i < bufferSize / 2; ++i) {
        float magnitude = std::abs(fftData[i]);
        if (magnitude > maxMagnitude) {
            maxMagnitude = magnitude;
            peakIndex = i;
        }
    }

    // Neighboring magnitudes
    float magL = (peakIndex > 0) ? std::abs(fftData[peakIndex - 1]) : 0.0f;
    float magC = std::abs(fftData[peakIndex]);
    float magR = (peakIndex < bufferSize / 2 - 1) ? std::abs(fftData[peakIndex + 1]) : 0.0f;

    // Interpolation to find a more accurate peak
    float delta = 0.5f * (magL - magR) / (magL - 2 * magC + magR);

    float frequency = (peakIndex + delta) * sampleRate / bufferSize;

    return frequency;
}

float freqAnalysis(const float *buffer, unsigned long bufferSize, int sampleRate) {
    // Only perform FFT if the amplitude is above a threshold
    float maxAmplitude = 0.0f;
    for (unsigned long i = 0; i < bufferSize; ++i) {
        maxAmplitude = std::max(maxAmplitude, std::abs(buffer[i]));
    }

    if (maxAmplitude < 0.01f) { // Threshold to avoid noise
        return 0.0f;
    }

    unsigned long paddedSize = 4096;

    std::complex<float>* fftInput = new std::complex<float>[paddedSize];
    for (unsigned long i = 0; i < paddedSize; ++i) {
        if (i < bufferSize) {
            fftInput[i] = std::complex<float>(buffer[i], 0.0f);
        } else {
            fftInput[i] = std::complex<float>(0.0f, 0.0f); // Zero padding
        }
    }

    fft(fftInput, paddedSize);

    float frequency = findPeakFrequency(fftInput, paddedSize, sampleRate);

    if (frequency < 50.0f || frequency > 2000.0f) {
        return 0.0f;
    }

    delete[] fftInput;

    return frequency;
}

NoteInfo freqToNote(float f) {
    static const char* NAMES[12] = {
        "C","C#","D","D#","E","F","F#","G","G#","A","A#","B"
    };

    if (!(f > 0.0f)) { // catches <=0, NaN
        return {"", 0, 0.0f, 0.0f, -1};
    }

    float m = 69.0f + 12.0f * std::log2(f / 440.0f);
    if (!std::isfinite(m)) {
        return {"", 0, 0.0f, 0.0f, -1};
    }

    int midi = (int)std::lround(m);
    midi = std::clamp(midi, 0, 127);  // MIDI standard range

    int idx  = midi % 12;
    int oct  = midi / 12 - 1;

    float noteFreq = 440.0f * std::pow(2.0f, (midi - 69) / 12.0f);
    float cents = 1200.0f * std::log2(f / noteFreq);

    return {std::string(NAMES[idx]), oct, cents, noteFreq, midi};
}
