#ifndef COREAUDIOPLAYER_H
#define COREAUDIOPLAYER_H

#include <condition_variable>
#include <mutex>

#include "IAudioData.h"
#include <AudioToolbox/AudioToolbox.h>
#include "IAudioDataHandler.h"

class CoreAudioPlayer : public IAudioDataHandler{
public:
    CoreAudioPlayer();
    ~CoreAudioPlayer();

    bool synchronizedPlay(IAudioData& audioData);
    bool handleAudioData(IAudioData& audioData) override;

private:
    static OSStatus renderCallback(void* inRefCon, AudioUnitRenderActionFlags* ioActionFlags,
                                  const AudioTimeStamp* inTimeStamp, UInt32 inBusNumber,
                                  UInt32 inNumberFrames, AudioBufferList* ioData);

    void initialize(IAudioData& audioData){};

    AudioComponentInstance audioUnit;
    IAudioData* audioData;
    size_t currentFrame;

    std::condition_variable conditionVar;
    std::mutex mutex;
    
};

#endif // COREAUDIOPLAYER_H