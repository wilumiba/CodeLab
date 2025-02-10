
#include <stdexcept>
#include <iostream>
#include <random>
#include <stdexcept>
#include "OpusEncoder.h"

OpusEncoder::OpusEncoder()
    : encoder(nullptr), maxPacketSize(4000),mFramePeriod(10) {
    
    mComplexity = 0;
    mPacketLoss = 0;
    mBitRate = OPUS_AUTO;
    mDredDuration = 0;
}

OpusEncoder::~OpusEncoder() {
    destroy();
}

bool OpusEncoder::initialize(int sampleRate, int numChannels, int application) {
    int error;

    mSampleRate = sampleRate;
    mNumChannels = numChannels;
    mApplication = application;

    std::cout << "sample rate: " << sampleRate << " channel num: " << numChannels << std::endl;
    encoder = opus_encoder_create(mSampleRate, mNumChannels, mApplication, &error);
    if (error != OPUS_OK) {
        throw std::runtime_error("Failed to create Opus encoder");
    }

    int ret = 0;
    ret = opus_encoder_ctl(encoder, OPUS_SET_BITRATE(mBitRate));   //min 2400 for speech after test, 20000 for singer
    std::cout<<"OPUS_SET_BITRATE to "<<mBitRate<<" ret: "<<ret<<std::endl;
    if ((mComplexity >= 0) && (mComplexity <= 10)) {
        ret = opus_encoder_ctl(encoder, OPUS_SET_COMPLEXITY(mComplexity));
        std::cout << "OPUS_SET_COMPLEXITY to "<<mComplexity<<" return: "<< ret<<std::endl;
    }

    ret = opus_encoder_ctl(encoder, OPUS_SET_SIGNAL(OPUS_AUTO));
    std::cout << "OPUS_SET_SIGNAL return: "<< ret<<std::endl;

    if (mPacketLoss > 0) {
        ret = opus_encoder_ctl(encoder, OPUS_SET_INBAND_FEC(1));
        std::cout << "OPUS_SET_INBAND_FEC return: "<< ret<<std::endl;
        ret = opus_encoder_ctl(encoder, OPUS_SET_PACKET_LOSS_PERC(mPacketLoss));  //min 8 to make both FEC and DRED work 
        std::cout << "OPUS_SET_PACKET_LOSS_PERC to "<<mPacketLoss<<" return: "<< ret<<std::endl;
    }
    if (mDredDuration > 0) {
        ret = opus_encoder_ctl(encoder, OPUS_SET_DRED_DURATION(mDredDuration));  //DRED_MAX_FRAMES max=104
        std::cout << "OPUS_SET_DRED_DURATION to "<<mDredDuration<<" return: "<< ret<<std::endl;
    }
    return true;
}

std::vector<uint8_t> OpusEncoder::encode(const int16_t* inputData, int frameSize) {
    // Calculate maxPacketSize based on frameSize
    maxPacketSize = frameSize * mNumChannels * sizeof(int16_t) * 2; // Conservative estimate

    std::vector<uint8_t> tempOutput(maxPacketSize);
    //std::cout<<"frame size: "<<frameSize<<" maxPacketSize: "<<maxPacketSize<<std::endl;
    int bytesEncoded = opus_encode(encoder, inputData, frameSize, tempOutput.data(), maxPacketSize);
    if (bytesEncoded < 0) {
        std::cout<< "error code: "<<bytesEncoded<<std::endl;
        throw std::runtime_error("Failed to encode audio data");
    }
    
    tempOutput.resize(bytesEncoded);
    return tempOutput;
}

bool OpusEncoder::handleAudioData(IAudioData& audioData) {
    int packetLossCnt = 0;
    // Initialize the encoder if not already initialized
    if (!encoder) {
        if (!initialize(audioData.getSampleRate(), audioData.getChannels(), OPUS_APPLICATION_AUDIO)) {
            return false;
        }
    }

    // Retrieve raw audio data from IAudioData
    const int16_t* inputData = audioData.getDataPointer();
    int inputSize = audioData.getDataSize();

    int frameNumPerSecond = 1000 / mFramePeriod;
    int sampleNumPerFrame = mSampleRate / frameNumPerSecond;
    int frameSize = sampleNumPerFrame;  //this frame size is for opus which is sample per channel of frame
    int frameShorts = sampleNumPerFrame * audioData.getChannels();

    int offset = 0;
    int seqNum = 1;

    std::cout<<"input bytes: "<<inputSize<<" frame size: "<<frameSize<<std::endl;
    // Encode the audio data in chunks
    while (offset < inputSize) {
        int chunkSize = std::min(frameShorts, inputSize - offset);
        std::vector<uint8_t> encodedChunk;
        if (chunkSize < frameShorts) {
            std::vector<int16_t> tempBuffer(frameShorts, 0);
            std::copy(inputData + offset, inputData + offset + chunkSize, tempBuffer.begin());
            encodedChunk = encode(tempBuffer.data(), frameSize);
        } else {
            encodedChunk = encode(inputData + offset, frameSize);
        }

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 99);
        
        if (dis(gen) >= mPacketLoss) {
            audioData.addEncodedData(seqNum, encodedChunk);
        } else {
            packetLossCnt++;
        }

        seqNum++;
        offset += chunkSize;
    }
    
    // Print the size of original data and encoded data
    std::cout << "Original audio data size: " << inputSize * sizeof(int16_t) << " bytes" << std::endl;
    std::cout << "Encoded audio data size: " << audioData.getEncodedDataSizeSum() << " bytes. Packet count(after loss): "
              << audioData.getEncodedDataList().size()<<" loss count: "<<packetLossCnt<< std::endl;

    return true;
}

void OpusEncoder::destroy() {
    if (encoder) {
        opus_encoder_destroy(encoder);
        encoder = nullptr;
    }
}

void OpusEncoder::setComplexity(int complexity) {
    mComplexity = complexity;
}

void OpusEncoder::setPacketLoss(int packetLoss) {
    mPacketLoss = packetLoss;
}

void OpusEncoder::setBitRate(int bitRate) {
    mBitRate = bitRate;
}

void OpusEncoder::setDredDuration(int dredDuration) {
    mDredDuration = dredDuration;
}