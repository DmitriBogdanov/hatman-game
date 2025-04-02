#include "level.h"

#include <fstream> // parsing from JSON (opening a file)
#include <type_traits>

#include "UTL/log.hpp"

#include "entity_base.h"
#include "graphics.h" // access to rendering (background)
#include "saver.h" // access to savefile info (level version)
#include "tags.h" // tag utility
#include "tile_unique.h" // creation of unique tiles
#include "entity_unique_m.h" // creation of unique entities
#include "globalconsts.hpp" // performnce-related consts
#include "game.h" // to play music


// # Level #
Level::Level(const std::string &name) :
	player(nullptr),
	levelName(name)
{
	this->parseFromJSON("content/levels/" + name + ".json");
}

Level::Level(const std::string &name, std::unique_ptr<ntt::Entity> &&player) :
	Level(name)
{
    UTL_LOG_INFO("Loaded level {", name, "} with player ", player.get());
    
    this->_insertNewEntity(std::move(player));
    
	//this->spawn(std::move(player));
}

void Level::update(Milliseconds elapsedTime) {
	// Spawn new entities
	if (this->_spawn_queue.size()) {
		this->_insertFromSpawnQueue();
		this->_spawn_queue.clear();
	}

	const auto cameraPos = this->player->cameraTrap_getPosition();

	const Vector2 centerIndex = helpers::divide32(cameraPos);

	const int leftBound = std::max(centerIndex.x - performance::TILE_FREEZE_RANGE_X, 0);
	const int rightBound = std::min(centerIndex.x + performance::TILE_FREEZE_RANGE_X, this->map_size.x - 1);
	const int upperBound = std::max(centerIndex.y - performance::TILE_FREEZE_RANGE_Y, 0);
	const int lowerBound = std::min(centerIndex.y + performance::TILE_FREEZE_RANGE_Y, this->map_size.y - 1);

	// Update tiles
	for (int X = leftBound; X <= rightBound; ++X)
		for (int Y = upperBound; Y <= lowerBound; ++Y) {
			const auto tileIndex = this->_getTile1DIndex(X, Y);

			const auto tile = this->tiles[tileIndex].get();

			if (tile) tile->update(elapsedTime);
		}

	// Update entities
	for (auto &entity : this->entities)
		if (std::abs(cameraPos.x - entity->position.x) < performance::ENTITY_FREEZE_RANGE_X &&
			std::abs(cameraPos.y - entity->position.y) < performance::ENTITY_FREEZE_RANGE_Y)
			entity->update(elapsedTime);

	// Erase 'dead' entities
	this->_eraseMarkedEntities();

	// Update scripts
	for (auto &script : this->scripts) { script.update(elapsedTime); }
}

void Level::draw() {
	// Draw backround
	Graphics::ACCESS->gui->draw_sprite(this->background_sprite);

	const auto cameraPos = this->player->cameraTrap_getPosition();

	const Vector2 centerIndex = helpers::divide32(cameraPos);

	const int leftBound = std::max(centerIndex.x - performance::TILE_DRAW_RANGE_X, 0);
	const int rightBound = std::min(centerIndex.x + performance::TILE_DRAW_RANGE_X, this->map_size.x - 1);
	const int upperBound = std::max(centerIndex.y - performance::TILE_DRAW_RANGE_Y, 0);
	const int lowerBound = std::min(centerIndex.y + performance::TILE_DRAW_RANGE_Y, this->map_size.y - 1);

	// Draw [backlayer]->[layer]->[midlayer]
	for (int X = leftBound; X <= rightBound; ++X)
		for (int Y = upperBound; Y <= lowerBound; ++Y) {
			const auto tileIndex = this->_getTile1DIndex(X, Y);

			const auto backtile = this->tiles_backlayer[tileIndex].get();
			const auto tile = this->tiles[tileIndex].get();

			if (backtile) backtile->draw();
			if (tile) tile->draw();
		}


	// Draw entities
	for (const auto &entity : this->entities)
		if (cameraPos.x - entity->position.x < performance::ENTITY_DRAW_RANGE_X &&
			cameraPos.y - entity->position.y < performance::ENTITY_DRAW_RANGE_Y)
			entity->draw();

	// Draw [frontlayer]
	for (int X = leftBound; X <= rightBound; ++X)
		for (int Y = upperBound; Y <= lowerBound; ++Y) {
			const auto tileIndex = this->_getTile1DIndex(X, Y);

			const auto fronttile = this->tiles_frontlayer[tileIndex].get();

			if (fronttile) fronttile->draw();
		}
}

