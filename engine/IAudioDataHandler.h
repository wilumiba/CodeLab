#ifndef IAUDIODATAHANDLER_H
#define IAUDIODATAHANDLER_H

#include "IAudioData.h"

class IAudioDataHandler {
public:
    virtual ~IAudioDataHandler() {};
    virtual bool handleAudioData(IAudioData& audioData) = 0;
};

#endif // IAUDIODATAHANDLER_H