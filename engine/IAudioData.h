#ifndef IAUDIODATA_H
#define IAUDIODATA_H
#include <vector>
#include <cstdint>
#include <list>

struct EncodedData {
    uint32_t sequenceNumber;       // Sequence number
    std::vector<uint8_t> data;     // Encoded data
};

class IAudioData {
public:
    virtual ~IAudioData() {};

    virtual int16_t* getDataPointer() = 0;
    virtual unsigned int getDataSize() const = 0;

    virtual int getSampleRate() const = 0;
    virtual int getChannels() const = 0;
    virtual int getSampleSize() const = 0;
    virtual void updateData(std::vector<int16_t> newData) = 0;
    virtual void addEncodedData(uint32_t sequenceNumber, const std::vector<uint8_t>& data) = 0;
    virtual std::list<EncodedData>& getEncodedDataList() = 0;
    virtual size_t getEncodedDataSizeSum() = 0;
};

#endif // IAUDIODATA_H