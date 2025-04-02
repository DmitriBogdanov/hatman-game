// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/UTL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::stre
// Documentation: https://github.com/DmitriBogdanov/UTL/blob/master/docs/module_stre.md
// Source repo:   https://github.com/DmitriBogdanov/UTL
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTLMODULE_STRE)
#ifndef UTLHEADERGUARD_STRE
#define UTLHEADERGUARD_STRE

// _______________________ INCLUDES _______________________

#include <algorithm>   // transform()
#include <cctype>      // tolower(), toupper()
#include <cstddef>     // size_t
#include <stdexcept>   // invalid_argument
#include <string>      // string
#include <string_view> // string_view
#include <vector>      // vector<>

// ____________________ DEVELOPER DOCS ____________________

// String utils. Nothing fancy, basic stuff, however there is a lot of really bad implementations
// found online, which is why I'd rather put an effort get them right once and be done with it.

// ____________________ IMPLEMENTATION ____________________

namespace utl::stre {

// ================
// --- Trimming ---
// ================

template <class T>
[[nodiscard]] std::string trim_left(T&& str, char trimmed_char = ' ') {
    std::string res = std::forward<T>(str);            // when 'str' is an r-value, we can avoid the copy
    res.erase(0, res.find_first_not_of(trimmed_char)); // seems to be the fastest way of doing it
    return res;
}

template <class T>
[[nodiscard]] std::string trim_right(T&& str, char trimmed_char = ' ') {
    std::string res = std::forward<T>(str);
    res.erase(res.find_last_not_of(trimmed_char) + 1);
    return res;
}

template <class T>
[[nodiscard]] std::string trim(T&& str, char trimmed_char = ' ') {
    return trim_right(trim_left(std::forward<T>(str), trimmed_char), trimmed_char);
}

// ===============
// --- Padding ---
// ===============

[[nodiscard]] inline std::string pad_left(std::string_view str, std::size_t length, char padding_char = ' ') {
    if (length > str.size()) {
        std::string res;
        res.reserve(length);
        res.append(length - str.size(), padding_char);
        res += str;
        return res;
    } else return std::string(str);
}

[[nodiscard]] inline std::string pad_right(std::string_view str, std::size_t length, char padding_char = ' ') {
    if (length > str.size()) {
        std::string res;
        res.reserve(length);
        res += str;
        res.append(length - str.size(), padding_char);
        return res;
    } else return std::string(str);
}

[[nodiscard]] inline std::string pad(std::string_view str, std::size_t length, char padding_char = ' ') {
    if (length > str.size()) {
        std::string res;
        res.reserve(length);
        const std::size_t left_pad_size = (length - str.size()) / 2;
        res.append(left_pad_size, padding_char);
        res += str;
        const std::size_t right_pad_size = length - str.size() - left_pad_size;
        res.append(right_pad_size, padding_char);
        return res;
        // we try to pad evenly on both sides, but one of the pads (the right one to be exact)
        // may be a character longer than the other if the length difference is odd
    } else return std::string(str);
}

[[nodiscard]] inline std::string pad_with_leading_zeroes(unsigned int number, std::size_t length = 12) {
    const std::string number_str = std::to_string(number);

    if (length > number_str.size()) {
        std::string res;
        res.reserve(length);
        res.append(length - number_str.size(), '0');
        res += number_str;
        return res;
    } else return number_str;
    // we do this instead of using 'std::ostringstream' with 'std::setfill('0')' + 'std::setw()'
    // so we don't need streams as a dependency. Plus it is faster that way.
}

// ========================
// --- Case conversions ---
// ========================

template <class T>
[[nodiscard]] std::string to_lower(T&& str) {
    std::string res = std::forward<T>(str); // when 'str' is an r-value, we can avoid the copy
    std::transform(res.begin(), res.end(), res.begin(), [](unsigned char c) { return std::tolower(c); });
    return res;
    // note that 'std::tolower()', 'std::toupper()' can only apply to unsigned chars, calling it on signed char
    // is UB. Implementation above was directly taken from https://en.cppreference.com/w/cpp/string/byte/tolower
}

template <class T>
[[nodiscard]] std::string to_upper(T&& str) {
    std::string res = std::forward<T>(str);
    std::transform(res.begin(), res.end(), res.begin(), [](unsigned char c) { return std::toupper(c); });
    return res;
}

// ========================
// --- Substring checks ---
// ========================

// Note:
// C++20 adds 'std::basic_string<T>::starts_with()', 'std::basic_string<T>::ends_with()',
// 'std::basic_string<T>::contains()', making these functions pointless in a new standard.

[[nodiscard]] inline bool starts_with(std::string_view str, std::string_view substr) {
    return str.size() >= substr.size() && str.compare(0, substr.size(), substr) == 0;
}

[[nodiscard]] inline bool ends_with(std::string_view str, std::string_view substr) {
    return str.size() >= substr.size() && str.compare(str.size() - substr.size(), substr.size(), substr) == 0;
}

[[nodiscard]] inline bool contains(std::string_view str, std::string_view substr) {
    return str.find(substr) != std::string_view::npos;
}

// ==========================
// --- Token manipulation ---
// ==========================

template <class T>
[[nodiscard]] std::string replace_all_occurences(T&& str, std::string_view from, std::string_view to) {
    std::string res = std::forward<T>(str);

    std::size_t i = 0;
    while ((i = res.find(from, i)) != std::string::npos) { // locate substring to replace
        res.replace(i, from.size(), to);                   // replace
        i += to.size();                                    // step over the replaced region
    }
    // Note: Not stepping over the replaced regions causes self-similar replacements
    // like "abc" -> "abcabc" to fall into an infinite loop, we don't want that.

    return res;
}

// Note:
// Most "split by delimer" implementations found online seem to be horrifically inefficient
// with unnecessary copying/erasure/intermediate tokens, stringstreams and etc.
//
// We can just scan through the string view once, while keeping track of the last segment between
// two delimiters, no unnecessary work, the only place where we do a copy is during emplacement into
// the vector where it's unavoidable
[[nodiscard]] inline std::vector<std::string> split_by_delimiter(std::string_view str, std::string_view delimiter,
                                                                 bool keep_empty_tokens = false) {
    if (delimiter.empty()) return {std::string(str)};
    // handle empty delimiter explicitly so we can't fall into an infinite loop

    std::vector<std::string> tokens;
    std::size_t              cursor        = 0;
    std::size_t              segment_start = cursor;

    while ((cursor = str.find(delimiter, cursor)) != std::string_view::npos) {
        if (keep_empty_tokens || segment_start != cursor)
            tokens.emplace_back(str.substr(segment_start, cursor - segment_start));
        // don't emplace empty tokens in case of leading/trailing/repeated delimiter
        cursor += delimiter.size();
        segment_start = cursor;
    }

    if (keep_empty_tokens || segment_start != str.size()) tokens.emplace_back(str.substr(segment_start));
    // 'cursor' is now at 'npos', so we compare to the size instead

    return tokens;
}

// ===================
// --- Other utils ---
// ===================

[[nodiscard]] inline std::string repeat_char(char ch, std::size_t repeats) { return std::string(repeats, ch); }

[[nodiscard]] inline std::string repeat_string(std::string_view str, std::size_t repeats) {
    std::string res;
    res.reserve(str.size() * repeats);
    while (repeats--) res += str;
    return res;
}

// Mostly useful to print strings with special chars in console and look at their contents.
[[nodiscard]] inline std::string escape_control_chars(std::string_view str) {
    std::string res;
    res.reserve(str.size()); // no necesseraly correct, but it's a godd first guess

    for (const char c : str) {
        // Control characters with dedicated escape sequences get escaped with those sequences
        if (c == '\a') res += "\\a";
        else if (c == '\b') res += "\\b";
        else if (c == '\f') res += "\\f";
        else if (c == '\n') res += "\\n";
        else if (c == '\r') res += "\\r";
        else if (c == '\t') res += "\\t";
        else if (c == '\v') res += "\\v";
        // Other non-printable chars get replaced with their codes
        else if (!std::isprint(static_cast<unsigned char>(c))) {
            res += '\\';
            res += std::to_string(static_cast<int>(c));
        }
        // Printable chars are appended as is.
        else
            res += c;
    }
    // Note: This could be implemented faster using the 'utl::json' method of handling escapes with buffering and
    // a lookup table, however I don't see much practical reason to complicate this implementation like that.

    return res;
}

[[nodiscard]] inline std::size_t index_of_difference(std::string_view str_1, std::string_view str_2) {
    using namespace std::string_literals;
    if (str_1.size() != str_2.size())
        throw std::invalid_argument("String {"s + std::string(str_1) + "} of size "s + std::to_string(str_1.size()) +
                                    " and {"s + std::string(str_2) + "} of size "s + std::to_string(str_2.size()) +
                                    " do not have a meaningful index of difference due to incompatible sizes."s);
    for (std::size_t i = 0; i < str_1.size(); ++i)
        if (str_1[i] != str_2[i]) return i;
    return str_1.size();
}

} // namespace utl::stre

#endif
#endif // module utl::stre
