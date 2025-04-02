// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/UTL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::progressbar
// Documentation: https://github.com/DmitriBogdanov/UTL/blob/master/docs/module_progressbar.md
// Source repo:   https://github.com/DmitriBogdanov/UTL
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTLMODULE_PROGRESSBAR)
#ifndef UTLHEADERGUARD_PROGRESSBAR
#define UTLHEADERGUARD_PROGRESSBAR

// _______________________ INCLUDES _______________________

#include <algorithm> // fill_n
#include <chrono>    // chrono::steady_clock, chrono::time_point<>, chrono::duration_cast<>, chrono::seconds
#include <cmath>     // floor()
#include <cstddef>   // size_t
#include <iomanip>   // setprecision()
#include <ios>       // fixed
#include <iostream>  // cout
#include <iterator>  // ostreambuf_iterator<>
#include <ostream>   // ostream
#include <sstream>   // ostringstream
#include <string>    // string

// ____________________ DEVELOPER DOCS ____________________

// Simple progress bars for terminal applications. Rendered in ASCII on the main thread with manual updates
// for maximal compatibility. Perhaps can be extended with some fancier async options that display animations.
//
// # ::set_ostream() #
// Sets ostream used for progress bars.
//
// # ::Percentage #
// Proper progress bar, uses carriage return escape sequence (\r) to render new states in the same spot.
// Shows an estimate of remaining time.
//
// # ::Ruler #
// Primitive & lightweight progress bar, useful when terminal has no proper support for escape sequences.

// ____________________ IMPLEMENTATION ____________________

namespace utl::progressbar {

inline std::ostream* _output_stream = &std::cout;

inline void set_ostream(std::ostream& new_ostream) { _output_stream = &new_ostream; }

class Percentage {
private:
    char done_char;
    char not_done_char;
    bool show_time_estimate;

    std::size_t length_total;   // full   bar length
    std::size_t length_current; // filled bar length

    double last_update_percentage;
    double update_rate;

    using Clock     = std::chrono::steady_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    TimePoint timepoint_start; // used to estimate remaining time
    TimePoint timepoint_current;

    int previous_string_length; // used to properly return the carriage when dealing with changed string size

    void draw_progressbar(double percentage) {
        const auto displayed_percentage = this->update_rate * std::floor(percentage / this->update_rate);
        // floor percentage to a closest multiple of 'update_rate' for nicer display
        // since actual updates are only required to happen no more ofter than 'update_rate'
        // and don't have to correspond to exact multiples of it

        // Estimate remaining time (linearly) and format it min + sec
        const auto time_elapsed       = this->timepoint_current - this->timepoint_start;
        const auto estimate_full      = time_elapsed / percentage;
        const auto estimate_remaining = estimate_full - time_elapsed;

        const auto estimate_remaining_sec = std::chrono::duration_cast<std::chrono::seconds>(estimate_remaining);

        const auto displayed_min = (estimate_remaining_sec / 60ll).count();
        const auto displayed_sec = (estimate_remaining_sec % 60ll).count();

        const bool show_min  = (displayed_min != 0);
        const bool show_sec  = (displayed_sec != 0) && !show_min;
        const bool show_time = (estimate_remaining_sec.count() > 0);

        std::ostringstream ss;

        // Print bar
        ss << '[';
        std::fill_n(std::ostreambuf_iterator<char>(ss), this->length_current, this->done_char);
        std::fill_n(std::ostreambuf_iterator<char>(ss), this->length_total - this->length_current, this->not_done_char);
        ss << ']';

        // Print percentage
        ss << ' ' << std::fixed << std::setprecision(2) << 100. * displayed_percentage << '%';

        // Print time estimate
        if (this->show_time_estimate && show_time) {
            ss << " (remaining:";
            if (show_min) ss << ' ' << displayed_min << " min";
            if (show_sec) ss << ' ' << displayed_sec << " sec";
            ss << ')';
        }

        const std::string bar_string = ss.str();

        // Add spaces at the end to overwrite the previous string if it was longer than current
        const int current_string_length = static_cast<int>(bar_string.length());
        const int string_length_diff    = this->previous_string_length - current_string_length;

        if (string_length_diff > 0) { std::fill_n(std::ostreambuf_iterator<char>(ss), string_length_diff, ' '); }

        this->previous_string_length = current_string_length;

        // Return the carriage
        (*_output_stream) << ss.str(); // don't reuse 'bar_string', now 'ss' can also contain spaces at the end
        (*_output_stream) << '\r';
        (*_output_stream).flush();
        // '\r' returns cursor to the beginning of the line => most sensible consoles will
        // render render new lines over the last one. Otherwise every update produces a
        // bar on a new line, which looks worse but isn't critical for the purpose.
    }

public:
    Percentage(char done_char = '#', char not_done_char = '.', std::size_t bar_length = 30, double update_rate = 1e-2,
               bool show_time_estimate = true)
        : done_char(done_char), not_done_char(not_done_char), show_time_estimate(show_time_estimate),
          length_total(bar_length), length_current(0), last_update_percentage(0), update_rate(update_rate),
          timepoint_start(Clock::now()), previous_string_length(static_cast<int>(bar_length) + sizeof("[] 100.00%")) {}

    void start() {
        this->last_update_percentage = 0.;
        this->length_current         = 0;
        this->timepoint_start        = Clock::now();
        (*_output_stream) << '\n';
    }

    void set_progress(double percentage) {
        if (percentage - this->last_update_percentage <= this->update_rate) return;

        this->last_update_percentage = percentage;
        this->length_current         = static_cast<std::size_t>(percentage * static_cast<double>(this->length_total));
        this->timepoint_current      = Clock::now();
        this->draw_progressbar(percentage);
    }

    void finish() {
        this->last_update_percentage = 1.;
        this->length_current         = this->length_total;
        this->draw_progressbar(1.);
        (*_output_stream) << '\n';
    }
};

class Ruler {
private:
    char done_char;

    std::size_t length_total;
    std::size_t length_current;

public:
    Ruler(char done_char = '#') : done_char(done_char), length_total(51), length_current(0) {}

    void start() {
        this->length_current = 0;

        (*_output_stream) << '\n'
                          << " 0    10   20   30   40   50   60   70   80   90   100%\n"
                          << " |----|----|----|----|----|----|----|----|----|----|\n"
                          << ' ';
    }

    void set_progress(double percentage) {
        const std::size_t length_new = static_cast<std::size_t>(percentage * static_cast<double>(this->length_total));

        if (length_new > length_current) {
            const auto chars_to_add = length_new - this->length_current;
            std::fill_n(std::ostreambuf_iterator<char>(*_output_stream), chars_to_add, this->done_char);
        }

        this->length_current = length_new;
    }

    void finish() {
        if (this->length_total > this->length_current) {
            const auto chars_to_add = this->length_total - this->length_current;
            std::fill_n(std::ostreambuf_iterator<char>(*_output_stream), chars_to_add, this->done_char);
        }

        this->length_current = this->length_total;

        (*_output_stream) << '\n';
    }
};

} // namespace utl::progressbar

#endif
#endif // module utl::progressbar
