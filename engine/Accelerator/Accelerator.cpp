#include "Accelerator.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>
#include <algorithm>
#include <cstdint>
#include <fstream>


#define BUFFER_SIZE 1920

void AudioAccelerator::RunSonic(int16_t *inputData, int inputSize, int16_t *outputData, int outputSize, 
                                 int sampleRate, int numChannels, float speed) {

    sonicStream stream;
    int16_t inBuffer[BUFFER_SIZE], outBuffer[BUFFER_SIZE];
    int shortsToRead, samplesWritten, shortsToWrite;
    int inputedShorts = 0;
    int outputedShorts = 0;

    stream = sonicCreateStream(sampleRate, numChannels);
    sonicSetSpeed(stream, speed);
    do {
        //calculate how many samples remaining to read
        if (inputedShorts >= inputSize) {
            shortsToRead = 0;
        } else if (inputSize - inputedShorts < BUFFER_SIZE) {
            shortsToRead = inputSize - inputedShorts;
        } else {
            shortsToRead = BUFFER_SIZE;
        }
        
        if (shortsToRead == 0) {
            sonicFlushStream(stream);
        } else {
            std::copy(inputData + inputedShorts, inputData + inputedShorts + shortsToRead, inBuffer);
            inputedShorts += shortsToRead;
            sonicWriteShortToStream(stream, inBuffer, shortsToRead / numChannels);
            putCounts++;
            //AppendAudioDataToWavFile("input.wav", (char*)inBuffer, shortsToRead * sizeof(int16_t));
        }

        do {
            samplesWritten = sonicReadShortFromStream(stream, outBuffer, BUFFER_SIZE / numChannels);
            if (samplesWritten > 0) {
                shortsToWrite = samplesWritten * numChannels;
                if ((outputedShorts + shortsToWrite) <= outputSize) {
                    std::copy(outBuffer, outBuffer + shortsToWrite, outputData + outputedShorts);
                }
                outputedShorts += shortsToWrite;
                //AppendAudioDataToWavFile("output.wav", (char*)outBuffer, shortsToWrite * sizeof(int16_t));
            }
        } while (samplesWritten > 0);
    } while (shortsToRead > 0);
    sonicDestroyStream(stream);
}

void AudioAccelerator::AppendAudioDataToWavFile(const char *filename, char *data, uint32_t len)
{
    std::ifstream infile(filename, std::ios::binary);
    bool fileExists = infile.good();
    infile.close();

    if (!fileExists) {
        std::ofstream outfile(filename, std::ios::binary);
        if (!outfile) {
            printf("Failed to create WAV file: %s", filename);
            return;
        }

        printf("Creating WAV file: %s", filename);
        // Write WAV header
        uint32_t fileSize = len + 36;
        uint16_t audioFormat = 1; // PCM
        uint16_t numChannels = 2; // Stereo
        uint32_t sampleRate = 48000;
        uint16_t bitsPerSample = 16;
        uint32_t byteRate = sampleRate * numChannels * bitsPerSample / 8;
        uint16_t blockAlign = numChannels * bitsPerSample / 8;

        outfile.write("RIFF", 4);
        outfile.write(reinterpret_cast<const char*>(&fileSize), 4);
        outfile.write("WAVE", 4);
        outfile.write("fmt ", 4);

        uint32_t subChunk1Size = 16;
        outfile.write(reinterpret_cast<const char*>(&subChunk1Size), 4);
        outfile.write(reinterpret_cast<const char*>(&audioFormat), 2);
        outfile.write(reinterpret_cast<const char*>(&numChannels), 2);
        outfile.write(reinterpret_cast<const char*>(&sampleRate), 4);
        outfile.write(reinterpret_cast<const char*>(&byteRate), 4);
        outfile.write(reinterpret_cast<const char*>(&blockAlign), 2);
        outfile.write(reinterpret_cast<const char*>(&bitsPerSample), 2);

        outfile.write("data", 4);
        uint32_t subChunk2Size = len;
        outfile.write(reinterpret_cast<const char*>(&subChunk2Size), 4);

        outfile.close();
    }

    // Append audio data to the file
    std::ofstream outfile(filename, std::ios::binary | std::ios::app);
    if (!outfile) {
        printf("Failed to open WAV file for appending: %s", filename);
        return;
    }
    outfile.write(data, len);
    outfile.close();

    if (fileExists) {
        // Open the file in binary mode to calculate the current file size
        std::ifstream fileSizeStream(filename, std::ios::binary | std::ios::ate);
        if (!fileSizeStream) {
            printf("Failed to open WAV file for size calculation: %s", filename);
            return;
        }
        uint32_t currentFileSize = fileSizeStream.tellg();
        fileSizeStream.close();

        // Update the WAV header with the new file size and data chunk size
        std::fstream wavFile(filename, std::ios::in | std::ios::out | std::ios::binary);
        if (!wavFile) {
            printf("Failed to open WAV file for updating header: %s", filename);
            return;
        }

        // Move to the file size position and update it
        wavFile.seekp(4, std::ios::beg);
        uint32_t fileSize = currentFileSize + len - 8;
        wavFile.write(reinterpret_cast<const char*>(&fileSize), 4);

        // Move to the data chunk size position and update it
        wavFile.seekp(40, std::ios::beg);
        uint32_t subChunk2Size = currentFileSize + len - 44;
        wavFile.write(reinterpret_cast<const char*>(&subChunk2Size), 4);

        wavFile.close();
    }
}

AudioAccelerator::AudioAccelerator(float speed):
                                    speed(speed), putCounts(0) {}

AudioAccelerator::~AudioAccelerator() {}


bool AudioAccelerator::handleAudioData(IAudioData& audioData) {

    std::vector<int16_t> output;
    int inputSize = audioData.getDataSize();
    output.resize(static_cast<int>(std::ceil(inputSize/speed)));

    // Record the start time
    auto start = std::chrono::high_resolution_clock::now();

    RunSonic(audioData.getDataPointer(), inputSize, output.data(), output.size(), 
                audioData.getSampleRate(), audioData.getChannels(), speed);

    // Record the end time
    auto end = std::chrono::high_resolution_clock::now();

    // Calculate the duration in milliseconds
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Print the duration
    std::cout << "RunSonic execution time: " << duration.count() << " milliseconds. put count: "<<putCounts<< std::endl;
    std::cout << "Length of output: " << output.size() << " input: "<< inputSize
              << " sample rate: "<<audioData.getSampleRate()<<" channel:"<<audioData.getChannels()<< std::endl;
    
    audioData.updateData(output);
    return true;
}
