#pragma once

#include <unordered_set>
#include <string>



using Flag = std::string;

// # FlagStorage #
// - Can be accessed wherever #include'ed through static 'READ' and 'ACCESS' fields
class Flags {
public:
	Flags();

	static const Flags* READ; // used for aka 'global' access
	static Flags* ACCESS;

	void add(const Flag &flag);
	void remove(const Flag &flag);

	void remove_containing_substring(const std::string &substring);

	bool check(const Flag &flag) const;
		//  <flag_name> => returns true if flag is present
		// Also accepts negative form
		// !<flag_name> => returns true if flag is NOT present

	std::unordered_set<Flag> flags; // std::unordered_set can be faster if number of flag is large enough
		// not protected from direct editing but setter/getter use is prefered
};