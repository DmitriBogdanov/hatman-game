#pragma once

/* Contains all unique types of items (classes derived from 'Item' class) */

#include "item_base.h" // 'Item' base class



namespace items {

	std::unique_ptr<Item> make_item(const std::string &name);
		// creates item of a correct class based on name and returns ownership


	// # EldritchBattery #
	struct EldritchBattery : public Item {
		EldritchBattery();
	};

	// # PowerShard #
	struct PowerShard : public Item {
		PowerShard();
	};

	// # SpiderSignet #
	struct SpiderSignet : public Item {
		SpiderSignet();
	};

	// # BoneMask #
	struct BoneMask : public Item {
		BoneMask();
	};

	// # MagicNegator #
	struct MagicNegator : public Item {
		MagicNegator();
	};

	// # TwinSouls #
	struct TwinSouls : public Item {
		TwinSouls();
	};

	// # WatchingEye #
	struct WatchingEye : public Item {
		WatchingEye();
	};
}