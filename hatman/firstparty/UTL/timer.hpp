// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/UTL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::timer
// Documentation: https://github.com/DmitriBogdanov/UTL/blob/master/docs/module_timer.md
// Source repo:   https://github.com/DmitriBogdanov/UTL
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTLMODULE_TIMER)
#ifndef UTLHEADERGUARD_TIMER
#define UTLHEADERGUARD_TIMER

// _______________________ INCLUDES _______________________

#include <array>   // array<>
#include <chrono>  // chrono::steady_clock, chrono::nanoseconds, chrono::duration_cast<>
#include <ctime>   // time, time_t, tm, strftime()
#include <string>  // string, to_string()
#include <utility> // forward<>()

// ____________________ DEVELOPER DOCS ____________________

// Global-state timer with built-in formatting. Functions for local date and time.
//
// Uses SFINAE to resolve platform-specific calls to local time (localtime_s() on Windows,
// localtime_r() on Linux), the same can be done with macros. The fact that there seems to
// be no portable way of getting local time before C++20 (which adds "Calendar" part of <chrono>)
// is rather bizzare, but not unmanageable.
//
// This implementation will probably be improved later to provide a more generic API with local
// timers and accumulators, but I haven't figured out a way to do it without increasing verbosity
// in a simple use case.

// ____________________ IMPLEMENTATION ____________________

namespace utl::timer {

// =================
// --- Internals ---
// =================

using _clock = std::chrono::steady_clock;
using _ns    = std::chrono::nanoseconds;

constexpr double _ns_in_ms = 1e6;

constexpr long long _ms_in_sec  = 1000;
constexpr long long _ms_in_min  = 60 * _ms_in_sec;
constexpr long long _ms_in_hour = 60 * _ms_in_min;

inline _clock::time_point _start_timepoint;

[[nodiscard]] inline double _elapsed_time_as_ms() noexcept {
    const auto elapsed = std::chrono::duration_cast<_ns>(_clock::now() - _start_timepoint).count();
    return static_cast<double>(elapsed) / _ns_in_ms;
}

inline void start() noexcept { _start_timepoint = _clock::now(); }

// ==============================
// --- Elapsed Time Functions ---
// ==============================

// --- Elapsed Time as 'double' ---
// --------------------------------

[[nodiscard]] inline double elapsed_ms() noexcept { return _elapsed_time_as_ms(); }
[[nodiscard]] inline double elapsed_sec() noexcept { return _elapsed_time_as_ms() / static_cast<double>(_ms_in_sec); }
[[nodiscard]] inline double elapsed_min() noexcept { return _elapsed_time_as_ms() / static_cast<double>(_ms_in_min); }
[[nodiscard]] inline double elapsed_hours() noexcept {
    return _elapsed_time_as_ms() / static_cast<double>(_ms_in_hour);
}

// --- Elapsed Time as 'std::string' ---
// -------------------------------------

[[nodiscard]] inline std::string elapsed_string_ms() { return std::to_string(elapsed_ms()) + " ms"; }
[[nodiscard]] inline std::string elapsed_string_sec() { return std::to_string(elapsed_sec()) + " sec"; }
[[nodiscard]] inline std::string elapsed_string_min() { return std::to_string(elapsed_min()) + " min"; }
[[nodiscard]] inline std::string elapsed_string_hours() { return std::to_string(elapsed_hours()) + " hours"; }

[[nodiscard]] inline std::string elapsed_string_fullform() {
    long long unaccounted_ms = static_cast<long long>(_elapsed_time_as_ms());

    long long ms    = 0;
    long long min   = 0;
    long long sec   = 0;
    long long hours = 0;

    if (unaccounted_ms > _ms_in_hour) {
        hours += unaccounted_ms / _ms_in_hour;
        unaccounted_ms -= hours * _ms_in_hour;
    }

    if (unaccounted_ms > _ms_in_min) {
        min += unaccounted_ms / _ms_in_min;
        unaccounted_ms -= min * _ms_in_min;
    }

    if (unaccounted_ms > _ms_in_sec) {
        sec += unaccounted_ms / _ms_in_sec;
        unaccounted_ms -= sec * _ms_in_sec;
    }

    ms = unaccounted_ms;

    return std::to_string(hours) + " hours " + std::to_string(min) + " min " + std::to_string(sec) + " sec " +
           std::to_string(ms) + " ms ";
}

// ============================
// --- Local Time Functions ---
// ============================

// - SFINAE to select localtime_s() or localtime_r() -
template <class TimeMoment, class TimeType>
auto _available_localtime_impl(TimeMoment time_moment, TimeType timer)
    -> decltype(localtime_s(std::forward<TimeMoment>(time_moment), std::forward<TimeType>(timer))) {
    return localtime_s(std::forward<TimeMoment>(time_moment), std::forward<TimeType>(timer));
}

template <class TimeMoment, class TimeType>
auto _available_localtime_impl(TimeMoment time_moment, TimeType timer)
    -> decltype(localtime_r(std::forward<TimeType>(timer), std::forward<TimeMoment>(time_moment))) {
    return localtime_r(std::forward<TimeType>(timer), std::forward<TimeMoment>(time_moment));
}

// - Implementation -
[[nodiscard]] inline std::string _datetime_string_with_format(const char* format) {
    std::time_t timer = std::time(nullptr);
    std::tm     time_moment{};

    // Call localtime_s() or localtime_r() depending on which one is present
    _available_localtime_impl(&time_moment, &timer);

    // // Macro version, can be used instead of SFINAE resolution
    // // Get localtime safely (if possible)
    // #if defined(__unix__)
    // localtime_r(&timer, &time_moment);
    // #elif defined(_MSC_VER)
    // localtime_s(&time_moment, &timer);
    // #else
    // // static std::mutex mtx; // mutex can be used to make thread-safe version but add another dependency
    // // std::lock_guard<std::mutex> lock(mtx);
    // time_moment = *std::localtime(&timer);
    // #endif

    // Convert time to C-string
    std::array<char, 100> mbstr;
    std::strftime(mbstr.data(), mbstr.size(), format, &time_moment);

    return std::string(mbstr.data());
}

[[nodiscard]] inline std::string datetime_string() { return _datetime_string_with_format("%Y-%m-%d %H:%M:%S"); }

[[nodiscard]] inline std::string datetime_string_id() { return _datetime_string_with_format("%Y-%m-%d-%H-%M-%S"); }

} // namespace utl::timer

#endif
#endif // module utl::timer
