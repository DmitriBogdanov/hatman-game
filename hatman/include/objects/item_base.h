#pragma once

#include <string> // related type
#include <SFML/Graphics.hpp>

#include "utility/geometry.h" // geometry types



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

	void drawAt(const Vector2d &screenPos); // takes position of top-left corner

	virtual void use(); // does nothing by default

	const std::string& getName() const;

	const std::string& getLabel() const;
	const std::string& get_description_lore() const;
	const std::string& get_description_effect() const;

protected:
	sf::Sprite sprite;

	std::string name; // technical name

	std::string label; // displayble name
	std::string description_lore;
	std::string description_effect;

	Item(const std::string &name, const std::string &label, const std::string &lore, const std::string &effect);
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