// Getters
const Vector2& Level::getSize() const { return this->map_size; }

int Level::getSizeX() const { return this->map_size.x; }

int Level::getSizeY() const { return this->map_size.y; }

const std::string& Level::getName() const { return this->levelName; }

Tile* Level::getTile(const Vector2 &index) {
	return this->tiles[this->_getTile1DIndex(index.x, index.y)].get();
}

Tile* Level::getTile(int indexX, int indexY) {
	return this->tiles[this->_getTile1DIndex(indexX, indexY)].get();
}

// Entities
void Level::spawn(std::unique_ptr<ntt::Entity> &&entity) {
	this->_spawn_queue.push_back(std::move(entity));
    UTL_LOG_WARN("spawn queue size -> ", _spawn_queue.size());
}

std::unique_ptr<ntt::Entity> Level::_extractPlayer() {
	std::unique_ptr<ntt::Entity> extracted;

	for (auto &entity : this->entities)
		if (entity.get() == this->player)
			extracted = std::move(entity);

	return extracted;
}

void Level::_insertFromSpawnQueue() {
	for (auto &&entity : this->_spawn_queue) {
		this->_insertNewEntity(std::move(entity));
	}
}

void Level::_insertNewEntity(std::unique_ptr<ntt::Entity> &&entity) {
	const auto ptr = entity.get();
	this->entities.push_back(std::move(entity));

	if (ptr->solid) this->entities_solid.insert(ptr);
	if (ptr->solid && ptr->health) this->entities_killable.insert(ptr);

	using Id = ntt::TypeId;

	// Add ptr to correct types (manually)
	switch (ptr->type_id()) {
	case Id::CREATURE:
		this->entities_type[Id::CREATURE].insert(ptr);
		break;
	case Id::ENEMY:
		this->entities_type[Id::CREATURE].insert(ptr);
		this->entities_type[Id::ENEMY].insert(ptr);
		break;
	case Id::PLAYER:
		this->entities_type[Id::CREATURE].insert(ptr);
		this->player = dynamic_cast<ntt::player::Player*>(ptr);
		break;
	case Id::ITEM_ENTITY:
		this->entities_type[Id::ITEM_ENTITY].insert(ptr);
		break;
	case Id::DESTRUCTIBLE:
		this->entities_type[Id::DESTRUCTIBLE].insert(ptr);
		break;
	case Id::PROJECTILE:
		this->entities_type[Id::PROJECTILE].insert(ptr);
		break;
	case Id::PARTICLE:
		this->entities_type[Id::PARTICLE].insert(ptr);
		break;
	default:
		break;
	}
}

template<class Container, typename Condition>
Container& swap_erase(Container &container, const Condition &condition) {
	auto iterToLast = container.end();

	for (auto iter = container.begin(); iter < iterToLast;)
		if (condition(*iter)) *iter = std::move(*(--iterToLast));
		else ++iter;

	container.resize(iterToLast - container.begin());

	return container;
}

