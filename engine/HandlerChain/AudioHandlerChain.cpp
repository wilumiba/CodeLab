#include "AudioHandlerChain.h"
#include <iostream>

void AudioHandlerChain::addHandler(std::shared_ptr<IAudioDataHandler> handler) {
    handlers.push_back(handler);
}

bool AudioHandlerChain::process(IAudioData& audioData) {
    for (auto& handler : handlers) {
        if (!handler->handleAudioData(audioData)) {
            return false;
        }
    }
    return true;
}