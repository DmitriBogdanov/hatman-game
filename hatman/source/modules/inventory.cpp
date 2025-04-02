#include "modules/inventory.h"



// # Inventory #
void Inventory::addItem(const Item &item, int quantity) {
	if (Stack* const existingStack = this->getStack(item)) { // add to existing stack
		existingStack->quantity() += quantity;
	}
	else { // create new stack
		this->stacks.push_back(Stack(item, quantity));
	}
}

void Inventory::removeItem(const Item &item, int quantity) {
	if (Stack* const existingStack = this->getStack(item)) { // remove from existing stack

		if (existingStack->quantity() - quantity > 0) { // items left => just substrack
			existingStack->quantity() -= quantity;
		}
		else { // no items will be left => delete stack
			for (auto iter = this->stacks.begin(); iter != this->stacks.end(); ++iter) {
				if ((*iter).item() == item) { this->stacks.erase(iter); break; };
			}
		}
	} // given item is not present => do nothing
}


bool Inventory::pullFrom(Inventory &other, const Item &item, int quantity) {
	Stack* const sourceStack = other.getStack(item);

	if (sourceStack && (sourceStack->quantity() >= quantity)) { // only pull if possible
		other.removeItem(item, quantity);
		this->addItem(item, quantity);

		return true;
	}

	return false;
}

bool Inventory::pushTo(Inventory &other, const Item &item, int quantity) {
	Stack* const sourceStack = this->getStack(item);

	if (sourceStack && (sourceStack->quantity() >= quantity)) { // only push if possible
		this->removeItem(item, quantity);
		other.addItem(item, quantity);

		return true;
	}

	return false;
}


Stack* Inventory::getStack(const Item &item) {
	for (auto &stack : this->stacks) {
		if (stack.item() == item) { return &stack; }
	}

	return nullptr;
}

unsigned int Inventory::count(const std::string &name) {
	for (auto &stack : this->stacks)
		if (stack.item().getName() == name)
			return stack.quantity();

	return 0; // no items were found
}