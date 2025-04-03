#include "systems/audio.h"

#include <iostream>

#include "utility/globalconsts.hpp"


// # Audio #

const Audio* Audio::READ;
Audio* Audio::ACCESS;

Audio::Audio(int music_volume_setting, int sound_volume_setting) :
    music_volume_mod(music_volume_setting / 10.),
    sound_volume_mod(sound_volume_setting / 10.)
{
	std::cout << "Creating audio system...\n";

	Audio::READ = this; // init global access
	Audio::ACCESS = this;
}

// - Sounds -

const sf::SoundBuffer& Audio::getSoundBuffer(const std::string &name) {
	auto it = this->loadedAudio.find(name);

	// Sound not loaded => load and return
	if (it == this->loadedAudio.end()) {
		std::string filepath = "content/audio/fx/" + name;
		sf::SoundBuffer buffer;
		buffer.loadFromFile(filepath);
		const auto emplaced_it = this->loadedAudio.try_emplace(name, std::move(buffer)).first;
		// sf::SoundBuffer accepts filename to load as a constructor arg
		return emplaced_it->second;
	}

	// Sound loaded => return
	return it->second;
}

// - Music -

void Audio::queue_music(const std::string &name) {
    if (this->music_current == name) return; // don't fade-out & fade-in the same track
    
    this->music_queued = name;
    
    // Fade-out previous music, if there was none start straight from the fade-in
    if (this->music_current.empty()) {
        this->set_music(name);
        this->set_music_volume(0.);
        this->music_do_fade_in = true;
        this->music_fade_in_timer.start(defaults::MUSIC_FADE_DURATION);
    }
    else {
        this->set_music_volume(1.);
        this->music_do_fade_out = true;
        this->music_fade_out_timer.start(defaults::MUSIC_FADE_DURATION);
    }
}

void Audio::update([[maybe_unused]] Milliseconds elapsedTime) {
    // Turbo-inefficient, but who cares
    
    if (this->music_do_fade_out) {       
        if (this->music_fade_out_timer.finished()) {
            
            this->set_music(this->music_queued);
            this->music_current = std::move(this->music_queued);
            this->music_queued.clear();
            
            this->music_do_fade_out = false;
            this->music_do_fade_in = true;
            this->music_fade_in_timer.start(defaults::MUSIC_FADE_DURATION);
        }
        
        this->set_music_volume(1. - this->music_fade_out_timer.elapsedPercentage());
    }
    
    if (this->music_do_fade_in) {
        if (this->music_fade_in_timer.finished()) {
            this->music_do_fade_in = false;
        }
        
        this->set_music_volume(this->music_fade_in_timer.elapsedPercentage());
    }
}

void Audio::set_music(const std::string &name) {
    if (this->music_current == name) return; // don't repeat music if it's already playing
    
    this->music_current = name;
    
    // Load SFML music
    if (!music.openFromFile("content/audio/mx/" + name))
    	std::cout << "Error: Could not open music file...\n";
    
    music.setLoop(true); // for some reason music doesn't loop by default
    
    music.play();
}

void Audio::set_music_volume(double volumeMod) {
    constexpr double SFML_MAX_VOLUME = 100; // SFML uses volume range [0, 100]
    const double total_volume = SFML_MAX_VOLUME * audio::MUSIC_BASE_VOLUME * Audio::READ->music_volume_mod * volumeMod;
    const float clamped_volume = std::clamp(static_cast<float>(total_volume), 0.f, 100.f);
    this->music.setVolume(clamped_volume);
}