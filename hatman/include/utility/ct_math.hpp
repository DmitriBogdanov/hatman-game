#pragma once

/*
* Contains constexpr math functions and convesions
* - Not intended to be used runtime
*/

#include <limits>



// ct_helpers::
// - Contains helper-function for internal logic
// - Don't touch it outside the header
namespace ct_helpers {
    double constexpr sqrtNewtonRaphson(double x, double curr, double prev) {
        return curr == prev
            ? curr
            : sqrtNewtonRaphson(x, 0.5 * (curr + x / curr), curr);
    }
}



constexpr double ct_PI = 3.1415926;

constexpr double ct_degree_to_rad(double degrees) {
    return degrees * ct_PI / 180.;
}

constexpr double ct_rad_to_degree(double radians) {
    return radians * 180. / ct_PI;
}

template<typename T>
constexpr int ct_sign(T val) { // standard 'sign()' function (x<0 => -1, x==0 => 0, x>0 => 1)
    return (T(0) < val) - (val < T(0));
}

double constexpr ct_sqrt(double x) {
    return x >= 0. && x < std::numeric_limits<double>::infinity()
        ? ct_helpers::sqrtNewtonRaphson(x, x, 0.)
        : std::numeric_limits<double>::quiet_NaN();
}

double constexpr ct_sqr(double x) {
    return x * x;
}

// Niche stuff
constexpr double ct_speed_corresponding_to_jump_height(double g, double height) {
    return ct_sqrt(2. * g * height);
}