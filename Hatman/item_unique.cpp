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
	{"eldritch_battery", make_derived<items::EldritchBattery>},
	{"power_shard", make_derived<items::PowerShard>},
	{"spider_signet", make_derived<items::SpiderSignet>},
	{"bone_mask", make_derived<items::BoneMask>},
	{"magic_negator", make_derived<items::MagicNegator>},
	{"twin_souls", make_derived<items::TwinSouls>},
	{"watching_eye", make_derived<items::WatchingEye>}
	/// new items go there
};

std::unique_ptr<Item> items::make_item(const std::string &name) {
	return ITEM_MAKERS.at(name)();
}


/* ### ITEMS ### */

// # EldritchBattery #
items::EldritchBattery::EldritchBattery() :
	Item(
		"eldritch_battery",
		"Eldritch Battery",
		"The most merciful thing in the world is the inability of human mind to comprehend the contents of that thing.",
		"Health regeneration is increased by 10 percent (additive)"
	)
{}

// # PowerShard #
items::PowerShard::PowerShard() :
	Item(
		"power_shard",
		"Power Shard",
		"The might of arcane, trapped in an intricate lattice.",
		"Increases damage of attacks by 10 percent (additive)"
	)
{}

// # SpiderSignet #
items::SpiderSignet::SpiderSignet() :
	Item(
		"spider_signet",
		"Spider Signet",
		"Twisting and skittering, yet static the when observed.",
		"Increases charged jump impulse by 8 percent (additive)"
	)
{}

// # BoneMask #
items::BoneMask::BoneMask() :
	Item(
		"bone_mask",
		"Bone Mask",
		"No mind to think. No will to break. No voice to cry when suffering.",
		"Reduces incoming physical damage by 20 percent (multiplicative)"
	)
{}

// # MagicNegator #
items::MagicNegator::MagicNegator() :
	Item(
		"magic_negator",
		"Magic Negator",
		"An unholy contraption beats like a heart.",
		"Reduces incoming magic damage by 20 percent (multiplicative)"
	)
{}

// # TwinSouls #
items::TwinSouls::TwinSouls() :
	Item(
		"twin_souls",
		"Twin Souls",
		"Independent, yet inseparable - two halves, chained together in a face of chaos.",
		"Reduces incoming chaos damage by 20 percent (multiplicative)"
	)
{}

// # WatchingEye #
items::WatchingEye::WatchingEye() :
	Item(
		"watching_eye",
		"Watching Eye",
		"Concealed within this shell, the lord of hunger sees all. His gaze pierces cloud, shadow, earth, and flesh.",
		"Increases number of charges by 1"
	)
{}