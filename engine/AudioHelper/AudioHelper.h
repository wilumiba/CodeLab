#ifndef AUDIOHELPER_H
#define AUDIOHELPER_H

#include <string>
#include "IAudioData.h"

class AudioHelper {
public:
    static void SaveAudioDataToWavFile(const std::string& filename, IAudioData& audioData);
};

#endif // AUDIOHELPER_H