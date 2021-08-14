#pragma once

#include <string> // related type
#include <SDL.h> // 'SDL_Texture' type

#include "geometry_utils.h" // geometry types



// # Item #
// - ABSTRACT
// - Base class for all unique items
// - On 'use()' call performs some action that differs between derived classes
class Item {
public:
	Item() = default;
	virtual ~Item() = default;

	bool operator==(const Item &other);
	bool operator!=(const Item &other);

	void drawAt(const Vector2d &screenPos) const; // takes position of top-left corner

	virtual void use(); // does nothing by default

	const std::string& getName() const;
	const std::string& getLabel() const;

protected:
	SDL_Texture* texture;

	std::string name; // technical name
	std::string label; // displayble name

	Item(const std::string &name, const std::string &label);
		// loads texture based on name (used by derived classes)
};



// # Stack #
// - A stack of items
// - Unlimited size
class Stack {
public:
	Stack() = delete;

	Stack(const Stack &other); // copy
	Stack& operator=(const Stack &other); // copy (for .push_back())

	Stack(const Item& item, int quantity = 1);

	Item& item();
	const Item& item() const;

	int& quantity();
	const int& quantity() const;

private:
	std::unique_ptr<Item> stacked_item;
	int stacked_quantity;
};