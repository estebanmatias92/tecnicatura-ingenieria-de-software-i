#include "AudioPlayerComposition.hpp"
AudioPlayerComposition::AudioPlayerComposition() {}
bool AudioPlayerComposition::open(std::string fp) { return source.open(fp); }
bool AudioPlayerComposition::play() { return source.play(); }
bool AudioPlayerComposition::stop() { return source.stop(); }
void AudioPlayerComposition::setVolume(float v) { source.setVolume(v); }
