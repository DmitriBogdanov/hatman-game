#pragma once

#include <string>
#include <unordered_map>

#include <SFML/Audio.hpp>

#include "systems/timer.h" // Timers



class Audio {
public:
    Audio(int music_volume_setting, int sound_volume_setting);

    ~Audio() = default;

    static const Audio* READ; // used for aka 'global' access
    static Audio*       ACCESS;
    
    // - Sounds -
    const sf::SoundBuffer& getSoundBuffer(const std::string& name);

    // - Music -
    void queue_music(const std::string& name);

    void update(Milliseconds elapsedTime); // updates music queue & volume
    
    double music_volume_mod;
    double sound_volume_mod; // may be accessed from outside
    
private:
    
    void set_music(const std::string& name);
    void set_music_volume(double volumeMod = 1.);

    std::string music_current;
    std::string music_queued;

    bool music_do_fade_out = false;
    bool music_do_fade_in  = false;

    Timer music_fade_out_timer;
    Timer music_fade_in_timer;

    sf::Music music;

    std::unordered_map<std::string, sf::SoundBuffer> loadedAudio; // all loaded sounds are saved here
                                                                  // (except music which is streamed directly from file)
};