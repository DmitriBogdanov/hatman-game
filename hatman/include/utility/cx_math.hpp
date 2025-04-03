#pragma once

// _______________________ INCLUDES _______________________

// Includes: std
#include <limits> // numeric_limits<>

// Includes: dependencies

// Includes: project

// ____________________ DEVELOPER DOCS ____________________

// Bits of constexpr math for when we need them

// ____________________ IMPLEMENTATION ____________________



// cx_math::
// - Constexpr math things
namespace cx_math {

// detail::
// - Implementation details, don't touch outside the header
namespace detail {
double constexpr sqrt_newton_raphson(double x, double curr, double prev) {
    return curr == prev ? curr : sqrt_newton_raphson(x, 0.5 * (curr + x / curr), curr);
}
} // namespace detail

double constexpr sqrt(double x) {
    return x >= 0. && x < std::numeric_limits<double>::infinity() ? detail::sqrt_newton_raphson(x, x, 0.)
                                                                  : std::numeric_limits<double>::quiet_NaN();
}

double constexpr sqr(double x) { return x * x; }

// Specific-for-this-game things
constexpr double speed_corresponding_to_jump_height(double g, double height) { return sqrt(2. * g * height); }

} // namespace cx_math
