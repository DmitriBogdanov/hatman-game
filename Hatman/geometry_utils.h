#pragma once

/* Contains classes that deal with geometry */

#include <SDL.h> // 'SDL_Rect' type (.to_SDL_Rect() method)
#include <initializer_list>

#include "vector2.hpp" // 'Vector2d' type used in 'dRect'



// # Side #
// - Represents sides of a rectangle and directions
enum class Side {
	NONE = 0,
	LEFT = 1,
	RIGHT = 2,
	TOP = 3,
	BOTTOM = 4
};



// # Orientation #
// - Like side but only LEFT/RIGHT
// - Mostly used by creatures
enum class Orientation {
	LEFT = -1,
	RIGHT = 1	
};

constexpr int sign(Orientation orientation) {
	return static_cast<int>(orientation);
}

template<typename T>
constexpr int sign(T val) { // standard 'sign()' function (x<0 => -1, x==0 => 0, x>0 => 1)
	return (T(0) < val) - (val < T(0));
}

constexpr Orientation invert(Orientation orientation) {
	return static_cast<Orientation>(-static_cast<int>(orientation));
}

// # srcRect #
// - Rect with int dimensions, used as a source rect when handling texture sheets
using srcRect = SDL_Rect;

constexpr srcRect make_srcRect(const Vector2 &point, const Vector2 &size, bool initializeAsCentered = false) {
	if (initializeAsCentered) {
		return srcRect{ point.x - size.x / 2, point.y - size.y / 2, size.x, size.y };
	}
	else {
		return srcRect{ point.x, point.y, size.x, size.y };
	}
}
constexpr srcRect make_srcRect(int x, int y, int width, int height, bool initializeAsCentered = false) {
	if (initializeAsCentered) {
		return srcRect{ x - width / 2, y - height / 2, width, height };
	}
	else {
		return srcRect{ x, y, width, height };
	}
}

// # dstRect #
// - Rect with double dimensions, used during rendering to allow proper texture scaling
struct dstRect {
	double x, y;
	double w, h;

	bool containsPoint(const Vector2d &point); // used in buttons
};

dstRect make_dstRect(const Vector2d &point, const Vector2d &size, bool initializeAsCentered = false);
dstRect make_dstRect(double x, double y, double width, double height, bool initializeAsCentered = false);



// helpers::
// - Small helper functions
namespace helpers {
	Side invert(Side side); // returns opposite side

	int roundUp32(int val); // rounds value UP to a multiple of 32

	Vector2 divide32(const Vector2d &position); // returns floor(position / 32)
	Vector2 divide32(const Vector2 &position);
	int divide32(double value); // sometimes we only need 1 coordinate

	bool rand_bool();
	int rand_int(int min, int max); // random int in [min, max] range
	double rand_double(); // random double in (0, 1] range
	double rand_double(double min, double max); // random double in (min, max] range
	
	template<class T>
	T rand_linear_combination(const T &A, const T &B) { // random linear combination of 2 colors/vectors/etc
		const auto coef = rand_double();
		return A * coef + B * (1. - coef);
	}

	template<class T>
	const T& rand_choise(std::initializer_list<T> objects) {
		return objects.begin()[rand_int(0, objects.size() - 1)];
	}
}



// # dRect #
// - Represents 'float' rectangle
// - Used for physics, hitboxes, etc
class dRect {
public:
	dRect(); // inits point1 and point2 as (0, 0)
	dRect(const Vector2d &point, const Vector2d &size, bool initializeAsCentered = false); // <point> is either a top left corner or a center depending on <initializeAsCentered> value
	dRect(double pointX, double pointY, double sizeX, double sizeY, bool initializeAsCentered = false);
	
	// Move methods
	void moveCornerTo(const Vector2d &newPoint1); // moves point1 (top-left corner) TO the <newPoint1> without changing dimensions 
	void moveCornerTo(double newPoint1X, double newPoint1Y);

	void moveCenterTo(const Vector2d &newCenter); // moves center point TO the <newCenter> without changing dimensions
	void moveCenterTo(double newCenterX, double newCenterY);

	void moveBy(const Vector2d &movement); // moves rectangle point BY <movement> without changing dimensions
	void moveBy(double movementX, double movementY);
	void moveByX(double movementX);
	void moveByY(double movementY);

	void moveSideTo(Side side, double newValue); // moves rectangle side to the <newValue> without changing dimensions
	void moveLeftTo(double newValue);
	void moveRightTo(double newValue);
	void moveTopTo(double newValue);
	void moveBottomTo(double newValue);

	// Scale methods
	void scaleInPlaceTo(const Vector2d &newSize); // sets new dimensions without changing center position
	void scaleInPlaceBy(double scale); // scales rectangle without changing center position
	
	// Corner getters
	Vector2d getCornerTopLeft() const;
	Vector2d getCornerTopRight() const;
	Vector2d getCornerBottomLeft() const;
	Vector2d getCornerBottomRight() const;

	// Center getters
	Vector2d getCenter() const;
	double getCenterX() const;
	double getCenterY() const;

	// Size getters
	Vector2d getSize() const;
	double getSizeX() const;
	double getSizeY() const;
	
	// Side getters
	double getSide(Side side) const;
	double getLeft() const;
	double getRight() const;
	double getTop() const;
	double getBottom() const;

	// Side middlepoint getters
	Vector2d getSideMiddlepoint(Side side) const;
	Vector2d getLeftMiddlepoint() const;
	Vector2d getRightMiddlepoint() const;
	Vector2d getTopMiddlepoint() const;
	Vector2d getBottomMiddlepoint() const;
	
	// Intersection methods
	struct CollisionInfo {
		double overlap_x;
		double overlap_y;
		double overlap_area; // == 0 => rects don't collide
	};

	CollisionInfo collideWithRect(const dRect &other) const; // provides collision info for 2 rects

	bool overlapsWithRect(const dRect &other) const; // faster than colliding but only gives bool result

	bool containsPoint(const Vector2d &point) const;
	
	// Convertions
	SDL_Rect to_SDL_Rect() const;
	srcRect to_srcRect() const;
	dstRect to_dstRect() const;

private:	
	Vector2d point1; // top-left corner
	Vector2d point2; // bottom-right corner
};
