#ifndef AUDIOHANDLERCHAIN_H
#define AUDIOHANDLERCHAIN_H

#include "IAudioDataHandler.h"
#include <vector>
#include <memory>

class AudioHandlerChain {
public:
    void addHandler(std::shared_ptr<IAudioDataHandler> handler);
    bool process(IAudioData& audioData);

private:
    std::vector<std::shared_ptr<IAudioDataHandler>> handlers;
};

#endif // AUDIOHANDLERCHAIN_H