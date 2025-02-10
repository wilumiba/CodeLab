#include "AudioData.h"
#include <iostream>
#include <fstream>

AudioData::AudioData(const std::string& filePath) {
    loadFromFile(filePath);
    encodedDataList.clear();
}

AudioData::~AudioData() {
    data.clear();
}

void AudioData::updateData(std::vector<int16_t> newData) {
    data.clear();
    data = std::move(newData);
}

int16_t* AudioData::getDataPointer() {
    return data.empty() ? nullptr : data.data();
}

unsigned int AudioData::getDataSize() const {
    return data.size();
}

int AudioData::getSampleRate() const {
    return header.sampleRate;
}

int AudioData::getChannels() const {
    return header.numChannels;
}

int AudioData::getSampleSize() const {
    return header.bitsPerSample;
}

void AudioData::loadFromFile(const std::string& filePath) {
    std::ifstream fileStream(filePath, std::ios::binary);

    if (!readWAVHeader(fileStream, header)) {
        std::cerr << "Error reading WAV data" << std::endl;
        return;
    }

    readWAVData(fileStream);
}

bool AudioData::readWAVHeader(std::ifstream& file, WAVHeader& header) {
    if (!file) {
        std::cerr << "null input file" << std::endl;
        return false;
    }

    file.read(reinterpret_cast<char*>(&header), sizeof(WAVHeader));
    if (!file) {
        std::cerr << "Error reading WAV header" << std::endl;
        return false;
    }

    if (std::strncmp(header.riff, "RIFF", 4) != 0 || std::strncmp(header.wave, "WAVE", 4) != 0) {
        std::cerr << "Invalid WAV file" << std::endl;
        return false;
    }

    return true;
}

void AudioData::readWAVData(std::ifstream& file) {
    if (!file) {
        std::cerr << "null input file" << std::endl;
        return;
    }

    file.seekg(sizeof(WAVHeader), std::ios::beg);
    // Resize the vector to hold the data
    data.resize(header.dataSize / sizeof(int16_t));
    // Read the file into the vector
    if (!file.read(reinterpret_cast<char*>(data.data()), header.dataSize)) {
        std::cerr << "Error reading file data" << std::endl;
    }
}


// Add encoded data to the list
void AudioData::addEncodedData(uint32_t sequenceNumber, const std::vector<uint8_t>& data) {
    EncodedData encodedData = {sequenceNumber, data};
    encodedDataList.push_back(encodedData);
}


// Add encoded data to the list
void AudioData::addEncodedData(const EncodedData& encodedData) {
    encodedDataList.push_back(encodedData);
}

// Remove encoded data from the list based on sequence number
void AudioData::removeEncodedData(uint32_t sequenceNumber) {
    encodedDataList.remove_if([sequenceNumber](const EncodedData& data) {
        return data.sequenceNumber == sequenceNumber;
    });
}

// Get the list of encoded data
std::list<EncodedData>& AudioData::getEncodedDataList() {
    return encodedDataList;
}

size_t AudioData::getEncodedDataSizeSum() {
    size_t totalSize = 0;
    for (const auto& encodedData : encodedDataList) {
        totalSize += encodedData.data.size();
    }
    return totalSize;
}