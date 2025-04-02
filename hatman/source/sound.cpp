#include "sound.h"

#include "game.h" // holds volume settings
#include "globalconsts.hpp" // holds base volume
#include "audio.h" // sound loading

Sound::Sound(const std::string &name, double volumeMod) {
	this->sound.setBuffer(Audio::ACCESS->getSoundBuffer(name));

	constexpr double SFML_MAX_VOLUME = 100; // SFML uses volume range [0, 100]
	const double total_volume = SFML_MAX_VOLUME * audio::FX_BASE_VOLUME * Game::READ->sound_volume_mod * volumeMod;
	const float clamped_volume = std::clamp(static_cast<float>(total_volume), 0.f, 100.f);
	this->sound.setVolume(clamped_volume);
}

void Sound::play() {
	this->sound.play();
}

constexpr Milliseconds _epsilon = 1e-3;
// we slightly overestimate duration just in case,
// don't want to cut the last tiny portion of the sound

Milliseconds Sound::get_duration() const {
	sf::Time sfml_duration = this->sound.getBuffer()->getDuration();
	Milliseconds duration = sfml_duration.asMicroseconds() / 1e3;
	return duration + _epsilon;
}

Milliseconds Sound::get_remaining_duration() const {
	sf::Time sfml_offset = this->sound.getPlayingOffset();
	sf::Time sfml_duration = this->sound.getBuffer()->getDuration();
	sf::Time sfml_remaining_duration = sfml_duration - sfml_offset;
	Milliseconds remaining_duration = sfml_remaining_duration.asMicroseconds() / 1e3;
	return remaining_duration + _epsilon;
}