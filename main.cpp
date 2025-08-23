#include <iostream>
#include <expected>
#include <ostream>
#include <stdlib.h>

#include "portaudio.h"
#include "freq_analysis.h"
#include "audio_engine.cpp"

#define SAMPLES_PER_FFT 2048

#define PA_SAMPLE_TYPE paFloat32
typedef float SAMPLE;

using std::cout, std::endl;

void displayNote(const NoteInfo& info) {
    // Display note and frequency delta as a bar graph
    // Note in the middle, and a marker to the left or right depending on cents
    int totalWidth = 50;
    int centerPosition = totalWidth / 2;
    int centsOffset = static_cast<int>(info.cents / 100.0f * (totalWidth / 2));
    int markerPosition = centerPosition + centsOffset;
    if (markerPosition < 0) markerPosition = 0;
    if (markerPosition >= totalWidth) markerPosition = totalWidth - 1;
    std::string leftBar(totalWidth / 2, '-');
    std::string rightBar(totalWidth / 2, '-');
    std::string bar = leftBar + info.name + rightBar;
    bar[markerPosition] = '*'; // Marker for cents
    cout << bar << '\r' << std::flush;

}

int main() {
    AudioEngine engine{};
    bool success = engine.init();
    if (!success) {
        return -1;
    }

    int deviceIndex = engine.findDevice("Focusrite");
    engine.openStream(deviceIndex, SAMPLES_PER_FFT);

    while (true) {
        engine.start();

        while(Pa_IsStreamActive(engine.inStream)) {
            Pa_Sleep(2);
        }

        engine.stop();

        float freq = freqAnalysis(engine.buffer, SAMPLES_PER_FFT, 48000);
        NoteInfo note = freqToNote(freq);
        cout << freq << " Hz, Note: " << note.name << note.octave
             << " Note freq " << note.noteFreq << ", Cents: " << note.cents << ", MIDI: " << note.midi << endl;
        displayNote(note);

    }

    return 0;
}

// typedef struct bufData {
//     SAMPLE* recording;
// } inStreamData;

// static int paRecordCallback(const void *inputBuffer, void *outputBuffer,
//                      unsigned long framesPerBuffer,
//                      const PaStreamCallbackTimeInfo *timeInfo,
//                      PaStreamCallbackFlags statusFlags,
//                      void *userData) {
//     inStreamData *data = (inStreamData*)userData;
//     const SAMPLE *rptr = (const SAMPLE*)inputBuffer;
//     SAMPLE *wptr = &data->recording[0];

//     for (unsigned long i = 0; i < SAMPLES_PER_FFT; i++) {
//         *wptr++ = *rptr++; // Copy input to output
//         *wptr++ = *rptr++;
//     }

//     return paComplete;
// }

// int main() {
//     PaError err = Pa_Initialize();
//     if (err != paNoError) {
//         cout << "PortAudio error: " << Pa_GetErrorText(err) << endl;
//     } else {
//         cout << "PortAudio terminated successfully." << endl;
//     }

//     PaDeviceIndex device_in = 3;
//     const PaDeviceInfo *deviceInfo = Pa_GetDeviceInfo(device_in);
//     cout << "Using device: " << deviceInfo->name << endl;

//     inStreamData data;
//     int numBytes = SAMPLES_PER_FFT * sizeof(SAMPLE) * 2; // Two channels
//     data.recording = (SAMPLE*) malloc(numBytes);
//     memset(data.recording, 0, numBytes);

//     PaStreamParameters inStreamParameters;
//     inStreamParameters.device = device_in;
//     inStreamParameters.channelCount = 2; // Dual channel input but only one channel is used
//     inStreamParameters.sampleFormat = PA_SAMPLE_TYPE;
//     inStreamParameters.suggestedLatency = deviceInfo->defaultLowInputLatency;
//     inStreamParameters.hostApiSpecificStreamInfo = NULL;

//     PaStream *inStream;
//     err = Pa_OpenStream(&inStream, &inStreamParameters, NULL,
//                         deviceInfo->defaultSampleRate, SAMPLES_PER_FFT, paClipOff,
//                         paRecordCallback, &data);

//     if (err != paNoError) {
//         goto done;
//     }

//     while(true) {
//         // Record audio then call fft on buffer while running
//         err = Pa_StartStream(inStream);
//         if (err != paNoError) {
//             goto done;
//         }

//         while(Pa_IsStreamActive(inStream) == 1) {
//             Pa_Sleep(1); // Sleep while recording
//         }
        
//         err = Pa_StopStream(inStream);
//         if (err != paNoError) {
//             goto done;
//         }

//         // Strip recording data to a single channel (channel 2)
//         SAMPLE *singleChannelData = (SAMPLE*) malloc(SAMPLES_PER_FFT * sizeof(SAMPLE));
//         for (int i = 0; i < SAMPLES_PER_FFT; i++){
//             singleChannelData[i] = data.recording[i * 2 + 1]; // Use channel 2
//         }

//         // Process the recorded data here (e.g., perform FFT)
//         float freq = freqAnalysis(singleChannelData, SAMPLES_PER_FFT, deviceInfo->defaultSampleRate);
//         NoteInfo note = freqToNote(freq);
//         cout << freq << " Hz, Note: " << note.name << note.octave
//              << " Note freq " << note.noteFreq << ", Cents: " << note.cents << ", MIDI: " << note.midi << endl;

//         free(singleChannelData);
//     }

// done:
//     if( err != paNoError )
//     {
//         fprintf( stderr, "An error occured while using the portaudio stream\n" );
//         fprintf( stderr, "Error number: %d\n", err );
//         fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
//     }

//     err = Pa_Terminate();
//     if (err != paNoError) {
//         cout << "PortAudio error: " << Pa_GetErrorText(err) << endl;
//     } else {
//         cout << "PortAudio terminated successfully." << endl;
//     }

//     return 0;
// }