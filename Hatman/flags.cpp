#include "flags.h"



// Helper functions
bool is_negative(const Flag &flag) {
	return flag.front() == '!';
}

std::string remove_negation(const Flag &flag) {
	return flag.substr(1);
}


// # Flags #
const Flags* Flags::READ;
Flags* Flags::ACCESS;

Flags::Flags() {
	this->READ = this;
	this->ACCESS = this;
}

void Flags::add(const Flag &flag) {
	this->flags.insert(flag);
}

void Flags::remove(const Flag &flag) {
	this->flags.erase(flag);
}

void Flags::remove_containing_substring(const std::string &substring) {
	// Standard 'remove_if' loop for std::set
	for (auto it = this->flags.begin(); it != this->flags.end(); )
		if ((*it).find(substring) != std::string::npos)
			this->flags.erase(it++);
		else
			++it;
}

bool Flags::check(const Flag &flag) const {
	if (is_negative(flag))
		return !this->flags.count(remove_negation(flag));
	else
		return this->flags.count(flag);
}
