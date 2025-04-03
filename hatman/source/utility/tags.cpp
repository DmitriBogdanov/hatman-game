// _______________________ INCLUDES _______________________

#include "utility/tags.h"

// Includes: std

// Includes: dependencies

// Includes: project

// ____________________ IMPLEMENTATION ____________________



namespace tags {

std::string get_prefix(const std::string& target) {
    const std::size_t start = target.find('[');
    const std::size_t end   = target.find(']');

    return target.substr(start + 1, end - start - 1);
}
std::string get_suffix(const std::string& target) {
    const std::size_t start = target.find('{');
    const std::size_t end   = target.find('}');

    return target.substr(start + 1, end - start - 1);
}

bool contains_prefix(const std::string& target, const std::string& prefix) {
    const std::string substr = '[' + prefix + ']';

    return target.find(substr) != std::string::npos;
}
bool contains_suffix(const std::string& target, const std::string& suffix) {
    const std::string substr = '[' + suffix + ']';

    return target.find(substr) != std::string::npos;
}

std::string make_tag(const std::string& prefix, const std::string& suffix) {
    return '[' + prefix + "]{" + suffix + '}';
}

} // namespace tags