void Level::_eraseMarkedEntities() {
	auto iterToLast = this->entities.end();
    
    // TODO: Hmmm, it's almost as if there should be an appropriate algo somewhere in 'std'...
    
	for (auto iter = this->entities.begin(); iter < iterToLast;)
		if (iter->get()->marked_for_erase()) {
			const auto ptrToErase = iter->get();

			// Move to the end
            --iterToLast;
            std::swap(*iter, *iterToLast);
			//*iter = std::move(*(--iterToLast));

			// Emit on-death flag (if present)
			auto iterToEmitedFlag = this->_on_death_emits.find(ptrToErase);

			if (iterToEmitedFlag != this->_on_death_emits.end()) {
				Flags::ACCESS->add((*iterToEmitedFlag).second);
				this->_on_death_emits.erase(iterToEmitedFlag);
			}

			// Clear dependencies
			this->entities_solid.erase(ptrToErase);
			this->entities_killable.erase(ptrToErase);
			for (auto& node : this->entities_type) node.second.erase(ptrToErase);
		}
		else {
			++iter;
		}

	this->entities.resize(iterToLast - this->entities.begin());
	
	/*const auto condition = [](ntt::Entity* ptr) { return ptr->marked_for_erase(); };
	const auto condition2 = [](const std::unique_ptr<ntt::Entity> &ptr) { return ptr->marked_for_erase(); };

	for (auto &node : this->entities_type) swap_erase(node.second, condition);
	swap_erase(this->entities_solid, condition);
	swap_erase(this->entities_killable, condition);
	swap_erase(this->entities, condition2);*/
}

// Tile
size_t Level::_getTile1DIndex(int indexX, int indexY) const {
	return indexX * this->map_size.y + indexY;
}

size_t Level::_getTile1DIndex(const Vector2 &index) const {
	return index.x * this->map_size.y + index.y;
}

void Level::add_Tile(const Tileset &tileset, int id, const Vector2 position, const std::string &layerPrefix) {
	auto newTile = tiles::make_tile(tileset, id, position * natural::TILE_SIZE);
	const auto newTileIndex = this->_getTile1DIndex(position);

	const static std::unordered_map<std::string, int> prefixToCase{
		{"backlayer", 0},
		{"layer", 1},
		{"midlayer", 2},
		{"frontlayer", 3}
	};

	switch (prefixToCase.at(layerPrefix)) {
	case 0:
		this->tiles_backlayer[newTileIndex] = std::move(newTile);
		break;
	case 1:
		this->tiles[newTileIndex] = std::move(newTile);
		break;
	case 2:
		this->tiles_midlayer[newTileIndex] = std::move(newTile);
		break;
	case 3:
		this->tiles_frontlayer[newTileIndex] = std::move(newTile);
		break;
	default:
		break;
	}
}
ntt::Entity* Level::add_Entity(const std::string &type, const std::string &name, Vector2d position) {
	auto entity = ntt::m::make_entity(type, name, position);
	const auto ptr = entity.get();

	this->_insertNewEntity(std::move(entity));

	return ptr;
}

// Parsing
void Level::parseFromJSON(const std::string &filePath) {
	// Load JSON doc
	std::ifstream ifStream(filePath);
	nlohmann::json JSON = nlohmann::json::parse(ifStream);

	// Parse map properties
	for (const auto &property_node : JSON["properties"]) {
		const std::string prefix = tags::getPrefix(property_node["name"].get<std::string>());

		if (prefix == "background") {
			this->background_sprite.setTexture(
				Graphics::ACCESS->getTexture_Background(property_node["value"].get<std::string>())
			);
		}
		if (prefix == "music") {
			Game::ACCESS->play_music(property_node["value"].get<std::string>());
		}
		/// new properties go there
	}

	// Parse tilesets
	const nlohmann::json &tilesets_array_node = JSON["tilesets"];
	for (const auto &tileset_node : tilesets_array_node) {
		// Extract tileset name
		std::string fileName = tileset_node["source"].get<std::string>();
		fileName = fileName.substr(fileName.rfind("/") + 1); // cut before '/'
		fileName = fileName.substr(fileName.rfind("\\") + 1); // cut before '\'

		// Create tileset object and set firstgid
		Tileset tileset = TilesetStorage::ACCESS->getTileset(fileName);
		tileset.first_gid = tileset_node["firstgid"].get<int>(); // firstgid is map-dependant (that's also why we copy tilesets)

		this->tilesets.push_back(std::move(tileset)); // save tileset
	}

	// Parse map properties (size and etc)
	this->map_size.x = JSON["width"].get<int>();
	this->map_size.y = JSON["height"].get<int>();

	this->tiles_backlayer.resize(this->map_size.x * this->map_size.y);
	this->tiles.resize(this->map_size.x * this->map_size.y);
	this->tiles_midlayer.resize(this->map_size.x * this->map_size.y);
	this->tiles_frontlayer.resize(this->map_size.x * this->map_size.y);

	// Parse tile layers
	const nlohmann::json &layers_array_node = JSON["layers"];
	for (const auto &layer_node : layers_array_node) {
		const std::string layer_type = layer_node["type"].get<std::string>(); // can be "tilelayer" or "objectgroup"

		// Parse data depending on layer_type
		if (layer_type == "tilelayer") {
			this->parse_tilelayer(layer_node);
		}
		else if (layer_type == "objectgroup") {
			this->parse_objectgroup(layer_node);
		}
	}

}

