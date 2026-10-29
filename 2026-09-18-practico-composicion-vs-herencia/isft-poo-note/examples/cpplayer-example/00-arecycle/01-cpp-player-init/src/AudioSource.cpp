#include "AudioSource.hpp"
#include <fstream>

AudioSource::AudioSource() : volume(10), isOpen(false), isPlaying(false) {}
bool AudioSource::open(std::string filePath) {
    std::ifstream f(filePath.c_str());
    isOpen = f.good();
    return isOpen;
}
bool AudioSource::play() { if(isOpen) isPlaying = true; return isPlaying; }
bool AudioSource::stop() { if(isPlaying) isPlaying = false; return isPlaying; }
void AudioSource::setVolume(float v) { volume = v; }
