#include "geometry_utils.h"

#include <algorithm> // std::max()
#include <cstdlib> // 'abs()' for int, 'rand()'



Side helpers::invert(Side side) {
	switch (side) {
	case Side::LEFT:
		return Side::RIGHT;
	case Side::RIGHT:
		return Side::LEFT;
	case Side::TOP:
		return Side::BOTTOM;
	case Side::BOTTOM:
		return Side::TOP;
	default:
		return Side::NONE;
	}
}

int helpers::roundUp32(int val) {
	const int POWER_OF_TWO = 32;
	return (val + POWER_OF_TWO - 1) & -POWER_OF_TWO; // bithacks allow ~4X faster rounding for powers of 2
}

Vector2 helpers::divide32(const Vector2d &position) {
	return (position / 32.).toVector2();
}

Vector2 helpers::divide32(const Vector2 &position) {
	return position / 32;
}

int helpers::divide32(double value) {
	return static_cast<int>(value / 32.);
}

bool helpers::rand_bool() {
	return static_cast<bool>(rand() % 2);
}

int helpers::rand_int(int min, int max) {
	return min + rand() % (max - min + 1);
}

double helpers::rand_double() {
	return rand() / (RAND_MAX + 1.);
}

double helpers::rand_double(double min, double max) {
	return min + (max - min) * rand_double();
}



// # srcRect #
//srcRect make_srcRect(const Vector2 &point, const Vector2 &size, bool initializeAsCentered) {
//	if (initializeAsCentered) {
//		return srcRect{ point.x - size.x / 2, point.y - size.y / 2, size.x, size.y };
//	}
//	else {
//		return srcRect{ point.x, point.y, size.x, size.y };
//	}
//}
//
//srcRect make_srcRect(int x, int y, int width, int height, bool initializeAsCentered) {
//	if (initializeAsCentered) {
//		return srcRect{ x - width / 2, y - height / 2, width, height };
//	}
//	else {
//		return srcRect{ x, y, width, height };
//	}
//}



// # dstRect #
bool dstRect::containsPoint(const Vector2d &point) {
	return
		(this->x < point.x && point.x < this->x + this->w) &&
		(this->y < point.y && point.y < this->y + this->h);
}

dstRect make_dstRect(const Vector2d &point, const Vector2d &size, bool initializeAsCentered) {
	if (initializeAsCentered) {
		return dstRect{ point.x - size.x / 2, point.y - size.y / 2, size.x, size.y };
	}
	else {
		return dstRect{ point.x, point.y, size.x, size.y };
	}
}

dstRect make_dstRect(double x, double y, double width, double height, bool initializeAsCentered) {
	if (initializeAsCentered) {
		return dstRect{ x - width / 2, y - height / 2, width, height };
	}
	else {
		return dstRect{ x, y, width, height };
	}
}



// # dRect #
dRect::dRect() {};

dRect::dRect(const Vector2d &point, const Vector2d &dimensions, const bool initializeAsCentered) {
	if (initializeAsCentered) {
		this->point1 = point - dimensions / 2.;
		this->point2 = point + dimensions / 2.;
	}
	else {
		this->point1 = point;
		this->point2 = point + dimensions;
	}
}

dRect::dRect(double pointX, double pointY, double sizeX, double sizeY, bool initializeAsCentered) :
	dRect(Vector2d(pointX, pointY), Vector2d(sizeX, sizeY), initializeAsCentered)
{}

// Move methods
void dRect::moveCornerTo(const Vector2d &newPoint1) {
	const auto size = this->getSize();
	this->point1 = newPoint1;
	this->point2 = newPoint1 + size;
}

void dRect::moveCornerTo(double newPoint1X, double newPoint1Y) {
	this->moveCornerTo(Vector2d(newPoint1X, newPoint1Y));
}

void dRect::moveCenterTo(const Vector2d &newCenter) {
	const auto size = this->getSize();
	this->point1 = newCenter - size / 2.;
	this->point2 = newCenter + size / 2.;
}

void dRect::moveCenterTo(double newCenterX, double newCenterY) {
	this->moveCenterTo(Vector2d(newCenterX, newCenterY));
}

void dRect::moveBy(const Vector2d &movement) {
	this->point1 += movement;
	this->point2 += movement;
}

void dRect::moveBy(double movementX, double movementY) {
	this->point1.x += movementX;
	this->point2.x += movementX;
	this->point1.y += movementY;
	this->point2.y += movementY;
}

void dRect::moveByX(double movementX) {
	this->point1.x += movementX;
	this->point2.x += movementX;
}

void dRect::moveByY(double movementY) {
	this->point1.y += movementY;
	this->point2.y += movementY;
}