void Level::parse_tilelayer(const nlohmann::json &tilelayer_node) {
	// Determine layer type
	const auto layerPrefix = tags::getPrefix(tilelayer_node["name"].get<std::string>());

	int tileCount = 0; // used to determine tile position

	const nlohmann::json &data_array_node = tilelayer_node["data"];
	for (const auto &data_node : data_array_node) {
		const int gid = data_node.get<int>();

		if (gid) { // if tile is present
			// Calculate tile position
			const Vector2 tilePosition(
				(tileCount % this->map_size.x),
				(tileCount / this->map_size.x)
			);

			// Determine which tileset tile belongs to (based on gid)
			const Tileset* correspondingTileset = &this->tilesets.front();
			for (const auto &tileset : this->tilesets)
				if (gid >= tileset.first_gid) correspondingTileset = &tileset;

			const int tileId = gid - correspondingTileset->first_gid;

			// Construct tile and add it to the level
			this->add_Tile(*correspondingTileset, tileId, tilePosition, layerPrefix);
		}

		++tileCount;
	}
}

void Level::parse_objectgroup(const nlohmann::json &objectgroup_node) {
	// get layer prefix and suffix
	const std::string layer_prefix = tags::getPrefix(objectgroup_node["name"].get<std::string>());
	const std::string layer_suffix = tags::getSuffix(objectgroup_node["name"].get<std::string>());

	if (layer_prefix == "entity") {
		this->parse_objectgroup_entity(objectgroup_node);
	}
	else if (layer_prefix == "script") {
		if (layer_suffix == "level_change") {
			this->parse_objectgroup_script_levelChange(objectgroup_node);
		}
		else if (layer_suffix == "level_switch") {
			this->parse_objectgroup_script_levelSwitch(objectgroup_node);
		}
		else if (layer_suffix == "portal") {
			this->parse_objectgroup_script_portal(objectgroup_node);
		}
		else if (layer_suffix == "hint") {
			this->parse_objectgroup_script_hint(objectgroup_node);
		}
		else if (layer_suffix == "checkpoint") {
			this->parse_objectgroup_script_checkpoint(objectgroup_node);
		}
		// new script types go there
	}
}

