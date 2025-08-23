#include <fstream>
#include <iostream>
#include "portaudio.h"

#define PA_SAMPLE_TYPE paInt16
typedef short SAMPLE;

using std::cout, std::endl;

typedef struct bufData {
    SAMPLE* recording;
    int frameIndex;
    int maxFrameIndex;
} recordingData;

static int paRecordCallback(const void *inputBuffer, void *outputBuffer,
                     unsigned long framesPerBuffer,
                     const PaStreamCallbackTimeInfo *timeInfo,
                     PaStreamCallbackFlags statusFlags,
                     void *userData) {
    recordingData *data = (recordingData*)userData;
    const SAMPLE *rptr = (const SAMPLE*)inputBuffer;
    SAMPLE *wptr = &data->recording[data->frameIndex * 2];
    unsigned long framesLeft = data->maxFrameIndex - data->frameIndex;
    unsigned long framesToCalc;
    int finished = paContinue;

    if (framesLeft < framesPerBuffer) {
        framesToCalc= framesLeft; // Prevent overflow
        finished = paComplete;
    } else {
        framesToCalc = framesPerBuffer; // Process all requested frames
        finished = paContinue;
    }

    if (inputBuffer == NULL) {
        for (unsigned long i = 0; i < framesToCalc; i++) {
            *wptr++ = 0; // Zero out the output buffer
            *wptr++ = 0;
        }
    } else {
        for (unsigned long i = 0; i < framesToCalc; i++) {
            *wptr++ = *rptr++; // Copy input to output
            *wptr++ = *rptr++;
        }
    }

    data->frameIndex += framesToCalc;
    return finished;
}

static int paPlayCallback(const void *inputBuffer, void *outputBuffer,
                     unsigned long framesPerBuffer,
                     const PaStreamCallbackTimeInfo *timeInfo,
                     PaStreamCallbackFlags statusFlags,
                     void *userData) {
    recordingData *data = (recordingData*)userData;
    SAMPLE *rptr = &data->recording[data->frameIndex * 2];
    SAMPLE *wptr = (SAMPLE*)outputBuffer;
    unsigned long framesLeft = data->maxFrameIndex - data->frameIndex;

    int finished = paContinue;

    if (framesLeft < framesPerBuffer) {
        for (unsigned long i = 0; i < framesLeft; i++) {
            *wptr++ = *rptr++; // Copy remaining recorded data to output
            *wptr++ = *rptr++;
        }
        // Zero out the rest of the output buffer if not enough frames
        for (unsigned long i = framesLeft; i < framesPerBuffer; i++) {
            *wptr++ = 0; // Zero out the output buffer
            *wptr++ = 0;
        }
        data->frameIndex += framesLeft;
        finished = paComplete; // Mark as complete if no more frames left
    }

    for (unsigned long i = 0; i < framesPerBuffer; i++) {
        *wptr++ = *rptr++; // Copy recorded data to output
        *wptr++ = *rptr++;
    }
    data->frameIndex += framesPerBuffer;

    return finished;
}

int main() {
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        cout << "PortAudio error: " << Pa_GetErrorText(err) << endl;
    } else {
        cout << "PortAudio terminated successfully." << endl;
    }

    /* ------------ RECORD AUDIO -------------*/

    int record_time = 5; 

    PaDeviceIndex device_in = 3;
    const PaDeviceInfo *deviceInfo = Pa_GetDeviceInfo(device_in);
    cout << "Using device: " << deviceInfo->name << endl;

    recordingData data;
    data.frameIndex = 0;
    data.maxFrameIndex = record_time * deviceInfo->defaultSampleRate;
    int numSamples = data.maxFrameIndex * 2; 
    int numBytes = numSamples * sizeof(SAMPLE);
    data.recording = (SAMPLE*) malloc(numBytes);
    for (int i = 0; i < numSamples; i++) {
        data.recording[i] = 0; // Initialize recording buffer
    }

    PaStreamParameters recordParameters;
    recordParameters.device = device_in;
    recordParameters.channelCount = 2;
    recordParameters.sampleFormat = PA_SAMPLE_TYPE;
    recordParameters.suggestedLatency = deviceInfo->defaultLowInputLatency;
    recordParameters.hostApiSpecificStreamInfo = NULL;

    PaStream *inStream;
    err = Pa_OpenStream(&inStream, &recordParameters, NULL,
                        deviceInfo->defaultSampleRate, 256, paClipOff,
                        paRecordCallback, &data);

    if (err != paNoError) {
        goto done;
    }

    err = Pa_StartStream(inStream);
    if (err != paNoError) {
        goto done;
    }

    cout << "Recording for " << record_time << " seconds..." << endl;
    while( Pa_IsStreamActive(inStream) == 1 ) {
        Pa_Sleep(1000); // Sleep for 1 second
    }
    cout << "Recording complete." << endl;

    err = Pa_CloseStream(inStream);
    if (err != paNoError) goto done;

    /* --------------- PLAYBACK OF RECORDED AUDIO --------------*/
    data.frameIndex = 0; // Reset frame index for playback
    PaStream *playbackStream;
    PaStreamParameters playbackParameters;
    playbackParameters.device = Pa_GetDefaultOutputDevice();
    playbackParameters.channelCount = 2;
    playbackParameters.sampleFormat = PA_SAMPLE_TYPE;
    playbackParameters.suggestedLatency = deviceInfo->defaultLowOutputLatency;
    playbackParameters.hostApiSpecificStreamInfo = NULL;

    err = Pa_OpenStream(&playbackStream, NULL, &playbackParameters, deviceInfo->defaultSampleRate, 256, paClipOff,
                        paPlayCallback, &data);

    if (err != paNoError) {
        goto done;
    }

    err = Pa_StartStream(playbackStream);
    if (err != paNoError) {
        goto done;
    }
    cout << "Playing back recorded audio..." << endl;
    while( (err = Pa_IsStreamActive(playbackStream)) == 1 ) {
        Pa_Sleep(100);
    }
    cout << "Playback complete." << endl;
    err = Pa_CloseStream(playbackStream);
    if (err != paNoError) goto done;

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