#include <fstream>
#include <iostream>
#include <vector>
#include <cstring>
#include <fstream>

#include "AudioHelper.h"
#include "AudioData.h"

void AudioHelper::SaveAudioDataToWavFile(const std::string& filename, IAudioData& audioData) {
    std::ofstream outFile(filename, std::ios::binary | std::ios::trunc);
    if (!outFile) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }

    // Prepare WAV header
    WAVHeader header;
    std::memcpy(header.riff, "RIFF", 4);
    std::memcpy(header.wave, "WAVE", 4);
    std::memcpy(header.fmt, "fmt ", 4);
    header.subchunk1Size = 16;
    header.audioFormat = 1; // PCM
    header.numChannels = audioData.getChannels();
    header.sampleRate = audioData.getSampleRate();
    header.bitsPerSample = audioData.getSampleSize();
    header.byteRate = header.sampleRate * header.numChannels * header.bitsPerSample / 8;
    header.blockAlign = header.numChannels * header.bitsPerSample / 8;
    header.dataSize = audioData.getDataSize() * sizeof(int16_t);
    header.chunkSize = 36 + header.dataSize;
    std::memcpy(header.data, "data", 4);

    // Write WAV header to file
    outFile.write(reinterpret_cast<const char*>(&header), sizeof(WAVHeader));

    // Get audio data
    int16_t* dataPointer = audioData.getDataPointer();
    unsigned int dataSize = audioData.getDataSize() * sizeof(int16_t);

    // Write audio data to file
    outFile.write(reinterpret_cast<const char*>(dataPointer), dataSize);

    // Close the file
    outFile.close();
}