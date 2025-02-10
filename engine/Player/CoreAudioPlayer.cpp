
#include "CoreAudioPlayer.h"
#include <iostream>

CoreAudioPlayer::CoreAudioPlayer() : audioUnit(nullptr), audioData(nullptr), currentFrame(0) {
    AudioComponentDescription desc = {0};
    desc.componentType = kAudioUnitType_Output;
    desc.componentSubType = kAudioUnitSubType_DefaultOutput;
    desc.componentManufacturer = kAudioUnitManufacturer_Apple;
    AudioComponent outputComponent = AudioComponentFindNext(nullptr, &desc);
    if (outputComponent == nullptr) {
        std::cerr << "Error finding audio component" << std::endl;
        return;
    }
    OSStatus status = AudioComponentInstanceNew(outputComponent, &audioUnit);
    if (status != noErr) {
        std::cerr << "Error creating audio component instance: " << status << std::endl;
        return;
    }
    AURenderCallbackStruct callbackStruct;
    callbackStruct.inputProc = renderCallback;
    callbackStruct.inputProcRefCon = this;
    status = AudioUnitSetProperty(audioUnit, kAudioUnitProperty_SetRenderCallback,
                                  kAudioUnitScope_Input, 0, &callbackStruct, sizeof(callbackStruct));
    if (status != noErr) {
        std::cerr << "Error setting render callback: " << status << std::endl;
        return;
    }

}

CoreAudioPlayer::~CoreAudioPlayer() {
    if (audioUnit) {
        AudioUnitUninitialize(audioUnit);
        AudioComponentInstanceDispose(audioUnit);
        audioUnit = nullptr;
    }
}

bool CoreAudioPlayer::handleAudioData(IAudioData& audioData) {
    return synchronizedPlay(audioData);
}

bool CoreAudioPlayer::synchronizedPlay(IAudioData& audioData) {
    this->audioData = &audioData;
    this->currentFrame = 0;

    AudioStreamBasicDescription streamDesc = {0};
    streamDesc.mSampleRate = audioData.getSampleRate();
    streamDesc.mFormatID = kAudioFormatLinearPCM;
    streamDesc.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
    streamDesc.mFramesPerPacket = 1;
    streamDesc.mChannelsPerFrame = audioData.getChannels();
    streamDesc.mBitsPerChannel = audioData.getSampleSize();
    streamDesc.mBytesPerFrame = streamDesc.mChannelsPerFrame * streamDesc.mBitsPerChannel / 8;
    streamDesc.mBytesPerPacket = streamDesc.mBytesPerFrame * streamDesc.mFramesPerPacket;
    OSStatus status = AudioUnitSetProperty(audioUnit, kAudioUnitProperty_StreamFormat,
                                  kAudioUnitScope_Input, 0, &streamDesc, sizeof(streamDesc));
    if (status != noErr) {
        std::cerr << "Error setting stream format: " << status << std::endl;
        return false;
    }
    status = AudioUnitInitialize(audioUnit);
    if (status != noErr) {
        std::cerr << "Error initializing audio unit: " << status << std::endl;
        return false;
    }

    status = AudioOutputUnitStart(audioUnit);
    if (status != noErr) {
        std::cerr << "Error starting audio unit: " << status << std::endl;
        return false;
    }
    
    std::unique_lock<std::mutex> lock(mutex);
    conditionVar.wait(lock, [this] { return currentFrame >= this->audioData->getDataSize(); });

    status = AudioOutputUnitStop(audioUnit);
    if (status != noErr) {
        std::cerr << "Error stopping audio unit: " << status << std::endl;
        return false;
    }
    return true;
}

OSStatus CoreAudioPlayer::renderCallback(void* inRefCon, AudioUnitRenderActionFlags* ioActionFlags,
                                        const AudioTimeStamp* inTimeStamp, UInt32 inBusNumber,
                                        UInt32 inNumberFrames, AudioBufferList* ioData) {
    CoreAudioPlayer* player = static_cast<CoreAudioPlayer*>(inRefCon);
    const int16_t* data = player->audioData->getDataPointer();
    size_t dataSize = player->audioData->getDataSize();
    unsigned int channels = player->audioData->getChannels();

    int16_t* out = static_cast<int16_t*>(ioData->mBuffers[0].mData);
    for (UInt32 i = 0; i < inNumberFrames; ++i) {
        if (player->currentFrame < dataSize) {
            // Write left and right channels
            *out++ = data[player->currentFrame++]; // Left channel
            if (channels == 2) {
                *out++ = data[player->currentFrame++]; // Right channel
            }
        }
    }

    if (player->currentFrame >= dataSize) {
        std::lock_guard<std::mutex> lock(player->mutex);
        player->conditionVar.notify_one();
    }

    return noErr;
}
