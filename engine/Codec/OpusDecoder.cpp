
#include <stdexcept>
#include <iostream>
#include <algorithm>

#include "OpusDecoder.h"
#include "IAudioData.h"

OpusDecoder::OpusDecoder()
    : decoder(nullptr), maxFrameSize(960 * 6), dred(nullptr), dredDecoder(nullptr), lastDecodeSeqNo(0) {
    mComplexity = 0;
    mSampleRate = 0;
    mNumChannels = 0;
    mPLCCount = 0;
    mFECCount = 0;
    mDREDCount = 0;
} // 120ms at 48kHz

OpusDecoder::~OpusDecoder() {
    destroy();
}

bool OpusDecoder::initialize(int sampleRate, int numChannels) {
    int error;

    mSampleRate = sampleRate;
    mNumChannels = numChannels;

    decoder = opus_decoder_create(mSampleRate, mNumChannels, &error);
    if (error != OPUS_OK) {
        throw std::runtime_error("Failed to create Opus decoder");
    }

    opus_decoder_ctl(decoder, OPUS_SET_COMPLEXITY(mComplexity));

    dredDecoder = opus_dred_decoder_create(&error);
    if (error != OPUS_OK) {
        throw std::runtime_error("Failed to create dred decoder");
    }

    dred = opus_dred_alloc(&error);
    if (error != OPUS_OK) {
        throw std::runtime_error("Failed to create dred");
    }

    return true;
}

std::vector<int16_t> OpusDecoder::decode(const uint8_t* inputData, int inputSize) {
    // Estimate the maximum number of samples that can be decoded
    int maxOutputSamples = maxFrameSize * mNumChannels; // Conservative estimate

    std::vector<int16_t> output(maxOutputSamples);
    int frameSize = opus_decode(decoder, inputData, inputSize, output.data(), maxOutputSamples, 0);
    if (frameSize < 0) {
        throw std::runtime_error("Failed to decode audio data");
    }

    output.resize(frameSize * mNumChannels);
    return output;
}

std::vector<int16_t> OpusDecoder::fillGap(EncodedData& encodedData, int gap) {
    // Estimate the maximum number of samples that can be decoded
    int maxOutputSamples = maxFrameSize * mNumChannels; // Conservative estimate

    std::vector<int16_t> filledData(maxOutputSamples);
    std::vector<int16_t> combinedOutput;
    int lostCnt = gap-1;

    int outputSamples = 0;
    int dred_end = 0;
    int dred_input = 0;
    opus_decoder_ctl(decoder, OPUS_GET_LAST_PACKET_DURATION(&outputSamples));
    dred_input = lostCnt * outputSamples;
    int ret = opus_dred_parse(dredDecoder, dred, encodedData.data.data(), encodedData.data.size(), std::min(48000, std::max(0, dred_input)), mSampleRate, &dred_end, 0);
    if (ret < 0) {
        throw std::runtime_error("Failed to parse DRED data");
    }
    dred_input = ret > 0 ? ret : 0;

    for (int recoveredCnt = 0; recoveredCnt < lostCnt; recoveredCnt++) {
        if (recoveredCnt == lostCnt - 1 && opus_packet_has_lbrr(encodedData.data.data(), encodedData.data.size())) {
            //std::cout<<"opus_packet_has_lbrr, use FEC for recover count: "<<recoveredCnt<<std::endl;
            mFECCount++;
            opus_decoder_ctl(decoder, OPUS_GET_LAST_PACKET_DURATION(&outputSamples));
            outputSamples = opus_decode(decoder, encodedData.data.data(), encodedData.data.size(), filledData.data(), outputSamples, 1);
        } else {
            opus_decoder_ctl(decoder, OPUS_GET_LAST_PACKET_DURATION(&outputSamples));
            if (dred_input > 0) {
                mDREDCount++;
                //std::cout<<"DRED contains samples: "<<dred_input<<", use DRED for recover count: "<<recoveredCnt<<std::endl;
                outputSamples = opus_decoder_dred_decode(decoder, dred, (lostCnt - recoveredCnt) * outputSamples, filledData.data(), outputSamples);
            } else {
                mPLCCount++;
                //std::cout<<"No DRED data, use PLC for recover count: "<<recoveredCnt<<std::endl;
                outputSamples = opus_decode(decoder, nullptr, 0, filledData.data(), outputSamples, 0);
            }
        }

        if (outputSamples > 0) {
            combinedOutput.insert(combinedOutput.end(), filledData.begin(), filledData.begin() + outputSamples * mNumChannels);
        }
    }

    return combinedOutput;
}

