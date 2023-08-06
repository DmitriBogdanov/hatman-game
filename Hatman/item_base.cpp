#include "item_base.h"

#include "graphics.h" // access to texture loading
#include "globalconsts.hpp"



// # Item #
Item::Item(const std::string &name, const std::string &label, const std::string &lore, const std::string &effect) :
	name(name),
	label(label),
	description_lore(lore),
	description_effect(effect)
{
	this->sprite.setTexture(Graphics::ACCESS->getTexture_Item(this->name + ".png"));
	this->sprite.setScale(2, 2);
}

void Item::drawAt(const Vector2d &screenPos) {
	this->sprite.setPosition(
		static_cast<float>(screenPos.x),
		static_cast<float>(screenPos.y)
	);

	// no need to scale

	Graphics::ACCESS->gui->draw_sprite(sprite);
}

bool Item::operator==(const Item &other) {
	return (this->name == other.name);
}
bool Item::operator!=(const Item &other) {
	return (this->name != other.name);
}

void Item::use() {} // does nothing by default

const std::string& Item::getName() const { return this->name; }
const std::string& Item::getLabel() const { return this->label; }
const std::string& Item::get_description_lore() const { return this->description_lore; }
const std::string& Item::get_description_effect() const { return this->description_effect; }



// # Stack #
Stack::Stack(const Stack &other) :
	Stack(other.item(), other.quantity())
{}
Stack& Stack::operator=(const Stack &other) {
	this->stacked_item = std::make_unique<Item>(other.item());
	this->stacked_quantity = other.quantity();

	return *this;
}

Stack::Stack(const Item& item, int quantity) :
	stacked_item(std::make_unique<Item>(item)),
	stacked_quantity(quantity)
{}

Item& Stack::item() {
	return *this->stacked_item;
}
const Item& Stack::item() const {
	return *this->stacked_item;
}

int& Stack::quantity() {
	return this->stacked_quantity;
}
const int& Stack::quantity() const {
	return this->stacked_quantity;
}