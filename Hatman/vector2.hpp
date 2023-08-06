#pragma once

/*
Classes/methods for 2D vector manipulation
*/

#include <cmath> // 'sqrt()', 'sin()', 'cos()', 'acos()', etc
#include <cstddef> // 'std::size_t' type for hash function
#include <functional> // contains 'std::hash()'
#include <string> // conversion to string

#include "ct_math.hpp" // constexpr math functions



// helpers::
// - Related utility functions
namespace helpers {
	constexpr double PI = 3.1415926;

	constexpr double degree_to_rad(double degrees) {
		return degrees * helpers::PI / 180.;
	}
	constexpr double rad_to_degree(double radians) {
		return radians * 180. / helpers::PI;
	}

	template<typename T>
	constexpr int sign(T val) { // standard 'sign()' function (x<0 => -1, x==0 => 0, x>0 => 1)
		return (T(0) < val) - (val < T(0));
	}
}



// # Vector2 #
// - Represents int 2D vector
class Vector2 {
public:
	constexpr Vector2() : x(0), y(0) {}
	constexpr Vector2(int X, int Y) : x(X), y(Y) {}

	int x, y;

	// Operators
	constexpr bool operator == (const Vector2 &other) const {
		return (this->x == other.x) && (this->y == other.y);
	}

	constexpr bool operator != (const Vector2 &other) const {
		return (this->x != other.x) || (this->y != other.y);
	}

	constexpr Vector2 operator + (const Vector2 &other) const {
		return Vector2(
			this->x + other.x,
			this->y + other.y);
	}

	constexpr Vector2 operator - (const Vector2 &other) const {
		return Vector2(
			this->x - other.x,
			this->y - other.y
		);
	}

	constexpr Vector2 operator * (double value) const {
		return Vector2(
			static_cast<int>(this->x * value),
			static_cast<int>(this->y * value)
		);
	}

	constexpr Vector2 operator / (double value) const {
		return Vector2(
			static_cast<int>(static_cast<double>(this->x) / value),
			static_cast<int>(static_cast<double>(this->y) / value)
		);
	}

	constexpr Vector2& operator += (const Vector2 &other) {
		this->x += other.x;
		this->y += other.y;
		return *this;
	}

	constexpr Vector2& operator -= (const Vector2 &other) {
		this->x -= other.x;
		this->y -= other.y;
		return *this;
	}

	constexpr Vector2& operator *= (double value) {
		this->x = static_cast<int>(this->x * value); // using *= would result in a warning due to double->int implicit conversion
		this->y = static_cast<int>(this->y * value);
		return *this;
	}

	constexpr Vector2& operator /= (double value) {
		this->x = static_cast<int>(this->x / value); // using *= would result in a warning due to double->int implicit conversion
		this->y = static_cast<int>(this->y / value);
		return *this;
	}

	// Methods
	constexpr Vector2& set(int x, int y) {
		this->x = x;
		this->y = y;
		return *this;
	}

	constexpr int length2() const { // square of length
		return this->x * this->x + this->y * this->y;
	}

	inline double length() const { // slower than length2()
		return std::sqrt(this->x * this->x + this->y * this->y);
	}

	inline std::string toString() const {
		return std::string("{" + std::to_string(this->x) + ", " + std::to_string(this->y) + "}");
	}
};

// Hash function
struct Vector2_Hash {
	// Hash function
	std::size_t operator () (const Vector2 &vector) const {
		// Hashes both fiels and apply bitwise XOR
		std::size_t h1 = std::hash<int>()(vector.x);
		std::size_t h2 = std::hash<int>()(vector.y);

		return h1 ^ (h2 << 1);
		// If we didn't shift the bits and two vectors were the same,
		// the XOR would cause them to cancel each other out.
		// So hash(A,A,1) would be the same as hash(B,B,1).
		// Also order wouldn't matter, so hash(A,B,1) would be the same as hash(B,A,1)
	}
};



// # Vector2d #
// - Represents double 2D vector
class Vector2d {
public:
	constexpr Vector2d() : x(0.), y(0.) {}
	constexpr Vector2d(double X, double Y) : x(X), y(Y) {}
	constexpr Vector2d(const Vector2 &vector) : x(vector.x), y(vector.y) {} // Vector2 implicitly converts to a wider tupe

	double x, y;

	// Operators
	constexpr bool operator == (const Vector2d &other) const {
		return (this->x == other.x) && (this->y == other.y);
	}

	constexpr bool operator != (const Vector2d &other) const {
		return (this->x != other.x) || (this->y != other.y);

	}

	constexpr Vector2d operator + (const Vector2d &other) const {
		return Vector2d(
			this->x + other.x,
			this->y + other.y
		);
	}

	constexpr Vector2d operator - (const Vector2d &other) const {
		return Vector2d(
			this->x - other.x,
			this->y - other.y
		);
	}

	constexpr Vector2d operator * (double value) const {
		return Vector2d(
			this->x * value,
			this->y * value
		);
	}

	constexpr Vector2d operator / (double value) const {
		return Vector2d(
			this->x / value,
			this->y / value
		);
	}

	constexpr Vector2d& operator += (const Vector2d &other) {
		this->x += other.x;
		this->y += other.y;
		return *this;
	}

	constexpr Vector2d& operator -= (const Vector2d &other) {
		this->x -= other.x;
		this->y -= other.y;
		return *this;
	}

	constexpr Vector2d& operator *= (double value) {
		this->x *= value;
		this->y *= value;
		return *this;
	}

	constexpr Vector2d& operator /= (double value) {
		this->x /= value;
		this->y /= value;
		return *this;
	}

	// Methods
	constexpr Vector2d& set(double x, double y) {
		this->x = x;
		this->y = y;
		return *this;
	}
	/// LOOK FOR OPTIMIZATIONS
	Vector2d& rotate(double radians) {
		const auto x1 = this->x;
		const auto y1 = this->y;
		this->x = x1 * std::cos(radians) - y1 * std::sin(radians);
		this->y = x1 * std::sin(radians) + y1 * std::cos(radians);

		return *this;
	}
	/// LOOK FOR OPTIMIZATIONS
	Vector2d& rotateDegrees(double degrees) {
		this->rotate(helpers::degree_to_rad(degrees));

		return *this;
	}

	/// LOOK FOR OPTIMIZATIONS
	double angleToX() const { // check out CORDIC if better performance is needed
		return std::acos(this->x / this->length()) * helpers::sign(this->y);
	}
	/// LOOK FOR OPTIMIZATIONS
	double andgleToY() const {
		return std::acos(this->y / this->length()) * helpers::sign(this->x);
	}

	double length2() const { // square of length
		return this->x * this->x + this->y * this->y;
	}

	inline double length() const { // slower than length2()
		return std::sqrt(this->x * this->x + this->y * this->y);
	}

	constexpr Vector2d normalized() const {
		return (*this / this->length());
	}

	constexpr Vector2 toVector2() const { // convertion to Vector2 loses precision, only explicit conversion is allowed
		return Vector2(
			static_cast<int>(this->x),
			static_cast<int>(this->y)
		);
	}

	inline std::string toString() const {
		return std::string("{" + std::to_string(this->x) + ", " + std::to_string(this->y) +"}");
	}
};

/// TRY IMPLEMENTING CONSTEXPR VERSION
inline Vector2d make_rotated_Vector2d(double length, double rotation) {
	return Vector2d(length, 0.).rotate(rotation);
}