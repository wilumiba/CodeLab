#ifndef AUDIOSOUNDTOUCHER_H
#define AUDIOSOUNDTOUCHER_H

#include <cstdint>
#include <vector>
#include "IAudioData.h"
#include "IAudioDataHandler.h"
#include <SoundTouch.h>

class AudioSoundToucher : public IAudioDataHandler{
public:
    AudioSoundToucher(float speed);
    ~AudioSoundToucher();

    bool handleAudioData(IAudioData& audioData);

private:
    float speed;
    int putCounts;
    void RunSoundTouch(int16_t *inputData, int inputSize, int16_t *outputData, int outputSize, 
                       int sampleRate, int numChannels, float speed);
};

#endif // AUDIOSOUNDTOUCHER_H