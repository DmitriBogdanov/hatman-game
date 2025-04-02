#pragma once

/* Contains classes that deal with geometry */

#include <initializer_list>
#include <algorithm> // std::min(), std::max()
#include <cstdlib> // abs(), rand()

#include "utility/vector2.hpp" // 'Vector2d' type used in 'dRect'



template<typename T>
constexpr int sign(T val) { // standard 'sign()' function (x<0 => -1, x==0 => 0, x>0 => 1)
	return (T(0) < val) - (val < T(0));
}



// # Side #
// - Represents sides of a rectangle and directions
enum class Side {
	NONE = 0,
	LEFT = 1,
	RIGHT = 2,
	TOP = 3,
	BOTTOM = 4
};

constexpr int sign(Side side) {
	switch (side) {
	case Side::LEFT: return -1;
	case Side::RIGHT: return 1;
	case Side::TOP: return -1;
	case Side::BOTTOM: return 1;
	default: return 0;
	}
}

constexpr Side invert(Side side) {
	switch (side) {
	case Side::LEFT: return Side::RIGHT;
	case Side::RIGHT: return Side::LEFT;
	case Side::TOP: return Side::BOTTOM;
	case Side::BOTTOM: return Side::TOP;
	default: return Side::NONE;
	}
}



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

constexpr Orientation invert(Orientation orientation) {
	return static_cast<Orientation>(-static_cast<int>(orientation));
}



// # srcRect #
// - Rect with int dimensions, used as a source rect when handling texture sheets
struct srcRect {
	int x, y;
	int w, h;
};

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

	constexpr bool containsPoint(const Vector2d &point) {
		return
			(this->x < point.x&& point.x < this->x + this->w) &&
			(this->y < point.y&& point.y < this->y + this->h);
	}
};

constexpr dstRect make_dstRect(const Vector2d &point, const Vector2d &size, bool initializeAsCentered = false) {
	if (initializeAsCentered) {
		return dstRect{ point.x - size.x / 2, point.y - size.y / 2, size.x, size.y };
	}
	else {
		return dstRect{ point.x, point.y, size.x, size.y };
	}
}

constexpr dstRect make_dstRect(double x, double y, double width, double height, bool initializeAsCentered = false) {
	if (initializeAsCentered) {
		return dstRect{ x - width / 2, y - height / 2, width, height };
	}
	else {
		return dstRect{ x, y, width, height };
	}
}



// helpers::
// - Small helper functions
namespace helpers {
	constexpr int roundUp32(int val) {
		// round value UP to a multiple of 32
		constexpr int POWER_OF_TWO = 32;
		return (val + POWER_OF_TWO - 1) & -POWER_OF_TWO; // bithacks allow ~4X faster rounding for powers of 2
	} 

	// Methods for getting cell index from position
	constexpr Vector2 divide32(const Vector2d &position) {
		return (position / 32.).toVector2(); // returns floor(position / 32)
	}

	constexpr Vector2 divide32(const Vector2 &position) {
		return position / 32;
	}

	constexpr int divide32(double value) {
		return static_cast<int>(value / 32.); // sometimes we only need 1 coordinate
	}
}



// # dRect #
// - Represents 'float' rectangle
// - Used for physics, hitboxes, etc
class dRect {
public:
	constexpr dRect() = default; // inits point1 and point2 as (0, 0)

	constexpr dRect(const Vector2d &point, const Vector2d &size, bool initializeAsCentered = false) {
		// 'point' is either a top left corner or a center depending on 'initializeAsCentered' value
		if (initializeAsCentered) {
			this->point1 = point - size / 2.;
			this->point2 = point + size / 2.;
		}
		else {
			this->point1 = point;
			this->point2 = point + size;
		}
	}

	constexpr dRect(double pointX, double pointY, double sizeX, double sizeY, bool initializeAsCentered = false) :
		dRect(Vector2d(pointX, pointY), Vector2d(sizeX, sizeY), initializeAsCentered)
	{}
	
