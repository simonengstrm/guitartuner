#include <complex>
#include "freq_analysis.h"

#define M_PI 3.14159265358979323846

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
        float angle = 2 * M_PI / len;
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

float findPeakFrequenct(const std::complex<float>* fftData, unsigned long bufferSize, int sampleRate) {
    float maxMagnitude = 0.0f;
    unsigned long peakIndex = 0;

    for (unsigned long i = 0; i < bufferSize / 2; ++i) {
        float magnitude = std::abs(fftData[i]);
        if (magnitude > maxMagnitude) {
            maxMagnitude = magnitude;
            peakIndex = i;
        }
    }

    float frequency = (float)peakIndex * sampleRate / bufferSize;

    return frequency;
}

float freqAnalysis(const float *buffer, unsigned long bufferSize, int sampleRate) {
    std::complex<float> fftInput[bufferSize];
    for (unsigned long i = 0; i < bufferSize; ++i) {
        fftInput[i] = std::complex<float>(buffer[i], 0.0f);
    }

    fft(fftInput, bufferSize);

    float frequency = findPeakFrequenct(fftInput, bufferSize, sampleRate);

    if (frequency < 80.0f || frequency > 2000.0f) {
        return 0.0f;
    }

    return frequency;
}