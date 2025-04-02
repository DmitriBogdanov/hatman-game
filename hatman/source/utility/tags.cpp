#include "utility/tags.h"



// tags::
std::string tags::getPrefix(const std::string& target) {
	const size_t start = target.find('[');
	const size_t end = target.find(']');
	return target.substr(start + 1, end - start - 1);
}
std::string tags::getSuffix(const std::string& target) {
	const size_t start = target.find('{');
	const size_t end = target.find('}');
	return target.substr(start + 1, end - start - 1);
}

bool tags::containsPrefix(const std::string &target, const std::string &prefix) {
	std::string tag = "";
	bool record = false;

	for (const auto &letter : target) {
		if (letter == '[') {
			record = true;
		}
		else if (letter == ']') {
			if (tag == prefix) {
				return true;
			}
			record = false;
			tag = "";
		}
		else if (record) {
			tag += letter;
		}
	}
	
	return false;
}
bool tags::containsSuffix(const std::string &target, const std::string &suffix) {
	std::string tag = "";
	bool record = false;

	for (const auto &letter : target) {
		if (letter == '{') {
			record = true;
		}
		else if (letter == '}') {
			if (tag == suffix) {
				return true;
			}
			record = false;
			tag = "";
		}
		else if (record) {
			tag += letter;
		}
	}

	return false;
}

std::string tags::makeTag(const std::string& prefix, const std::string& suffix) {
	return ("[" + prefix + "]{" + suffix + "}");
}