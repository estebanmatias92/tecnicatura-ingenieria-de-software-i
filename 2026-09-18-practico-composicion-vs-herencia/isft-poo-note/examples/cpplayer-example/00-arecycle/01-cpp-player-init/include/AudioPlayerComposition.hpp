#ifndef AUDIOPLAYERCOMPOSITION_H
#define AUDIOPLAYERCOMPOSITION_H
#include "AudioSource.hpp"

class AudioPlayerComposition {
public:
    AudioPlayerComposition();
    bool open(std::string filePath);
    bool play();
    bool stop();
    void setVolume(float value);
private:
    AudioSource source; // composición
};
#endif
