// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/UTL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::sleep
// Documentation: https://github.com/DmitriBogdanov/UTL/blob/master/docs/module_sleep.md
// Source repo:   https://github.com/DmitriBogdanov/UTL
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTLMODULE_SLEEP)
#ifndef UTLHEADERGUARD_SLEEP
#define UTLHEADERGUARD_SLEEP

// _______________________ INCLUDES _______________________

#include <chrono>  // chrono::steady_clock, chrono::nanoseconds, chrono::duration_cast<>
#include <cmath>   // sqrt()
#include <cstdint> // int64_t
#include <thread>  // this_thread::sleep_for()

// ____________________ DEVELOPER DOCS ____________________

// Various implementation of sleep(), used for precise delays.
//
// # ::spinlock() #
// Best precision, uses CPU.
//
// # ::hybrid() #
// Recommended option, similar precision to spinlock with minimal CPU usage.
// Loops short system sleep while statistically estimating its error on the fly and once within error
// margin of the end time, finished with spinlock sleep (essentialy negating usual system sleep error).
//
// # ::system() #
// Worst precision, frees CPU.

// ____________________ IMPLEMENTATION ____________________

namespace utl::sleep {

// =============================
// --- Sleep Implementations ---
// =============================

using _clock     = std::chrono::steady_clock;
using _chrono_ns = std::chrono::nanoseconds;

inline void spinlock(double ms) {
    const long long ns              = static_cast<std::int64_t>(ms * 1e6);
    const auto      start_timepoint = _clock::now();

    volatile int i = 0; // volatile 'i' prevents standard-compliant compilers from optimizing away the loop
    while (std::chrono::duration_cast<_chrono_ns>(_clock::now() - start_timepoint).count() < ns) { ++i; }
}

inline void hybrid(double ms) {
    static double       estimate = 5e-3; // initial sleep_for() error estimate
    static double       mean     = estimate;
    static double       m2       = 0;
    static std::int64_t count    = 1;

    // We treat sleep_for(1 ms) as a random variate "1 ms + random_value()"
    while (ms > estimate) {
        const auto start = _clock::now();
        std::this_thread::sleep_for(_chrono_ns(static_cast<std::int64_t>(1e6)));
        const auto end = _clock::now();

        const double observed = std::chrono::duration_cast<_chrono_ns>(end - start).count() / 1e6;
        ms -= observed;

        ++count;

        // Welford's algorithm for mean and unbiased variance estimation
        const double delta = observed - mean;
        mean += delta / static_cast<double>(count);
        m2 += delta * (observed - mean); // intermediate values 'm2' reduce numerical instability
        const double variance = std::sqrt(m2 / static_cast<double>(count - 1));

        estimate = mean + variance; // set estimate 1 standard deviation above the mean
        // can be adjusted to make estimate more or less pessimistic
    }

    utl::sleep::spinlock(ms);
}

inline void system(double ms) { std::this_thread::sleep_for(_chrono_ns(static_cast<std::int64_t>(ms * 1e6))); }

} // namespace utl::sleep

#endif
#endif // module utl::sleep
