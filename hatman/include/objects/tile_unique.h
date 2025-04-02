#pragma once

/* Contains all unique types of tiles (classes derived from 'Tile' class) */

#include "objects/tile_base.h" // 'Tile' base class
#include "utility/collection.hpp" // 'Collection<>::handle' class
#include "graphics/graphics.h" // 'Text' class (text collection handles)



namespace tiles {

	std::unique_ptr<Tile> make_tile(const Tileset &tileset, int id, const Vector2 &position);
		// creates a tile of a correct class based on data from tileset and returns ownership



	// # SaveOrb #
	// - Saves the game when interacted with
	class SaveOrb : public Tile {
	public:
		SaveOrb() = delete;

		SaveOrb(const Tileset &tileset, int id, const Vector2 &position);

		~SaveOrb(); // don't forget to erase text pop-up if level unloads

		bool checkActivation() const override;
		bool checkTrigger() const override; // checks if player is in range, and holds a 'USE' button
		void activate() override;
		void deactivate() override;
		void trigger() override;

	private:
		Collection<Text>::handle popup_handle; // handle to popup created upon tile activation
		Sound activation_sound;
	};



	// # Portal #
	// - Displays portal hint when player is near
	// - (actual portal mechanic is done through a script)
	class Portal : public Tile {
	public:
		Portal() = delete;
		Portal(const Tile &other);
		Portal(Tile &&other);

		Portal(const Tileset &tileset, int id, const Vector2 &position);

		~Portal(); // don't forget to erase text pop-up if level unloads

		bool checkActivation() const override;
		bool checkTrigger() const override; // checks if player is in range, and holds a 'USE' button
		void activate() override;
		void deactivate() override;
		void trigger() override;

	private:
		Collection<Text>::handle popup_handle; // handle to popup created upon tile activation
	};
}
