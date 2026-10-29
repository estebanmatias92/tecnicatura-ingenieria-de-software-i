#ifndef AUDIOSOURCE_H
#define AUDIOSOURCE_H
#include <string>

class AudioSource {
public:
    AudioSource();
    bool open(std::string filePath);
    bool play();
    bool stop();
    void setVolume(float value);
private:
    float volume;
    bool isOpen;
    bool isPlaying;
};
#endif