// Entity types parsing
void Level::parse_objectgroup_entity(const nlohmann::json &objectgroup_node) {
	const nlohmann::json &objects_array_node = objectgroup_node["objects"];
	for (const auto& object_node : objects_array_node) {
		// Get custom properties
		Flag requires_flag = "";
		Flag emits_flag = "";

		const auto properties_node_iter = object_node.find("properties");

		if (properties_node_iter != object_node.end()) {
			const nlohmann::json &properties_array = *properties_node_iter;

			for (const auto &property_node : properties_array) {
				const std::string name = property_node["name"];

				if (name == "requires_flag") {
					requires_flag = property_node["value"].get<std::string>();
				}
				else if (name == "emits_flag") {
					emits_flag = property_node["value"].get<std::string>();
				}
			}
		}

		// If 'reqired_flag' is not satisfied, further parsing is unnecessary
		if (!requires_flag.empty() && !Flags::READ->check(requires_flag)) continue;

		// Determine which tileset 'entity-tile' belongs to (based on gid)
		const auto gid = object_node["gid"].get<int>();
		
		const Tileset* correspondingTileset = &this->tilesets.front();

		for (const auto& tileset : this->tilesets)
			if (gid >= tileset.first_gid) correspondingTileset = &tileset;

		// Get spawn data
		const int id = gid - correspondingTileset->first_gid;
		const auto &enitySpawnData = correspondingTileset->get_entity_spawn_data(id);

		// Parse position
		const auto tilePosition = Vector2d(object_node["x"].get<double>(), object_node["y"].get<double>());

		// Create entity
		const auto ptr_to_entity = this->add_Entity(
			enitySpawnData.type,
			enitySpawnData.name,
			tilePosition + enitySpawnData.position_in_tile - Vector2d(0, natural::TILE_SIZE)
				// !!! for some bizarre reason Tiled uses BOTTOM-left corner coordinates for
				// tile objects so we have to move it up 1 tile to get normal coords
		);

		// Set flag emited on death (if present)
		if (!emits_flag.empty()) this->_on_death_emits[ptr_to_entity] = emits_flag;
	}
}

// Scripts parsing
void Level::parse_objectgroup_script_levelChange(const nlohmann::json &objectgroup_node) {
	const nlohmann::json &objects_array_node = objectgroup_node["objects"];
	for (const auto &object_node : objects_array_node) {
		// Parse hitbox
		const dRect hitbox = dRect(
			object_node["x"].get<int>(), object_node["y"].get<int>(),
			object_node["width"].get<int>(), object_node["height"].get<int>()
		);

		std::string goes_to_level;
		Vector2 goes_to_pos;

		// Parse custom properties
		for (const auto &property_node : object_node["properties"]) {
			const std::string prefix = tags::getPrefix(property_node["name"].get<std::string>());

			if (prefix == "goes_to_level") {
				goes_to_level = property_node["value"].get<std::string>();
			}
			else if (prefix == "goes_to_x") {
				goes_to_pos.x = property_node["value"].get<int>();
			}
			else if (prefix == "goes_to_y") {
				goes_to_pos.y = property_node["value"].get<int>();
			}
		}

		this->scripts.insert(std::make_unique<scripts::LevelChange>(hitbox, goes_to_level, goes_to_pos));
	}
}

void Level::parse_objectgroup_script_levelSwitch(const nlohmann::json &objectgroup_node) {
	const nlohmann::json &objects_array_node = objectgroup_node["objects"];
	for (const auto &object_node : objects_array_node) {
		// Parse hitbox
		const dRect hitbox = dRect(
			object_node["x"].get<int>(), object_node["y"].get<int>(),
			object_node["width"].get<int>(), object_node["height"].get<int>()
		);

		std::string goes_to_level;
		Vector2 goes_to_pos;

		// Parse custom properties
		for (const auto &property_node : object_node["properties"]) {
			const std::string prefix = tags::getPrefix(property_node["name"].get<std::string>());

			if (prefix == "goes_to_level") {
				goes_to_level = property_node["value"].get<std::string>();
			}
			else if (prefix == "goes_to_x") {
				goes_to_pos.x = property_node["value"].get<int>();
			}
			else if (prefix == "goes_to_y") {
				goes_to_pos.y = property_node["value"].get<int>();
			}
		}

		this->scripts.insert(std::make_unique<scripts::LevelSwitch>(hitbox, goes_to_level, goes_to_pos));
	}
}

