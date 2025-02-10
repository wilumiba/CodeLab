#include <iostream>
#include <unistd.h> // for getopt
#include <getopt.h>
#include "AudioData.h"
#include "CoreAudioPlayer.h"
#include "IAudioDataHandler.h"
#include "AudioHandlerChain.h"
#include "Accelerator.h"
#include "SoundToucher.h"
#include "OpusEncoder.h"
#include "OpusDecoder.h"
#include "AudioHelper.h"

int main(int argc, char* argv[]) {
    int opt;
    std::string file;
    std::string accelerate;
    std::string speed;
    std::string codec;
    std::string encoder_complexity;
    std::string decoder_complexity;
    std::string packet_loss;
    std::string bit_rate;
    std::string dred_duration;

    const struct option long_options[] = {
        {"file", required_argument, nullptr, 'f'}, 
        {"accelerate", required_argument, nullptr, 'a'}, 
        {"speed", required_argument, nullptr, 1}, 
        {"codec", required_argument, nullptr, 'c'},  
        {"encoder_complexity", required_argument, nullptr, 2},
        {"decoder_complexity", required_argument, nullptr, 3}, 
        {"packet_loss", required_argument, nullptr, 4}, 
        {"bit_rate", required_argument, nullptr, 5}, 
        {"dred_duration", required_argument, nullptr, 6}, 
        {nullptr, 0, nullptr, 0}
    };

    const char* short_options = "f:a:c:";

    while ((opt = getopt_long(argc, argv, short_options, long_options, nullptr)) != -1) {
        switch (opt) {
            case 'f':
                file = optarg;
                std::cout << "File: " << file << std::endl;
                break;
            case 'a':
                accelerate = optarg;
                break;
            case 'c':
                codec = optarg;
                break;
            case 1:
                speed = optarg;
                break;
            case 2:
                encoder_complexity = optarg;
                break;
            case 3:
                decoder_complexity = optarg;
                break;
            case 4:
                packet_loss = optarg;
                break;
            case 5:
                bit_rate = optarg;
                break;
            case 6:
                dred_duration = optarg;
                break;
            case '?':
                std::cerr << "Unknown option: " << optopt << std::endl;
                return 1;
            default:
                std::cerr << "Unexpected option" << std::endl;
                return 1;
        }
    }

    
    if (file.empty()) {
        std::cerr << "Usage: " << argv[0] << " --file <path_to_pcm_file.pcm> \
        [-a <sonic/soundtouch> --speed [0.5~2.0]]] \
        [-c <opus> --encoder_complexity <1~10> --decoder_complexity <1~10> --packet_loss <0~100> --bit_rate <500~512000> --dred_duration <1~100>]"
        << std::endl;
        return 1;
    }

    std::shared_ptr<IAudioDataHandler> accHandler = nullptr;
    std::shared_ptr<OpusEncoder> opusEncoder = nullptr;
    std::shared_ptr<OpusDecoder> opusDecoder = nullptr;

    if (!accelerate.empty()) {
        float fSpeed = 1.0;
        if (!speed.empty()) {
            fSpeed = std::stof(speed);
        }

        if (accelerate == "sonic") {
            accHandler = std::make_shared<AudioAccelerator>(fSpeed);
        } else if (accelerate == "soundtouch") {
            accHandler = std::make_shared<AudioSoundToucher>(fSpeed);
        } else {
            std::cerr << "Invalid audio accelerator engine: " << accelerate << std::endl;
            return 1;
        }
    }

    if (!codec.empty()) {
        if (codec != "opus") {
            std::cerr << "Invalid codec: " << codec << std::endl;
            return 1;
        }
        
        opusEncoder = std::make_shared<OpusEncoder>();
        opusDecoder = std::make_shared<OpusDecoder>();

        if (!encoder_complexity.empty()) {
            opusEncoder->setComplexity(std::stoi(encoder_complexity));
        }
        if (!packet_loss.empty()) {
            opusEncoder->setPacketLoss(std::stoi(packet_loss));
        }
        if (!bit_rate.empty()) {
            opusEncoder->setBitRate(std::stoi(bit_rate));
        }
        if (!dred_duration.empty()) {
            opusEncoder->setDredDuration(std::stoi(dred_duration));
        }
        if (!decoder_complexity.empty()) {
            opusDecoder->setComplexity(std::stoi(decoder_complexity));
        }

    }

    AudioData audioData(file);
    int16_t* dataPointer = audioData.getDataPointer();
    if (dataPointer) {
        std::shared_ptr<IAudioDataHandler> player = std::make_shared<CoreAudioPlayer>();

        AudioHandlerChain processor;
        //processor.addHandler(player);
        if (opusEncoder) {
            processor.addHandler(opusEncoder);
        }
        if (opusDecoder) {
            processor.addHandler(opusDecoder);
        }
        if (accHandler) {
            processor.addHandler(accHandler);
        }
        processor.addHandler(player);

        if (processor.process(audioData)) {
            std::cout << "Processing audio finished..." << std::endl;
            AudioHelper::SaveAudioDataToWavFile("output.wav", audioData);
        } else {
            std::cout << "Failed to process audio." << std::endl;
        }
    } else {
        std::cout << "Failed to read audio data." << std::endl;
    }

    return 0;
}