	// Move methods
	constexpr dRect& moveCornerTo(const Vector2d &newPoint1) {
		// moves 'point1' (top-left corner) TO the 'newPoint1' without changing size
		const auto size = this->getSize();
		this->point1 = newPoint1;
		this->point2 = newPoint1 + size;

		return *this;
	}

	constexpr dRect& moveCornerTo(double newPoint1X, double newPoint1Y) {
		return this->moveCornerTo(Vector2d(newPoint1X, newPoint1Y));
	}

	constexpr dRect& moveCenterTo(const Vector2d &newCenter) {
		// move center point TO the 'newCenter' without changing dimensions
		const auto size = this->getSize();
		this->point1 = newCenter - size / 2.;
		this->point2 = newCenter + size / 2.;

		return *this;
	} 

	constexpr dRect& moveCenterTo(double newCenterX, double newCenterY) {
		return this->moveCenterTo(Vector2d(newCenterX, newCenterY));
	}

	constexpr dRect& moveBy(const Vector2d &movement) {
		// move rectangle point BY <movement> without changing dimensions
		this->point1 += movement;
		this->point2 += movement;

		return *this;
	} 

	constexpr dRect& moveBy(double movementX, double movementY) {
		this->point1.x += movementX;
		this->point2.x += movementX;
		this->point1.y += movementY;
		this->point2.y += movementY;

		return *this;
	}

	constexpr dRect& moveByX(double movementX) {
		this->point1.x += movementX;
		this->point2.x += movementX;

		return *this;
	}

	constexpr dRect& moveByY(double movementY) {
		this->point1.y += movementY;
		this->point2.y += movementY;

		return *this;
	}

	constexpr dRect& moveSideTo(Side side, double newValue) {
		// move rectangle side to the 'newValue' without changing dimensions
		const auto size = this->getSize();
		switch (side) {
		case Side::LEFT:
			this->point1.x = newValue;
			this->point2.x = newValue + size.x;
			break;
		case Side::RIGHT:
			this->point2.x = newValue;
			this->point1.x = newValue - size.x;
			break;
		case Side::TOP:
			this->point1.y = newValue;
			this->point2.y = newValue + size.y;
			break;
		case Side::BOTTOM:
			this->point2.y = newValue;
			this->point1.y = newValue - size.y;
			break;
		case Side::NONE:
			break;
		}

		return *this;
	} 

	constexpr dRect& moveLeftTo(double newValue) {
		const auto sizeX = this->getSizeX();

		this->point1.x = newValue;
		this->point2.x = newValue + sizeX;

		return *this;
	}

	constexpr dRect& moveRightTo(double newValue) {
		const auto sizeX = this->getSizeX();

		this->point1.x = newValue - sizeX;
		this->point2.x = newValue;

		return *this;
	}

	constexpr dRect& moveTopTo(double newValue) {
		const auto sizeY = this->getSizeY();

		this->point1.y = newValue;
		this->point2.y = newValue + sizeY;

		return *this;
	}

	constexpr dRect& moveBottomTo(double newValue) {
		const auto sizeY = this->getSizeY();

		this->point1.y = newValue - sizeY;
		this->point2.y = newValue;

		return *this;
	}

	// Scale methods
	constexpr dRect& scaleInPlaceTo(const Vector2d &newSize) {
		// set new size without changing center position
		const auto center = this->getCenter();

		*this = dRect(center, newSize, true);

		return *this;
	} 

	constexpr dRect& scaleInPlaceBy(double scale) {
		// scale rectangle without changing center position
		const auto center = this->getCenter();
		const auto size = this->getSize();

		*this = dRect(center, size * scale, true);

		return *this;
	} 
	
	// Corner getters
	constexpr Vector2d getCornerTopLeft() const { return this->point1; }

	constexpr Vector2d getCornerTopRight() const { return Vector2d(this->point2.x, this->point1.y); }

	constexpr Vector2d getCornerBottomLeft() const { return Vector2d(this->point1.x, this->point2.y); }

	constexpr Vector2d getCornerBottomRight() const { return this->point2; }

	// Center getters
	constexpr Vector2d getCenter() const { return (this->point1 + this->point2) / 2.; }

	constexpr double getCenterX() const { return (this->point1.x + this->point2.x) / 2.; }