void Level::parse_objectgroup_script_portal(const nlohmann::json &objectgroup_node) {
	const nlohmann::json &objects_array_node = objectgroup_node["objects"];
	for (const auto &object_node : objects_array_node) {
		// Parse hitbox
		const dRect hitbox = dRect(
			object_node["x"].get<int>(), object_node["y"].get<int>(),
			object_node["width"].get<int>(), object_node["height"].get<int>()
		);

		std::string goes_to_level;
		Vector2 goes_to_pos;

		// Parse custom properties
		for (const auto &property_node : object_node["properties"]) {
			const std::string prefix = tags::getPrefix(property_node["name"].get<std::string>());

			if (prefix == "goes_to_x") {
				goes_to_pos.x = property_node["value"].get<int>();
			}
			else if (prefix == "goes_to_y") {
				goes_to_pos.y = property_node["value"].get<int>();
			}
		}

		this->scripts.insert(std::make_unique<scripts::Portal>(hitbox, goes_to_pos));
	}
}

void Level::parse_objectgroup_script_hint(const nlohmann::json &objectgroup_node) {
	const nlohmann::json &objects_array_node = objectgroup_node["objects"];
	for (const auto &object_node : objects_array_node) {
		// Parse hitbox
		const dRect hitbox = dRect(
			object_node["x"].get<int>(), object_node["y"].get<int>(),
			object_node["width"].get<int>(), object_node["height"].get<int>()
		);

		Vector2d field_center;
		Vector2d field_size;

		std::string text;
		
		// Parse custom properties
		for (const auto &property_node : object_node["properties"]) {
			const std::string prefix = tags::getPrefix(property_node["name"].get<std::string>());

			if (prefix == "text") {
				text = property_node["value"].get<std::string>();
			}
			else if (prefix == "text_x") {
				field_center.x = property_node["value"].get<int>();
			}
			else if (prefix == "text_y") {
				field_center.y = property_node["value"].get<int>();
			}
			else if (prefix == "text_width") {
				field_size.x = property_node["value"].get<int>();
			}
			else if (prefix == "text_height") {
				field_size.y = property_node["value"].get<int>();
			}
		}

		const auto text_field = dRect(field_center, field_size, true);

		this->scripts.insert(std::make_unique<scripts::Hint>(hitbox, text_field, text));
	}
}

