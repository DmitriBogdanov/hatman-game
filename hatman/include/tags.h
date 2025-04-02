#pragma once

#include <string> // related type



// tags::
// - Contains utility function that deal with [prefixes] and {suffixes] inside a string
namespace tags {
	std::string getPrefix(const std::string &target); // [prefix]
	std::string getSuffix(const std::string &target); // {suffix}

	bool containsPrefix(const std::string &target, const std::string &prefix);
	bool containsSuffix(const std::string &target, const std::string &suffix);

	std::string makeTag(const std::string &prefix, const std::string &suffix); // makes string "[prefix]{suffix}"
}
