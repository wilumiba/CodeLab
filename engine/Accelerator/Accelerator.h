#ifndef ACCELERATOR_H
#define ACCELERATOR_H

#include <cstdint>
#include <vector>
#include "IAudioData.h"
#include "IAudioDataHandler.h"
#include "sonic.h"

class AudioAccelerator : public IAudioDataHandler{
public:
    AudioAccelerator(float speed);
    ~AudioAccelerator();

    bool handleAudioData(IAudioData& audioData);

private:
    float speed;
    int putCounts;
    void RunSonic(int16_t *inputData, int inputSize, int16_t *outputData, int outputSize, 
                    int sampleRate, int numChannels, float speed);
    void AppendAudioDataToWavFile(const char *filename, char *data, uint32_t len);
};

#endif // ACCELERATOR_H