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
	{"spider_signet", make_derived<items::SpiderSignet>},
	{"power_shard", make_derived<items::PowerShard>},
	{"eldritch_battery", make_derived<items::EldritchBattery>},
	{"watching_eye", make_derived<items::WatchingEye>},
	{"brass_relic", make_derived<items::BrassRelic>},
	{"paper", make_derived<items::Paper>}
	/// new items go there
};

std::unique_ptr<Item> items::make_item(const std::string &name) {
	return ITEM_MAKERS.at(name)();
}


/* ### ITEMS ### */

// # SpiderSignet #
items::SpiderSignet::SpiderSignet() :
	Item("spider_signet", "Spider signet")
{}

// # PowerShard #
items::PowerShard::PowerShard() :
	Item("power_shard", "Power shard")
{}

// # EldritchBattery #
items::EldritchBattery::EldritchBattery() :
	Item("eldritch_battery", "Eldritch battery")
{}

// # WatchingEye #
items::WatchingEye::WatchingEye() :
	Item("watching_eye", "Watching eye")
{}

// # BrassRelic #
items::BrassRelic::BrassRelic() :
	Item("brass_relic", "Brass relic")
{}

// # Paper #
items::Paper::Paper() :
	Item("paper", "Paper")
{}