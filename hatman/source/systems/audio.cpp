#include "systems/audio.h"

#include <iostream>



// # Audio #

const Audio* Audio::READ;
Audio* Audio::ACCESS;

Audio::Audio() {
	std::cout << "Creating audio system...\n";

	Audio::READ = this; // init global access
	Audio::ACCESS = this;
}

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