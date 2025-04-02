// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/UTL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::table
// Documentation: https://github.com/DmitriBogdanov/UTL/blob/master/docs/module_table.md
// Source repo:   https://github.com/DmitriBogdanov/UTL
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTLMODULE_TABLE)
#ifndef UTLHEADERGUARD_TABLE
#define UTLHEADERGUARD_TABLE

// _______________________ INCLUDES _______________________

#include <cstddef>          // size_t
#include <initializer_list> // initializer_list<>
#include <iomanip>          // resetiosflags(), setw()
#include <ios>              // streamsize, ios_base::fmtflags, ios
#include <iostream>         // cout
#include <ostream>          // ostream
#include <sstream>          // ostringstream
#include <string>           // string
#include <type_traits>      // is_arithmetic_v<>, is_same_v<>
#include <vector>           // vector<>

// ____________________ DEVELOPER DOCS ____________________

// Functions used to build and render simple ASCII table in console.
//
// Tries to be simple and minimize boilerplate, exposes a LaTeX-like API.
// In fact this is for a reason - these tables can be formatted for a quick LaTeX export
// by enabling a 'set_latex_mode(true)'.
//
// As of now the implementation is short but quite frankly ugly, it feels like with some thought
// it could be generalized much better to support multiple styles and perform faster, however there
// is ~0 need for this to be fast since it's mean for human-readable tables and not massive data export.
// Adding more styles while nice also doesn't seem like an important thing as of now so the old implementation
// is left to be as is.

// ____________________ IMPLEMENTATION ____________________

namespace utl::table {

// =====================
// --- Column Format ---
// =====================

using uint       = std::streamsize;
using _ios_flags = std::ios_base::fmtflags;

struct ColumnFormat {
    _ios_flags flags;
    uint       precision;
};

struct _Column {
    uint         width;
    ColumnFormat col_format;
};

// --- Predefined Formats ---
// --------------------------

constexpr ColumnFormat NONE = {std::ios::showpoint, 6};

constexpr ColumnFormat FIXED(uint decimals = 3) { return {std::ios::fixed, decimals}; }
constexpr ColumnFormat DEFAULT(uint decimals = 6) { return {std::ios::showpoint, decimals}; }
constexpr ColumnFormat SCIENTIFIC(uint decimals = 3) { return {std::ios::scientific, decimals}; }

constexpr ColumnFormat BOOL = {std::ios::boolalpha, 3};

// --- Internal Table State ---
// ----------------------------

inline std::vector<_Column> _columns;
inline std::size_t          _current_column = 0;
inline std::ostream*        _output_stream  = &std::cout;
inline bool                 _latex_mode     = false;

// ===================
// --- Table Setup ---
// ===================

inline void create(std::initializer_list<uint>&& widths) {
    _columns.resize(widths.size());
    for (std::size_t i = 0; i < _columns.size(); ++i) {
        _columns[i].width      = widths.begin()[i];
        _columns[i].col_format = DEFAULT();
    }
}

inline void set_formats(std::initializer_list<ColumnFormat>&& formats) {
    for (std::size_t i = 0; i < _columns.size(); ++i) _columns[i].col_format = formats.begin()[i];
}

inline void set_ostream(std::ostream& new_ostream) { _output_stream = &new_ostream; }

inline void set_latex_mode(bool toggle) { _latex_mode = toggle; }

// =======================
// --- Table Rendering ---
// =======================

// We want to only apply additional typesetting to "actual mathematical numbers", not bools & chars
template <class T>
constexpr bool _is_arithmetic_number_v =
    std::is_arithmetic_v<T> && !std::is_same_v<T, bool> && !std::is_same_v<T, char>;

[[nodiscard]] inline std::string _trim_left(const std::string& str, char trimmed_char) {
    std::string res = str;
    res.erase(0, res.find_first_not_of(trimmed_char));
    return res;
}

// Function that adds some LaTeX decorators to appropriate types
template <class T>
void _append_decorated_value(std::ostream& os, const T& value) {
    using V = std::decay_t<T>;

    if (!_latex_mode) {
        os << value;
        return;
    }

    if constexpr (_is_arithmetic_number_v<V>) {
        // In order to respect format flags of the table, we have to copy fmt into a stringstream
        // and use IT to stringify a number, simple 'std::to_string()' won't do it here
        std::ostringstream ss;
        ss.copyfmt(os);
        ss.width(0); // cancel out 'std::setw()' that was copied with all the other flags
        ss << value;
        std::string number_string = ss.str();

        // Convert scientific form number to a LaTeX-friendly form,
        // for example, "1.3e-15" becomes "1.3 \cdot 10^{-15}"
        const std::size_t e_index = number_string.find('e');
        if (e_index != std::string::npos) {
            const std::string mantissa = number_string.substr(0, e_index - 1);
            const char        sign     = number_string.at(e_index + 1);
            const std::string exponent = number_string.substr(e_index + 2);

            number_string.clear();
            number_string += mantissa;
            number_string += " \\cdot 10^{";
            if (sign == '-') number_string += sign;
            number_string += _trim_left(exponent, '0');
            number_string += '}';
        }

        // Typeset numbers as formulas 
        os << "$" + number_string + "$";
        // we append it as a single string so ostream 'setw()' doesn't mess up alignment
    } else os << value;
}

inline void cell(){};

template <class T, class... Types>
void cell(const T& value, const Types&... other_values) {
    const auto left_delimer  = _latex_mode ? "" : "|";
    const auto delimer       = _latex_mode ? " & " : "|";
    const auto right_delimer = _latex_mode ? " \\\\\n" : "|\n";

    const std::string left_cline      = (_current_column == 0) ? left_delimer : "";
    const std::string right_cline     = (_current_column == _columns.size() - 1) ? right_delimer : delimer;
    const _ios_flags  format          = _columns[_current_column].col_format.flags;
    const uint        float_precision = _columns[_current_column].col_format.precision;

    // Save old stream state
    std::ios old_state(nullptr);
    old_state.copyfmt(*_output_stream);

    // Set table formatting
    (*_output_stream) << std::resetiosflags((*_output_stream).flags());
    (*_output_stream).flags(format);
    (*_output_stream).precision(float_precision);

    // Print
    (*_output_stream) << left_cline << std::setw(_columns[_current_column].width);
    _append_decorated_value(*_output_stream, value);
    (*_output_stream) << right_cline;

    // Return old stream state
    (*_output_stream).copyfmt(old_state);

    // Advance column counter
    _current_column = (_current_column == _columns.size() - 1) ? 0 : _current_column + 1;

    cell(other_values...);
}

inline void hline() {
    if (_latex_mode) {
        (*_output_stream) << "\\hline\n";
    } else {
        (*_output_stream) << "|";
        for (const auto& col : _columns)
            (*_output_stream) << std::string(static_cast<std::size_t>(col.width), '-') << "|";
        (*_output_stream) << "\n";
    }
}

} // namespace utl::table

#endif
#endif // module utl::table
