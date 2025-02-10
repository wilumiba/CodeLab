#ifndef AUDIODATA_H
#define AUDIODATA_H

#include "IAudioData.h"
#include <fstream>
#include <string>
#include <cstdint>

struct WAVHeader {
    char riff[4];                // "RIFF"
    uint32_t chunkSize;          // Size of the entire file in bytes minus 8 bytes
    char wave[4];                // "WAVE"
    char fmt[4];                 // "fmt "
    uint32_t subchunk1Size;      // Size of the fmt chunk (16 for PCM)
    uint16_t audioFormat;        // Audio format (1 for PCM)
    uint16_t numChannels;        // Number of channels
    uint32_t sampleRate;         // Sample rate
    uint32_t byteRate;           // Byte rate (sampleRate * numChannels * bitsPerSample / 8)
    uint16_t blockAlign;         // Block align (numChannels * bitsPerSample / 8)
    uint16_t bitsPerSample;      // Bits per sample
    char data[4];                // "data"
    uint32_t dataSize;           // Size of the data section
};

class AudioData : public IAudioData {
public:
    AudioData(const std::string& filePath);
    ~AudioData();

    int16_t* getDataPointer() override;
    unsigned int getDataSize() const override;

    int getSampleRate() const override;
    int getChannels() const override;
    int getSampleSize() const override;
    void updateData(std::vector<int16_t> newData) override;

    // Methods to manage encoded data
    void addEncodedData(uint32_t sequenceNumber, const std::vector<uint8_t>& data) override;
    void addEncodedData(const EncodedData& encodedData);
    void removeEncodedData(uint32_t sequenceNumber);
    std::list<EncodedData>& getEncodedDataList() override;
    size_t getEncodedDataSizeSum() override;

private:
    void loadFromFile(const std::string& filePath);
    void readWAVData(std::ifstream& file);
    bool readWAVHeader(std::ifstream& file, WAVHeader& header);

    std::vector<int16_t> data;
    std::list<EncodedData> encodedDataList; // List to store encoded data

    WAVHeader header;
};

#endif // AUDIODATA_H