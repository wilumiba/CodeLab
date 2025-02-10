#include "SoundToucher.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>
#include <algorithm>
#include <cstdint>
#include <fstream>

inline int saturate(float fvalue, float minval, float maxval)
{
    if (fvalue > maxval)
    {
        fvalue = maxval;
    }
    else if (fvalue < minval)
    {
        fvalue = minval;
    }
    return (int)fvalue;
}

AudioSoundToucher::AudioSoundToucher(float speed):
                                    speed(speed), putCounts(0) {}

AudioSoundToucher::~AudioSoundToucher() {}

void AudioSoundToucher::RunSoundTouch(int16_t *inputData, int inputSize, int16_t *outputData, int outputSize, 
                                      int sampleRate, int numChannels, float speed) {
    // Initialize SoundTouch object
    soundtouch::SoundTouch soundTouch;
    soundTouch.setSampleRate(sampleRate);
    soundTouch.setChannels(numChannels);
    soundTouch.setTempo(speed);  // Set speed factor

    // Process input data in chunks
    const int chunkSize = 1920;  // Number of samples to process at a time
    soundtouch::SAMPLETYPE buffer[chunkSize];
    int numSamplesOut;
    int outputIndex = 0;
    double conv = 1.0 / 32768.0;

    for (int i = 0; i < inputSize; i += chunkSize) {
        int chunkLength = std::min(chunkSize, inputSize - i);
        for (int j = 0; j < chunkLength; ++j) {
            buffer[j] = static_cast<soundtouch::SAMPLETYPE>(inputData[i + j] * conv);
        }
        soundTouch.putSamples(buffer, chunkLength / numChannels);
        putCounts++;
        //std::cout<<"put chunkLength:"<<chunkLength<<" numChannels:"<<numChannels<<std::endl;
        // Retrieve processed samples
        while ((numSamplesOut = soundTouch.receiveSamples(buffer, chunkSize / numChannels)) > 0) {
            if (outputIndex + numSamplesOut * numChannels > outputSize) {
                std::cerr<<"put outputIndex:"<<outputIndex<<" numSamplesOut:"<<numSamplesOut<<" numChannels:"<<numChannels<<" outputSize:"<<outputSize<<std::endl;
                return;
            }

            //std::cout<<buffer[0]<<" "<<buffer[1]<<std::endl;
            for (int j = 0; j < numSamplesOut * numChannels; ++j) {
                outputData[outputIndex + j] = saturate(buffer[j] * 32768.0f, -32768.0f, 32767.0f);
            }
            //std::cout<<"receive samples "<<numSamplesOut<<std::endl;
            outputIndex += numSamplesOut * numChannels;
        }
    }

    // Flush remaining samples from SoundTouch buffer
    soundTouch.flush();
    while ((numSamplesOut = soundTouch.receiveSamples(buffer, chunkSize / numChannels)) > 0) {
        if (outputIndex + numSamplesOut * numChannels > outputSize) {
            std::cerr<<"flush outputIndex:"<<outputIndex<<" numSamplesOut:"<<numSamplesOut<<" numChannels:"<<numChannels<<" outputSize:"<<outputSize<<std::endl;
            return;
        }
        for (int j = 0; j < numSamplesOut * numChannels; ++j) {
            outputData[outputIndex + j] = saturate(buffer[j] * 32768.0f, -32768.0f, 32767.0f);
        }
        outputIndex += numSamplesOut * numChannels;
    }

}

bool AudioSoundToucher::handleAudioData(IAudioData& audioData) {

    std::vector<int16_t> output;
    int inputSize = audioData.getDataSize();
    output.resize(static_cast<int>(std::ceil(inputSize/speed)));

    // Record the start time
    auto start = std::chrono::high_resolution_clock::now();

    RunSoundTouch(audioData.getDataPointer(), inputSize, output.data(), output.size(), 
                 audioData.getSampleRate(), audioData.getChannels(), speed);
    
    // Record the end time
    auto end = std::chrono::high_resolution_clock::now();

    // Calculate the duration in milliseconds
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Print the duration
    std::cout << "RunSoundTouch execution time: " << duration.count() << " milliseconds. put count: "<<putCounts<< std::endl;
    std::cout << "Length of output: " << output.size() << " input: "<< inputSize
              << " sample rate: "<<audioData.getSampleRate()<<" channel:"<<audioData.getChannels()<< std::endl;

    audioData.updateData(output);
    return true;
}
