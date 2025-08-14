#include <fstream>
#include <iostream>
#include "portaudio.h"
#include "freq_analysis.h"

#define SAMPLES_PER_FFT 4096

#define PA_SAMPLE_TYPE paFloat32
typedef float SAMPLE;

using std::cout, std::endl;

typedef struct bufData {
    SAMPLE* recording;
} inStreamData;

static int paRecordCallback(const void *inputBuffer, void *outputBuffer,
                     unsigned long framesPerBuffer,
                     const PaStreamCallbackTimeInfo *timeInfo,
                     PaStreamCallbackFlags statusFlags,
                     void *userData) {
    inStreamData *data = (inStreamData*)userData;
    const SAMPLE *rptr = (const SAMPLE*)inputBuffer;
    SAMPLE *wptr = &data->recording[0];

    for (unsigned long i = 0; i < SAMPLES_PER_FFT; i++) {
        *wptr++ = *rptr++; // Copy input to output
        *wptr++ = *rptr++;
    }

    return paComplete;
}

int main() {
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        cout << "PortAudio error: " << Pa_GetErrorText(err) << endl;
    } else {
        cout << "PortAudio terminated successfully." << endl;
    }

    /* ------------ RECORD AUDIO -------------*/
    PaDeviceIndex device_in = 3;
    const PaDeviceInfo *deviceInfo = Pa_GetDeviceInfo(device_in);
    cout << "Using device: " << deviceInfo->name << endl;

    inStreamData data;
    int numBytes = SAMPLES_PER_FFT * sizeof(SAMPLE) * 2; // Two channels
    data.recording = (SAMPLE*) malloc(numBytes);
    memset(data.recording, 0, numBytes);

    PaStreamParameters inStreamParameters;
    inStreamParameters.device = device_in;
    inStreamParameters.channelCount = 2; // Dual channel input but only one channel is used
    inStreamParameters.sampleFormat = PA_SAMPLE_TYPE;
    inStreamParameters.suggestedLatency = deviceInfo->defaultLowInputLatency;
    inStreamParameters.hostApiSpecificStreamInfo = NULL;

    PaStream *inStream;
    err = Pa_OpenStream(&inStream, &inStreamParameters, NULL,
                        deviceInfo->defaultSampleRate, SAMPLES_PER_FFT, paClipOff,
                        paRecordCallback, &data);

    if (err != paNoError) {
        goto done;
    }

    while(true) {
        // Record audio then call fft on buffer while running
        err = Pa_StartStream(inStream);
        if (err != paNoError) {
            goto done;
        }

        while(Pa_IsStreamActive(inStream) == 1) {
            Pa_Sleep(10); // Sleep while recording
        }
        
        err = Pa_StopStream(inStream);
        if (err != paNoError) {
            goto done;
        }

        // Process the recorded data here (e.g., perform FFT)
        cout << freqAnalysis(data.recording, SAMPLES_PER_FFT, deviceInfo->defaultSampleRate) << " Hz" << endl;
    }

done:
    free(data.recording);
    if( err != paNoError )
    {
        fprintf( stderr, "An error occured while using the portaudio stream\n" );
        fprintf( stderr, "Error number: %d\n", err );
        fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
    }

    err = Pa_Terminate();
    if (err != paNoError) {
        cout << "PortAudio error: " << Pa_GetErrorText(err) << endl;
    } else {
        cout << "PortAudio terminated successfully." << endl;
    }

    return 0;
}