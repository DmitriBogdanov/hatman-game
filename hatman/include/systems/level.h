#pragma once

#include <unordered_map> // entities sorted by type are stored in a map
#include <unordered_set> // used to create access groups for entities
#include "thirdparty/nlohmann.hpp" // parsing from JSON, 'nlohmann::json' type

#include "utility/geometry.h" // geometry types
#include "objects/tile_base.h" // 'Tile' base class
#include "entity/type_m.h" // 'Entity' base class, 'Creature' class
#include "objects/script.h"
///#include "script_base.h" // 'Script' base class
#include "entity/player.h" // 'Player' base class
#include "utility/collection.hpp" // 'Collection' class
#include "systems/timer.h" // 'Milliseconds' type
#include "systems/flags.h"



// # Level #
// - Holds tiles, entities and scripts present on a map
// - Holds stores copies of tilesets that are used in given level
// - Holds level background
// - Handles updating and drawing of all aforementioned objects
class Level {
public:
	Level() {};

	Level(const std::string &name);
	Level(const std::string &name, std::unique_ptr<ntt::Entity> &&player);
		// also inits player upon construction

	void update(Milliseconds elapsedTime);
	void draw();

	Collection<Script> scripts;

	// Getters
	const Vector2& getSize() const;
	int getSizeX() const;
	int getSizeY() const;
	const std::string& getName() const;

	Tile* getTile(const Vector2 &index); // assumes index isn't out-of-bounds, otherwise you explode
	Tile* getTile(int indexX, int indexY);

	// Entities
	// Container that actually owns all entites
	std::vector<std::unique_ptr<ntt::Entity>> entities;

	// Containers that allow accessing entities in a 'sorted by properties/type' fashion
	std::unordered_set<ntt::Entity*> entities_solid;
	std::unordered_set<ntt::Entity*> entities_killable; // 'killable' == 'has .solid' + 'has .health'

	std::unordered_map<ntt::TypeId, std::unordered_set<ntt::Entity*>> entities_type; // dynamic_cast if access to type-specific stuff is needed
	
	ntt::player::Player* player;

	void spawn(std::unique_ptr<ntt::Entity> &&entity); // adds entity to spawn_queue

	std::unique_ptr<ntt::Entity> _extractPlayer(); // !!! after calling, level object is no longer valid !!!
	
private:
	std::vector<std::unique_ptr<ntt::Entity>> _spawn_queue;
	void _insertFromSpawnQueue();
	void _insertNewEntity(std::unique_ptr<ntt::Entity> &&entity);

	void _eraseMarkedEntities();
	std::unordered_map<ntt::Entity*, Flag> _on_death_emits;
		// flags that are emited when corresponding entity gets erased

	// Tiles
	std::vector<std::unique_ptr<Tile>> tiles_backlayer;
	std::vector<std::unique_ptr<Tile>> tiles; // actually handles logic
	std::vector<std::unique_ptr<Tile>> tiles_midlayer;
	std::vector<std::unique_ptr<Tile>> tiles_frontlayer;
		// - all layer types except 'tiles' are purely decorative and have physics/logic turned off
		// - rendering order is as follows: [backlayer]->[midlayer]->[layer]->[entities]->[frontlayer]
	
	size_t _getTile1DIndex(const Vector2 &index) const;
	size_t _getTile1DIndex(int indexX, int indexY) const;

	// Parsing
	void parseFromJSON(const std::string &filePath);

	// Tile parsing
	void parse_tilelayer(const nlohmann::json &tilelayer_node); // does all tilelayer parsing

	// Entity parsing
	void parse_objectgroup(const nlohmann::json &objectgroup_node); // redirects to parse_entity() or parse_script()
	void parse_objectgroup_entity(const nlohmann::json &objectgroup_node);

	// Scripts parsing
	void parse_objectgroup_script_levelChange(const nlohmann::json &objectgroup_node);
	void parse_objectgroup_script_levelSwitch(const nlohmann::json &objectgroup_node);
	void parse_objectgroup_script_portal(const nlohmann::json &objectgroup_node);
	void parse_objectgroup_script_hint(const nlohmann::json &objectgroup_node);
	void parse_objectgroup_script_checkpoint(const nlohmann::json &objectgroup_node);
	/*void parse_objectgroup_script_PlayerInArea(const nlohmann::json &objectgroup_node);
	void parse_objectgroup_script_AND(const nlohmann::json &objectgroup_node);
	void parse_objectgroup_script_OR(const nlohmann::json &objectgroup_node);
	void parse_objectgroup_script_XOR(const nlohmann::json &objectgroup_node);
	void parse_objectgroup_script_NAND(const nlohmann::json &objectgroup_node);
	void parse_objectgroup_script_NOR(const nlohmann::json &objectgroup_node);
	void parse_objectgroup_script_XNOR(const nlohmann::json &objectgroup_node);*/


	void add_Tile(const Tileset &tileset, int id, const Vector2 position, const std::string &layerPrefix);
		// adds tile to the level with respect to its interactions and etc
		// backlayer tiles are only drawn, other logic is ignored

	ntt::Entity* add_Entity(const std::string &type, const std::string &name, Vector2d position);
		// returns ptr to created entity
		// no need for 'add_Item()' as items can't exist outside of inventories
		// no need for 'add_Script()' as scripts can't be standardized under the same constructor parameters

	std::vector<Tileset> tilesets; // all tilesets of a current level, tilesets are ordered by firstgid

	sf::Sprite background_sprite;

	Vector2 map_size;

	std::string levelName;
};