void dRect::moveSideTo(const Side side, double newValue) {
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
}

void dRect::moveLeftTo(double newValue) {
	const auto sizeX = this->getSizeX();

	this->point1.x = newValue;
	this->point2.x = newValue + sizeX;
}

void dRect::moveRightTo(double newValue) {
	const auto sizeX = this->getSizeX();

	this->point1.x = newValue - sizeX;
	this->point2.x = newValue;
}

void dRect::moveTopTo(double newValue) {
	const auto sizeY = this->getSizeY();

	this->point1.y = newValue;
	this->point2.y = newValue + sizeY;
}

void dRect::moveBottomTo(double newValue) {
	const auto sizeY = this->getSizeY();

	this->point1.y = newValue - sizeY;
	this->point2.y = newValue;
}

// Scale methods
void dRect::scaleInPlaceBy(double scale) {
	const auto center = this->getCenter();
	const auto size = this->getSize();

	*this = dRect(center, size * scale, true);
}

void dRect::scaleInPlaceTo(const Vector2d &newDimensions) {
	const auto center = this->getCenter();

	*this = dRect(center, newDimensions, true);
}

// Corner getters
Vector2d dRect::getCornerTopLeft() const { return this->point1; }

Vector2d dRect::getCornerTopRight() const { return Vector2d(this->point2.x, this->point1.y); }

Vector2d dRect::getCornerBottomLeft() const { return Vector2d(this->point1.x, this->point2.y); }

Vector2d dRect::getCornerBottomRight() const { return this->point2; }

// Center getters
Vector2d dRect::getCenter() const { return (this->point1 + this->point2) / 2.; }
double dRect::getCenterX() const { return (this->point1.x + this->point2.x) / 2.; }
double dRect::getCenterY() const { return (this->point1.y + this->point2.y) / 2.; }

// Size getters
Vector2d dRect::getSize() const { return this->point2 - this->point1; }

double dRect::getSizeX() const { return this->point2.x - this->point1.x; }

double dRect::getSizeY() const { return this->point2.y - this->point1.y; }

// Side getters
double dRect::getSide(const Side side) const {
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

double dRect::getLeft() const {	return this->point1.x; }

double dRect::getRight() const { return this->point2.x; }

double dRect::getTop() const { return this->point1.y; }

double dRect::getBottom() const { return this->point2.y; }

// Side middlepoint getters
Vector2d dRect::getSideMiddlepoint(Side side) const {
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

Vector2d dRect::getLeftMiddlepoint() const {
	return Vector2d(this->point1.x, (this->point1.y + this->point2.y) / 2.);
}
Vector2d dRect::getRightMiddlepoint() const {
	return Vector2d(this->point2.x, (this->point1.y + this->point2.y) / 2.);
}
Vector2d dRect::getTopMiddlepoint() const {
	return Vector2d((this->point1.x + this->point2.x) / 2., this->point1.y);
}
Vector2d dRect::getBottomMiddlepoint() const {
	return Vector2d((this->point1.x + this->point2.x) / 2., this->point2.y);
}

// Intersection methods
dRect::CollisionInfo dRect::collideWithRect(const dRect &other) const {
	const double overlapX = std::max(
		0.,
		std::min(this->point2.x, other.point2.x) - std::max(this->point1.x, other.point1.x)
	);
	const double overlapY = std::max(
		0.,
		std::min(this->point2.y, other.point2.y) - std::max(this->point1.y, other.point1.y)
	);

	return dRect::CollisionInfo{ overlapX, overlapY, overlapX * overlapY };
}

bool dRect::overlapsWithRect(const dRect &other) const {
	return
		!(other.point1.x >= this->point2.x ||
		this->point1.x >= other.point2.x ||
		this->point1.y >= other.point2.y ||
		other.point1.y >= this->point2.y); // true if intersection is impossible => invert it to get the result
}

bool dRect::containsPoint(const Vector2d &point) const {
	return
		(this->getLeft() < point.x) &&
		(this->getRight() > point.x) &&
		(this->getTop() < point.y) &&
		(this->getBottom() > point.y);
}

// Conversions
SDL_Rect dRect::to_SDL_Rect() const {	
	return SDL_Rect{
		static_cast<int>(this->point1.x),
		static_cast<int>(this->point1.y),
		static_cast<int>(this->getSizeX()),
		static_cast<int>(this->getSizeY())
	};
}

srcRect dRect::to_srcRect() const {
	return this->to_SDL_Rect();
}

dstRect dRect::to_dstRect() const {
	return dstRect{ 
		this->point1.x, 
		this->point1.y,
		this->getSizeX(), 
		this->getSizeY() 
	};
}