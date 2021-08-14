#pragma once

/* Contains all unique types of items (classes derived from 'Item' class) */

#include "item_base.h" // 'Item' base class



namespace items {

	std::unique_ptr<Item> make_item(const std::string &name);
		// creates item of a correct class based on name and returns ownership



	// #BrassRelic #
	struct BrassRelic : public Item {
		BrassRelic();
	};



	// # Paper #
	struct Paper : public Item {
		Paper();
	};
}