	constexpr double getCenterY() const { return (this->point1.y + this->point2.y) / 2.; }

	// Size getters
	constexpr Vector2d getSize() const { return this->point2 - this->point1; }
	constexpr double getSizeX() const { return this->point2.x - this->point1.x; }
	constexpr double getSizeY() const { return this->point2.y - this->point1.y; }
	
	// Side getters
	constexpr double getSide(Side side) const {
		switch (side) {
		case Side::BOTTOM:
			return this->point2.y;
		case Side::TOP:
			return this->point1.y;
		case Side::LEFT:
			return this->point1.x;
		case Side::RIGHT:
			return this->point2.x;
		default:
			return -1.;
		}
	}

	constexpr double getLeft() const { return this->point1.x; }
	constexpr double getRight() const { return this->point2.x; }
	constexpr double getTop() const { return this->point1.y; }
	constexpr double getBottom() const { return this->point2.y; }

	// Side middlepoint getters
	constexpr Vector2d getSideMiddlepoint(Side side) const {
		switch (side) {
		case Side::LEFT:
			return this->getLeftMiddlepoint();
		case Side::RIGHT:
			return this->getRightMiddlepoint();
		case Side::TOP:
			return this->getTopMiddlepoint();
		case Side::BOTTOM:
			return this->getBottomMiddlepoint();
		default:
			return Vector2();
		}
	}

	constexpr Vector2d getLeftMiddlepoint() const {
		return Vector2d(this->point1.x, (this->point1.y + this->point2.y) / 2.);
	}

	constexpr Vector2d getRightMiddlepoint() const {
		return Vector2d(this->point2.x, (this->point1.y + this->point2.y) / 2.);
	}

	constexpr Vector2d getTopMiddlepoint() const {
		return Vector2d((this->point1.x + this->point2.x) / 2., this->point1.y);
	}

	constexpr Vector2d getBottomMiddlepoint() const {
		return Vector2d((this->point1.x + this->point2.x) / 2., this->point2.y);
	}
	
	// Intersection methods
	struct CollisionInfo {
		double overlap_x;
		double overlap_y;
		double overlap_area; // == 0 => rects don't collide
	};

	constexpr CollisionInfo collideWithRect(const dRect &other) const {
		const double overlapX = std::max(
			0.,
			std::min(this->point2.x, other.point2.x) - std::max(this->point1.x, other.point1.x)
		);
		const double overlapY = std::max(
			0.,
			std::min(this->point2.y, other.point2.y) - std::max(this->point1.y, other.point1.y)
		);

		return dRect::CollisionInfo{ overlapX, overlapY, overlapX * overlapY }; // provides collision info for 2 rects
	} 

	constexpr bool overlapsWithRect(const dRect &other) const {
		// faster than colliding but only gives bool result
		return
			!(other.point1.x >= this->point2.x ||
				this->point1.x >= other.point2.x ||
				this->point1.y >= other.point2.y ||
				other.point1.y >= this->point2.y); // true if intersection is impossible => invert it to get the result
	} 

	constexpr bool containsPoint(const Vector2d &point) const {
		return
			(this->getLeft() < point.x) &&
			(this->getRight() > point.x) &&
			(this->getTop() < point.y) &&
			(this->getBottom() > point.y);
	}
	
	// Convertions
	constexpr dstRect to_dstRect() const {
		return dstRect{
			this->point1.x,
			this->point1.y,
			this->getSizeX(),
			this->getSizeY()
		};
	}

private:	
	Vector2d point1; // top-left corner
	Vector2d point2; // bottom-right corner
};



// Random functions
bool rand_bool();
int rand_int(int min, int max); // random int in [min, max] range
double rand_double(); // random double in (0, 1] range
double rand_double(double min, double max); // random double in (min, max] range

template<class T>
T rand_linear_combination(const T& A, const T& B) { // random linear combination of 2 colors/vectors/etc
	const auto coef = rand_double();
	return A * coef + B * (1. - coef);
}

template<class T>
const T& rand_choise(std::initializer_list<T> objects) {
	return objects.begin()[rand_int(0, objects.size() - 1)];
}