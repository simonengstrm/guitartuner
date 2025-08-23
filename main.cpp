#include <iostream>

#include "portaudio.h"
#include "freq_analysis.h"
#include "audio_engine.h"
#include "config.h"

using std::cout;

void displayNote(const NoteInfo& info, float freq) {
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
    cout << bar << "\t" <<  freq << '\r' << std::flush;
}

int main() {
    AudioEngine engine{};
    bool success = engine.init();
    if (!success) {
        return -1;
    }

    int deviceIndex = engine.findDevice("Focusrite");
    PaDeviceInfo const *deviceInfo = Pa_GetDeviceInfo(deviceIndex);
    engine.openStream(deviceIndex);

    while (true) {
        auto started = engine.start();
        if (!started) {
            std::cout << "Could not start audio stream" << std::endl;
            return -1;
        }

        while(Pa_IsStreamActive(engine.inStream)) {
            Pa_Sleep(1);
        }

        auto stopped = engine.stop();
        if (!stopped) {
            std::cout << "Could not stop audio stream" << std::endl;
            return -1;
        }

        float freq = freqAnalysis(engine.buffer, SAMPLES_PER_FFT, deviceInfo->defaultSampleRate);
        NoteInfo note = freqToNote(freq);
        displayNote(note, freq);
    }

    return 0;
}
