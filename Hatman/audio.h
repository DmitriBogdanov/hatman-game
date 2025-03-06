#pragma once

#include <unordered_map>
#include <string>

#include <SFML/Audio.hpp>


class Audio {
public:
	Audio();

	~Audio() = default;

	static const Audio* READ; // used for aka 'global' access
	static Audio* ACCESS;

	const sf::SoundBuffer& getSoundBuffer(const std::string &name);

private:
	std::unordered_map<std::string, sf::SoundBuffer> loadedAudio; // all loaded sounds are saved here
	// (except music which is streamed directly from file)
};