void Level::parse_objectgroup_script_checkpoint(const nlohmann::json &objectgroup_node) {
	const nlohmann::json &objects_array_node = objectgroup_node["objects"];
	for (const auto &object_node : objects_array_node) {
		// Parse hitbox
		const dRect hitbox = dRect(
			object_node["x"].get<int>(), object_node["y"].get<int>(),
			object_node["width"].get<int>(), object_node["height"].get<int>()
		);

		// Get custom properties
		Flag requires_flag = "";
		Flag emits_flag = "";

		const auto properties_node_iter = object_node.find("properties");

		if (properties_node_iter != object_node.end()) {
			const nlohmann::json &properties_array = *properties_node_iter;

			for (const auto &property_node : properties_array) {
				const std::string name = property_node["name"];

				if (name == "requires_flag") {
					requires_flag = property_node["value"].get<std::string>();
				}
				else if (name == "emits_flag") {
					emits_flag = property_node["value"].get<std::string>();
				}
			}
		}

		// If 'reqired_flag' is not satisfied, further parsing is unnecessary
		if (!requires_flag.empty() && !Flags::READ->check(requires_flag)) continue;

		this->scripts.insert(std::make_unique<scripts::Checkpoint>(hitbox, emits_flag));
	}
}
//void Level::parse_objectgroup_script_PlayerInArea(const nlohmann::json &objectgroup_node) {
//	const nlohmann::json &objects_array_node = objectgroup_node["objects"];
//	for (const auto &object_node : objects_array_node) {
//		// Parse hitbox
//		const dRect hitbox = dRect(
//			object_node["x"].get<int>(), object_node["y"].get<int>(),
//			object_node["width"].get<int>(), object_node["height"].get<int>()
//		);
//
//		std::string emit_output = ""; // optional
//		int emit_output_lifetime = 0; // optional
//
//		// Parse custom properties
//		for (const auto &property_node : object_node["properties"]) {
//			const std::string prefix = tags::getPrefix(property_node["name"].get<std::string>());
//
//			if (prefix == "emit_output") {
//				emit_output = property_node["value"].get<std::string>();
//			}
//			else if (prefix == "emit_output_lifetime") {
//				emit_output_lifetime = property_node["value"].get<int>();
//			}
//		}
//
//		auto script = std::make_unique<scripts::PlayerInArea>(hitbox);
//		script->setOutput(emit_output, emit_output_lifetime);
//
//		this->scripts.insert(std::move(script));
//	}
//}
//void Level::parse_objectgroup_script_AND(const nlohmann::json &objectgroup_node) {
//	const nlohmann::json &objects_array_node = objectgroup_node["objects"];
//	for (const auto &object_node : objects_array_node) {
//
//		std::string emit_output = ""; // optional
//		int emit_output_lifetime = 0; // optional
//
//		std::unordered_set<std::string> emit_inputs;
//
//		// Parse custom properties
//		for (const auto &property_node : object_node["properties"]) {
//			const std::string prefix = tags::getPrefix(property_node["name"].get<std::string>());
//
//			if (prefix == "emit_output") {
//				emit_output = property_node["value"].get<std::string>();
//			}
//			else if (prefix == "emit_output_lifetime") {
//				emit_output_lifetime = property_node["value"].get<int>();
//			}
//			else if (prefix == "emit_input") {
//				// we don't care about suffix in this case, we only care about having duplicates-by-prefix
//				emit_inputs.insert(property_node["value"].get<std::string>());
//			}
//		}
//
//		auto script = std::make_unique<scripts::AND>(emit_inputs);
//		script->setOutput(emit_output, emit_output_lifetime);
//
//		this->scripts.insert(std::move(script));
//	}
//}
//void Level::parse_objectgroup_script_OR(const nlohmann::json &objectgroup_node) {
//	const nlohmann::json &objects_array_node = objectgroup_node["objects"];
//	for (const auto &object_node : objects_array_node) {
//
//		std::string emit_output = ""; // optional
//		int emit_output_lifetime = 0; // optional
//
//		std::unordered_set<std::string> emit_inputs;
//
//		// Parse custom properties
//		for (const auto &property_node : object_node["properties"]) {
//			const std::string prefix = tags::getPrefix(property_node["name"].get<std::string>());
//
//			if (prefix == "emit_output") {
//				emit_output = property_node["value"].get<std::string>();
//			}
//			else if (prefix == "emit_output_lifetime") {
//				emit_output_lifetime = property_node["value"].get<int>();
//			}
//			else if (prefix == "emit_input") {
//				// we don't care about suffix in this case, we only care about having duplicates-by-prefix
//				emit_inputs.insert(property_node["value"].get<std::string>());
//			}
//		}
//
//		auto script = std::make_unique<scripts::OR>(emit_inputs);
//		script->setOutput(emit_output, emit_output_lifetime);
//
//		this->scripts.insert(std::move(script));
//	}
//}
//void Level::parse_objectgroup_script_XOR(const nlohmann::json &objectgroup_node) {
//	const nlohmann::json &objects_array_node = objectgroup_node["objects"];
//	for (const auto &object_node : objects_array_node) {
//
//		std::string emit_output = ""; // optional
//		int emit_output_lifetime = 0; // optional
//
//		std::unordered_set<std::string> emit_inputs;
//
//		// Parse custom properties
//		for (const auto &property_node : object_node["properties"]) {
//			const std::string prefix = tags::getPrefix(property_node["name"].get<std::string>());
//
//			if (prefix == "emit_output") {
//				emit_output = property_node["value"].get<std::string>();
//			}
//			else if (prefix == "emit_output_lifetime") {
//				emit_output_lifetime = property_node["value"].get<int>();
//			}
//			else if (prefix == "emit_input") {
//				// we don't care about suffix in this case, we only care about having duplicates-by-prefix
//				emit_inputs.insert(property_node["value"].get<std::string>());
//			}
//		}
//
//		auto script = std::make_unique<scripts::XOR>(emit_inputs);
//		script->setOutput(emit_output, emit_output_lifetime);
//
//		this->scripts.insert(std::move(script));
//	}
//}
//void Level::parse_objectgroup_script_NAND(const nlohmann::json &objectgroup_node) {
//	const nlohmann::json &objects_array_node = objectgroup_node["objects"];
//	for (const auto &object_node : objects_array_node) {
//
//		std::string emit_output = ""; // optional
//		int emit_output_lifetime = 0; // optional
//
//		std::unordered_set<std::string> emit_inputs;
//
//		// Parse custom properties
//		for (const auto &property_node : object_node["properties"]) {
//			const std::string prefix = tags::getPrefix(property_node["name"].get<std::string>());
//
//			if (prefix == "emit_output") {
//				emit_output = property_node["value"].get<std::string>();
//			}
//			else if (prefix == "emit_output_lifetime") {
//				emit_output_lifetime = property_node["value"].get<int>();
//			}
//			else if (prefix == "emit_input") {
//				// we don't care about suffix in this case, we only care about having duplicates-by-prefix
//				emit_inputs.insert(property_node["value"].get<std::string>());
//			}
//		}
//
//		auto script = std::make_unique<scripts::NAND>(emit_inputs);
//		script->setOutput(emit_output, emit_output_lifetime);
//
//		this->scripts.insert(std::move(script));
//	}
//}
//void Level::parse_objectgroup_script_NOR(const nlohmann::json &objectgroup_node) {
//	const nlohmann::json &objects_array_node = objectgroup_node["objects"];
//	for (const auto &object_node : objects_array_node) {
//
//		std::string emit_output = ""; // optional
//		int emit_output_lifetime = 0; // optional
//
//		std::unordered_set<std::string> emit_inputs;
//
//		// Parse custom properties
//		for (const auto &property_node : object_node["properties"]) {
//			const std::string prefix = tags::getPrefix(property_node["name"].get<std::string>());
//
//			if (prefix == "emit_output") {
//				emit_output = property_node["value"].get<std::string>();
//			}
//			else if (prefix == "emit_output_lifetime") {
//				emit_output_lifetime = property_node["value"].get<int>();
//			}
//			else if (prefix == "emit_input") {
//				// we don't care about suffix in this case, we only care about having duplicates-by-prefix
//				emit_inputs.insert(property_node["value"].get<std::string>());
//			}
//		}
//
//		auto script = std::make_unique<scripts::NOR>(emit_inputs);
//		script->setOutput(emit_output, emit_output_lifetime);
//
//		this->scripts.insert(std::move(script));
//	}
//}
//void Level::parse_objectgroup_script_XNOR(const nlohmann::json &objectgroup_node) {
//	const nlohmann::json &objects_array_node = objectgroup_node["objects"];
//	for (const auto &object_node : objects_array_node) {
//
//		std::string emit_output = ""; // optional
//		int emit_output_lifetime = 0; // optional
//
//		std::unordered_set<std::string> emit_inputs;
//
//		// Parse custom properties
//		for (const auto &property_node : object_node["properties"]) {
//			const std::string prefix = tags::getPrefix(property_node["name"].get<std::string>());
//
//			if (prefix == "emit_output") {
//				emit_output = property_node["value"].get<std::string>();
//			}
//			else if (prefix == "emit_output_lifetime") {
//				emit_output_lifetime = property_node["value"].get<int>();
//			}
//			else if (prefix == "emit_input") {
//				// we don't care about suffix in this case, we only care about having duplicates-by-prefix
//				emit_inputs.insert(property_node["value"].get<std::string>());
//			}
//		}
//
//		auto script = std::make_unique<scripts::XNOR>(emit_inputs);
//		script->setOutput(emit_output, emit_output_lifetime);
//
//		this->scripts.insert(std::move(script));
//	}
//}