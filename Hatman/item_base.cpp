#include "item_base.h"

#include "graphics.h" // access to texture loading
#include "globalconsts.hpp"



// # Item #
Item::Item(const std::string &name, const std::string &label) :
	name(name),
	label(label)
{
	this->texture = Graphics::ACCESS->getTexture_Item(this->name + ".png");
}

void Item::drawAt(const Vector2d &screenPos) const {
	dstRect destRect = { screenPos.x, screenPos.y, natural::ITEM_SIZE, natural::ITEM_SIZE };
	Graphics::ACCESS->gui->textureToGUI(this->texture, NULL, &destRect);
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