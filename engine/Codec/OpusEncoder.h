#ifndef OPUS_ENCODER_H
#define OPUS_ENCODER_H

#include <opus.h>
#include <vector>
#include <cstdint>
#include <set>
#include "IAudioData.h"
#include "IAudioDataHandler.h"

class OpusEncoder : public IAudioDataHandler {
public:
    OpusEncoder();
    ~OpusEncoder();

    bool initialize(int sampleRate, int numChannels, int application);
    std::vector<uint8_t> encode(const int16_t* inputData, int frameSize);
    void destroy();
    bool handleAudioData(IAudioData& audioData);

    void setComplexity(int complexity);
    void setPacketLoss(int packetLoss);
    void setBitRate(int bitRate);
    void setDredDuration(int dredDuration);

private:
    OpusEncoder(const OpusEncoder&) = delete;
    OpusEncoder& operator=(const OpusEncoder&) = delete;

    OpusEncoder* encoder;
    int mSampleRate;
    int mNumChannels;
    int mApplication;
    int maxPacketSize;
    int mFramePeriod;    //in unit ms
    
    int mComplexity;
    int mPacketLoss;
    int mBitRate;
    int mDredDuration;
};

#endif // OPUS_ENCODER_H