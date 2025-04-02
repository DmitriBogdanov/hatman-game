#pragma once

/* Contains module: 'Inventory' */

#include <vector> // related type

#include "item_base.h" // 'Item' class



// # Inventory #
// - MODULE
// - Holds stacks of items
// - Can exchange items with other inventories
class Inventory {
public:
	//void addItem(const Item &item);
	void addItem(const Item &item, int quantity = 1);

	//void removeItem(const Item &item);
	void removeItem(const Item &item, int quantity = 1);

	bool pullFrom(Inventory &other, const Item &item, int quantity = 1);
	// returns whether transfer was successfull
	bool pushTo(Inventory &other, const Item &item, int quantity = 1);
	// returns whether transfer was successfull

	Stack* getStack(const Item &item); // returns nullptr if item is not found
	
	unsigned int count(const std::string &name); // returns nmeber of items with given name

	std::vector<Stack> stacks;
};