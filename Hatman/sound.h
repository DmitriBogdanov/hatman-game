#pragma once

#include <string>

#include <SFML/Audio.hpp>

#include "timer.h" // 'Milliseconds' type



class Sound {
public:
	Sound() = delete;

	Sound(const Sound&) = default;
	Sound(Sound&&) = default;

	Sound(const std::string &name, double volumeMod = 1.);

	void play();

	Milliseconds get_duration() const;
	Milliseconds get_remaining_duration() const;
	// some things need duration to deduce when the sound can be destoyed

private:
	sf::Sound sound;
};