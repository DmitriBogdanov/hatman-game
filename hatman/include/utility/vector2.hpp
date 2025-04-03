#pragma once

// _______________________ INCLUDES _______________________

#include <cmath>      // sqrt(), sin(), cos(), acos()
#include <cstddef>    // size_t
#include <functional> // std::hash
#include <string>     // string, to_string()

#include "utility/cx_math.hpp" // constexpr math functions

// ____________________ DEVELOPER DOCS ____________________

// 2D integer / float vector class, also a few misc utilities for math

// ____________________ IMPLEMENTATION ____________________



// helpers::
// - Math utils for 2D vectors
namespace helpers {

constexpr double pi = 3.14159265358979323846;

constexpr double degree_to_rad(double degrees) noexcept { return degrees * helpers::pi / 180.; }
constexpr double rad_to_degree(double radians) noexcept { return radians * 180. / helpers::pi; }

template <typename T>
constexpr int sign(T val) { // standard 'sign()' function (x<0 => -1, x==0 => 0, x>0 => 1)
    return (T(0) < val) - (val < T(0));
}

} // namespace helpers



// # Vector2 #
// - Represents int 2D vector
class Vector2 {
public:
    constexpr Vector2()                          = default;
    constexpr Vector2(const Vector2&)            = default;
    constexpr Vector2(Vector2&&)                 = default;
    constexpr Vector2& operator=(const Vector2&) = default;
    constexpr Vector2& operator=(Vector2&&)      = default;

    constexpr Vector2(int X, int Y) : x(X), y(Y) {}

    int x{}, y{};

    // Operators
    constexpr bool operator==(const Vector2& other) const noexcept {
        return (this->x == other.x) && (this->y == other.y);
    }
    constexpr bool operator!=(const Vector2& other) const noexcept {
        return (this->x != other.x) || (this->y != other.y);
    }
    constexpr Vector2 operator+(const Vector2& other) const noexcept {
        return Vector2(this->x + other.x, this->y + other.y);
    }
    constexpr Vector2 operator-(const Vector2& other) const noexcept {
        return Vector2(this->x - other.x, this->y - other.y);
    }
    constexpr Vector2 operator*(double value) const noexcept {
        return Vector2(static_cast<int>(this->x * value), static_cast<int>(this->y * value));
    }
    constexpr Vector2 operator/(double value) const noexcept {
        return Vector2(static_cast<int>(static_cast<double>(this->x) / value),
                       static_cast<int>(static_cast<double>(this->y) / value));
    }
    constexpr Vector2& operator+=(const Vector2& other) noexcept {
        this->x += other.x;
        this->y += other.y;
        return *this;
    }
    constexpr Vector2& operator-=(const Vector2& other) noexcept {
        this->x -= other.x;
        this->y -= other.y;
        return *this;
    }
    constexpr Vector2& operator*=(double value) noexcept {
        this->x = static_cast<int>(this->x * value);
        // using *= would result in a warning due to implicit double -> int conversion
        this->y = static_cast<int>(this->y * value);
        return *this;
    }

    constexpr Vector2& operator/=(double value) noexcept {
        this->x = static_cast<int>(this->x / value);
        // using *= would result in a warning due to implicit double -> int conversion
        this->y = static_cast<int>(this->y / value);
        return *this;
    }

    // Methods
    constexpr Vector2& set(int x, int y) noexcept {
        this->x = x;
        this->y = y;
        return *this;
    }

    constexpr int length2() const noexcept { // square of length
        return this->x * this->x + this->y * this->y;
    }

    double length() const { // slower than length2()
        return std::sqrt(this->x * this->x + this->y * this->y);
    }

    std::string to_string() const { return '{' + std::to_string(this->x) + ", " + std::to_string(this->y) + '}'; }
};

// # Vector2d #
// - Represents double 2D vector
class Vector2d {
public:
    constexpr Vector2d()                           = default;
    constexpr Vector2d(const Vector2d&)            = default;
    constexpr Vector2d(Vector2d&&)                 = default;
    constexpr Vector2d& operator=(const Vector2d&) = default;
    constexpr Vector2d& operator=(Vector2d&&)      = default;

    constexpr Vector2d(double X, double Y) noexcept : x(X), y(Y) {}
    constexpr Vector2d(const Vector2& vector) noexcept : x(vector.x), y(vector.y) {}
    // Vector2 implicitly converts to a wider type

    double x{}, y{};

    // Operators
    constexpr bool operator==(const Vector2d& other) const noexcept {
        return (this->x == other.x) && (this->y == other.y);
    }
    constexpr bool operator!=(const Vector2d& other) const noexcept {
        return (this->x != other.x) || (this->y != other.y);
    }
    constexpr Vector2d operator+(const Vector2d& other) const noexcept {
        return Vector2d(this->x + other.x, this->y + other.y);
    }
    constexpr Vector2d operator-(const Vector2d& other) const noexcept {
        return Vector2d(this->x - other.x, this->y - other.y);
    }
    constexpr Vector2d operator*(double value) const noexcept { return Vector2d(this->x * value, this->y * value); }
    constexpr Vector2d operator/(double value) const noexcept { return Vector2d(this->x / value, this->y / value); }

    constexpr Vector2d& operator+=(const Vector2d& other) noexcept {
        this->x += other.x;
        this->y += other.y;
        return *this;
    }

    constexpr Vector2d& operator-=(const Vector2d& other) noexcept {
        this->x -= other.x;
        this->y -= other.y;
        return *this;
    }

    constexpr Vector2d& operator*=(double value) noexcept {
        this->x *= value;
        this->y *= value;
        return *this;
    }

    constexpr Vector2d& operator/=(double value) noexcept {
        this->x /= value;
        this->y /= value;
        return *this;
    }

    // Methods
    constexpr Vector2d& set(double x, double y) noexcept {
        this->x = x;
        this->y = y;
        return *this;
    }

    Vector2d& rotate(double radians) {
        const auto x1 = this->x;
        const auto y1 = this->y;
        this->x       = x1 * std::cos(radians) - y1 * std::sin(radians);
        this->y       = x1 * std::sin(radians) + y1 * std::cos(radians);

        return *this;
    }

    /// LOOK FOR OPTIMIZATIONS
    double angle_to_x() const { // check out CORDIC if better performance is needed
        return std::acos(this->x / this->length()) * helpers::sign(this->y);
    }
    /// LOOK FOR OPTIMIZATIONS
    double angle_to_y() const { return std::acos(this->y / this->length()) * helpers::sign(this->x); }

    double length2() const noexcept { // square of length
        return this->x * this->x + this->y * this->y;
    }

    double length() const { // slower than length2()
        return std::sqrt(this->x * this->x + this->y * this->y);
    }

    Vector2d normalized() const { return (*this / this->length()); }

    constexpr Vector2 to_Vector2() const { // conversion to Vector2 loses precision, only explicit conversion is allowed
        return Vector2(static_cast<int>(this->x), static_cast<int>(this->y));
    }

    std::string to_string() const {
        return std::string("{" + std::to_string(this->x) + ", " + std::to_string(this->y) + "}");
    }
};