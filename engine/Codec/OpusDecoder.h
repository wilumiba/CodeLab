#ifndef OPUS_DECODER_H
#define OPUS_DECODER_H

#include <opus.h>
#include <vector>
#include <cstdint>
#include "IAudioData.h"
#include "IAudioDataHandler.h"

class OpusDecoder : public IAudioDataHandler {
public:
    OpusDecoder();
    ~OpusDecoder();

    bool initialize(int sampleRate, int numChannels);
    std::vector<int16_t> decode(const uint8_t* inputData, int inputSize);
    std::vector<int16_t> decodeAll(const std::list<EncodedData>& encodedDataList);
    std::vector<int16_t> fillGap(EncodedData& encodedData, int gap);
    bool handleAudioData(IAudioData& audioData);
    void destroy();
    void setComplexity(int complexity);

private:
    OpusDecoder(const OpusDecoder&) = delete;
    OpusDecoder& operator=(const OpusDecoder&) = delete;

    OpusDecoder* decoder;
    OpusDRED *dred;
    OpusDREDDecoder *dredDecoder;
    int mSampleRate;
    int mNumChannels;
    int maxFrameSize;

    int lastDecodeSeqNo;

    int mComplexity;
    int mPLCCount;
    int mFECCount;
    int mDREDCount;
};

#endif // OPUS_DECODER_H