#include "item_unique.h"

#include <functional> // functional types (derived object creation)
#include <unordered_map> // related type (derived object creation)



/* ### CONTROLLERS ### */

typedef std::function<std::unique_ptr<Item>()> make_derived_ptr;

// std::make_unique() wrapper
template<class UniqueItem>
std::unique_ptr<UniqueItem> make_derived() {
	return std::make_unique<UniqueItem>();
}

// !!! NAMES !!!
const std::unordered_map<std::string, make_derived_ptr> ITEM_MAKERS = {
	{"brass_relic", make_derived<items::BrassRelic>},
	{"paper", make_derived<items::Paper>}
	/// new items go there
};

std::unique_ptr<Item> items::make_item(const std::string &name) {
	return ITEM_MAKERS.at(name)();
}


/* ### ITEMS ### */

// # BrassRelic #
items::BrassRelic::BrassRelic() :
	Item("brass_relic", "Brass relic")
{}



// # Paper #
items::Paper::Paper() :
	Item("paper", "Paper")
{}