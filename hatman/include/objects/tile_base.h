#pragma once

/* Contains classes that deal with tilesets and individual tiles */

#include <SFML/Graphics.hpp>

#include <string> // related type
#include <unordered_map> // related type (tileset storage)
#include <memory> // 'unique_ptr' type

#include "thirdparty/nlohmann.hpp" // parsing from JSON
#include "utility/geometry.h" // geometry types
#include "modules/sprite.h" // 'Sprite' module



class Tileset; // forward-declaration



// # EntitySpawnData #
struct EntitySpawnData {
	std::string type;
	std::string name;

	Vector2d position_in_tile;
};


struct TileHitboxRect {
	dRect rect;
	bool is_platform;
};

// # TileHitbox #
// - Contains rectangles that make up a tile hitbox
// - Used to store tile info inside a tileset
struct TileHitbox {
	TileHitbox() {};

	TileHitbox(const std::vector<TileHitboxRect> &rects);
	TileHitbox(std::vector<TileHitboxRect> &&rects); // move semantics

	std::vector<TileHitboxRect> rectangles;
};



// # TileInteraction #
// - Some sort of unique action that happens upon meeting certaint conditions, specified in a derived class
// - Used to store tile info inside a tileset
struct TileInteraction {
	TileInteraction() {};

	TileInteraction(const std::string &interactive_type, dRect &actionbox);

	void setInput(const std::string &emit);

	void setOutput(const std::string &emit, int lifetime = 0);

	std::string interactive_type;
	dRect actionbox;

	// Emit input
	std::string emit_input;

	// Emit output
	std::string emit_output;
	Milliseconds emit_duration;
};



// # Tile #
// - NOT abstract (unlike 'Entity' and 'Item')
// - Holds necessary info about a single tile
// - Created by pulling data from tilesets by tile ID
class Tile {
public:
	Tile() = delete;
	Tile(const Tile &other);
	Tile(Tile &&other);

	Tile(const Tileset &tileset, int id, const Vector2 position);

	virtual ~Tile() = default;

	void update(Milliseconds elapsedTime); // updates animation
	void draw() const; // draws to the screen

	virtual bool checkActivation() const { return false; }  // checks if tile should be activated
	virtual bool checkTrigger() const { return false; } // checks if trigger condition is true (tile must be ativated)
	virtual void activate() {} // toggles active state
	virtual void deactivate() {} // executed upon leaving active state
	virtual void trigger() {} // triggers some action

	Vector2d position; // position on the level

	std::unique_ptr<TileHitbox> hitbox; // contains a vector of hitbox Rectangle's
	std::unique_ptr<Sprite> sprite; // animated or static
	std::unique_ptr<TileInteraction> interaction; // Unique logic for derived classes

protected:
	bool toggle_active;

	sf::Sprite tilesheet_sprite;
};



// # Tileset #
// - Holds all data that defines a tileset
// - Parses tileset data from tileset .JSON
// - Provides tile data by ID
class Tileset {
public:
	Tileset() {};
	Tileset(const std::string &fileName);

	void parseFromJSON(const std::string &filePath);

	TileHitboxRect parse_as_hitboxrect(const nlohmann::json& object_node);
	TileInteraction parse_as_interaction(const nlohmann::json& object_node);
	EntitySpawnData parse_as_entity(const nlohmann::json& object_node);

	// Getting data from tileset
	srcRect get_tile_source_rect(int tileId) const;

	bool has_tile_hitbox(int tileId) const;
	bool has_tile_animation(int tileId) const;
	bool has_tile_interaction(int tileId) const;
	bool has_entity_spawn_data(int tileId) const;

	TileHitbox get_tile_hitbox(int tileId) const; // returns tile hitbox on the map
	Animation get_tile_animation(int tileId) const;
	TileInteraction get_tile_interaction(int tileId) const; // should only be used when hit/actionbox is present
	const EntitySpawnData& get_entity_spawn_data(int tileId) const;

	// Tileset getters
	std::string tileset_get_filename() const;
	sf::Texture* tileset_get_texture() const;

	int first_gid;

private:
	std::string filename;

	sf::Texture* texture;
	Vector2 size;

	std::unordered_map<int, TileHitbox> tileHitboxes;
	std::unordered_map<int, Animation> tileAnimations;
	std::unordered_map<int, TileInteraction> tileInteractions;
	std::unordered_map<int, EntitySpawnData> entity_objects;
};



// # TilesetStorage #
// - Can be accessed wherever #include'ed through static 'ACCESS' field
// - Used to load, manage and access loaded tilesets
// - Only one instance at a time should exits logically, but more can be created
class TilesetStorage {
public:
	TilesetStorage();

	static const TilesetStorage* READ;
	static TilesetStorage* ACCESS;

	const Tileset& getTileset(const std::string &fileName); // loads or returns a loaded tileset by name

private:
	std::unordered_map<std::string, Tileset> loadedTilesets;
};