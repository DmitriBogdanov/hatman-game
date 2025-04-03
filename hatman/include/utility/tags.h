#pragma once

// _______________________ INCLUDES _______________________

// Includes: std
#include <string> // string

// Includes: dependencies

// Includes: project

// ____________________ DEVELOPER DOCS ____________________

// The game often uses strings of following format: [prefix]{suffix},
// this header contains functions for dealing with such strings

// ____________________ IMPLEMENTATION ____________________



namespace tags {

std::string get_prefix(const std::string& target); // [prefix]
std::string get_suffix(const std::string& target); // {suffix}

bool contains_prefix(const std::string& target, const std::string& prefix);
bool contains_suffix(const std::string& target, const std::string& suffix);

std::string make_tag(const std::string& prefix, const std::string& suffix); // returns string "[prefix]{suffix}"

} // namespace tags