std::vector<int16_t> OpusDecoder::decodeAll(const std::list<EncodedData>& encodedDataList) {
    std::vector<int16_t> combinedOutput;
    int gap = 0;
    std::vector<int16_t> decodedData;

    // Check only the tail of the encoded data list to see if opus_packet_has_lbrr and opus_dred_parse can get dred data
    /*auto tail = encodedDataList.rbegin();
    if (tail != encodedDataList.rend()) {
        const auto& encodedData = *tail;
        int dred_end = 0;
        int ret = opus_dred_parse(dredDecoder, dred, encodedData.data.data(), encodedData.data.size(), 48000, mSampleRate, &dred_end, 0);
        if (ret > 0) {
            std::cout<<"Tail packet has DRED data. Detected samples: "<<ret<<std::endl;
        } else {
            std::cout<<"Tail packet has no DRED data"<<std::endl;
        }
        if (opus_packet_has_lbrr(encodedData.data.data(), encodedData.data.size())) {
            std::cout << "Tail packet has LBRR data" << std::endl;
        } else {
            std::cout << "Tail packet has no LBRR data" << std::endl;
        }
    }*/

    for (const auto& encodedData : encodedDataList) {
        gap = encodedData.sequenceNumber - lastDecodeSeqNo;
        if (gap == 1) {
            decodedData = decode(encodedData.data.data(), encodedData.data.size());
            combinedOutput.insert(combinedOutput.end(), decodedData.begin(), decodedData.end());
        } else if (gap > 1) {
            //std::cout<<"Gap is: "<<gap<<" from "<<lastDecodeSeqNo<<" to "<<encodedData.sequenceNumber<<", fill the gap"<<std::endl;
            decodedData = fillGap(const_cast<EncodedData&>(encodedData), gap);
            combinedOutput.insert(combinedOutput.end(), decodedData.begin(), decodedData.end());

            decodedData = decode(encodedData.data.data(), encodedData.data.size());
            combinedOutput.insert(combinedOutput.end(), decodedData.begin(), decodedData.end());
        }
        else {
            std::cout << "Gap is less than 1, what happened??" << std::endl;
            exit(1);
        }
        lastDecodeSeqNo = encodedData.sequenceNumber;
    }

    return combinedOutput;
}

bool OpusDecoder::handleAudioData(IAudioData& audioData) {
    // Initialize the decoder if not already initialized
    if (!decoder) {
        if (!initialize(audioData.getSampleRate(), audioData.getChannels())) {
            return false;
        }
    }

    // Retrieve encoded audio data from IAudioData
    const std::list<EncodedData>& encodedDataList = audioData.getEncodedDataList();

    // Decode all encoded data blocks and combine them
    std::vector<int16_t> combinedDecodedData = decodeAll(encodedDataList);

    // Update the IAudioData object with the combined decoded data
    audioData.updateData(combinedDecodedData);
    // Print the size of audioData after update
    std::cout << "Decoded audio data size: " << audioData.getDataSize() * sizeof(int16_t) <<" bytes."<<" PLC count: "<<mPLCCount
             <<" FEC count: "<<mFECCount<<" DRED count: "<<mDREDCount<< std::endl;
    return true;
}

void OpusDecoder::destroy() {
    if (decoder) {
        opus_decoder_destroy(decoder);
        decoder = nullptr;
    }

    if (dred) {
        opus_dred_free(dred);
        dred = nullptr;
    }
    
    if (dredDecoder) {
        opus_dred_decoder_destroy(dredDecoder);
        dredDecoder = nullptr;
    }
}

void OpusDecoder::setComplexity(int complexity) {
    mComplexity = complexity;
}