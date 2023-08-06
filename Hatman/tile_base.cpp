#include "tile_base.h"

#include <fstream> // parsing from JSON (opening a file)

#include "graphics.h" // access to texture loading (tileset texture loading)
#include "globalconsts.hpp" // natural consts (tile size)
#include "tags.h"



// # Tile::Hitbox #
TileHitbox::TileHitbox(const std::vector<TileHitboxRect> &rects) :
	rectangles(rects)
{}
TileHitbox::TileHitbox(std::vector<TileHitboxRect> &&rects) :
	rectangles(std::move(rects))
{}

// # Tile::Interaction #
TileInteraction::TileInteraction(const std::string &interactive_type, dRect &actionbox) :
	interactive_type(interactive_type),
	actionbox(actionbox)
{}

void TileInteraction::setInput(const std::string &emit) {
	this->emit_input = emit; 
}
void TileInteraction::setOutput(const std::string &emit, int lifetime) {
	this->emit_output = emit; 
	this->emit_duration = lifetime; 
}



// # Tile #
Tile::Tile(const Tile &other) :
	position(other.position),
	tilesheet_sprite(other.tilesheet_sprite)
{
	if (other.hitbox) { this->hitbox = std::make_unique<TileHitbox>(*other.hitbox); }
	else { this->hitbox = nullptr; }

	if (other.sprite) { this->sprite = nullptr;/*std::make_unique<Sprite>(*other.sprite);*/ }
	else { this->sprite = nullptr; }

	if (other.interaction) { this->interaction = std::make_unique<TileInteraction>(*other.interaction); }
	else { this->interaction = nullptr; }
}
Tile::Tile(Tile &&other) : 
	position(other.position),
	hitbox(std::move(other.hitbox)),
	sprite(std::move(other.sprite)),
	interaction(std::move(other.interaction))
{}

Tile::Tile(const Tileset &tileset, int id, const Vector2 position) :
	position(position),
	hitbox(nullptr),
	sprite(nullptr),
	interaction(nullptr)
{
	// set hitbox (if present)
	if (tileset.has_tile_hitbox(id)) {
		this->hitbox = std::make_unique<TileHitbox>(tileset.get_tile_hitbox(id));

		for (auto& hitboxRect : this->hitbox->rectangles) { hitboxRect.rect.moveBy(this->position); }
	}

	// set animation (if present)
	if (tileset.has_tile_animation(id)) {
		this->sprite = std::make_unique<AnimatedSprite>(
			this->position,
			false,
			false,
			tileset.get_tile_animation(id)
			);
	}
	else {
		this->sprite = std::make_unique<StaticSprite>(	
			this->position,
			false,
			false,
			*tileset.tileset_get_texture(),
			tileset.get_tile_source_rect(id)
			);
	}

	// set actionbox (if present)
	if (tileset.has_tile_interaction(id)) {
		this->interaction = std::make_unique<TileInteraction>(tileset.get_tile_interaction(id));

		this->interaction->actionbox.moveBy(this->position);
	}
}

void Tile::update(Milliseconds elapsedTime) {
	// Update sprite
	if (this->sprite) { this->sprite->update(elapsedTime); }

	// Update interaction (if present)
	if (this->interaction) {
		const bool activation = this->checkActivation();
		if (this->toggle_active != activation) { // activation changed
			this->toggle_active = activation;

			if (activation) { this->activate(); }
			else { this->deactivate(); }
		}

		if (this->toggle_active && this->checkTrigger()) { this->trigger(); }
	}
}

void Tile::draw() const {
	if (this->sprite) { this->sprite->draw(); }
}



// # Tileset #
Tileset::Tileset(const std::string &fileName) {
	this->parseFromJSON("content/tilesets/" + fileName);
}

void Tileset::parseFromJSON(const std::string &filePath) {
	std::ifstream ifStream(filePath);
	nlohmann::json JSON = nlohmann::json::parse(ifStream);

	// Parsing...
	// (these field have to be in any valid tileset)
	std::string tilesetFileName = filePath;
	tilesetFileName = tilesetFileName.substr(tilesetFileName.rfind("/") + 1); // cut before '/'
	tilesetFileName = tilesetFileName.substr(tilesetFileName.rfind("\\") + 1); // cut before '\'
	this->filename = tilesetFileName;

	std::string imageFileName = JSON["image"].get<std::string>();
	imageFileName = imageFileName.substr(imageFileName.rfind("/") + 1); // cut before '/'
	imageFileName = imageFileName.substr(imageFileName.rfind("\\") + 1); // cut before '\'
	this->texture = &Graphics::ACCESS->getTexture_Tileset(imageFileName);

	const int columns = JSON["columns"].get<int>();
	const int rows = JSON["tilecount"].get<int>() / columns;
	this->size = Vector2(columns, rows);

	// Parsing tile objects (hitboxes, animations)
	// (this field may not be present, in that case 'for' does 0 iterations)
	nlohmann::json tiles_array_node = JSON["tiles"]; // Array of tile objects (contains hitboxes)
	for (auto const& tile_node : tiles_array_node) {
		const int tileId = tile_node["id"].get<int>();

		// Parse hitbox/interaction/entity
		if (tile_node.find("objectgroup") != tile_node.end()) { // ["objectgroup"] is present => parse hitboxes/actionboxes

			TileHitbox hitbox;
			bool hitboxPresent = false;

			TileInteraction interaction;
			bool interactionPresent = false;

			EntitySpawnData entityData;
			bool entityPresent = false;

			// Parse things above
			nlohmann::json objects_node = tile_node["objectgroup"]["objects"];
			for (auto const& object : objects_node) {
				const std::string objectType = tags::getPrefix(object["type"].get<std::string>());

				// HITBOX
				if (objectType == "tile_hitbox") {
					hitboxPresent = true;

					hitbox.rectangles.push_back(this->parse_as_hitboxrect(object));
				}
				// ACTIONBOX
				else if (objectType == "tile_actionbox") {
					interactionPresent = true;

					interaction = this->parse_as_interaction(object);
				}
				// ENTITY
				else if (objectType == "entity") {
					entityPresent = true;

					entityData = this->parse_as_entity(object);
				}

				

				///// OLD
				//const dRect hitboxRect = dRect(
				//	Vector2(object["x"].get<int>(), object["y"].get<int>()),
				//	Vector2(object["width"].get<int>(), object["height"].get<int>())
				//);

				//bool hitboxRectIsPlatform = false;

				//// Parse type of the rectangle (hitbox/actionbox)
				//std::string rectType;
				//nlohmann::json properties_array_node = object["properties"];
				//for (const auto& property_node : properties_array_node) {
				//	const auto propertyName = property_node["name"].get<std::string>();
				//	if (propertyName == "type") {
				//		rectType = property_node["value"].get<std::string>();
				//	}
				//	else if (propertyName == "is_platform") {
				//		hitboxRectIsPlatform = property_node["value"].get<bool>();
				//	}
				//}
				// 

				//if (rectType == "hitbox") { 
				//	hitboxPresent = true;
				//	hitbox.rectangles.push_back({ hitboxRect, hitboxRectIsPlatform });
				//}
				//else if (rectType == "actionbox") {
				//	interactionPresent = true;
				//	interaction.actionbox = hitboxRect;
				//} // if there's more than one actionbox => map creator fucked up

				//// Parse properties unique for interactions (if interaction is present)
				//if (interactionPresent) {
				//	for (const auto& property_node : properties_array_node) {
				//		if (property_node["name"].get<std::string>() == "interactive_type") {
				//			interaction.interactive_type = property_node["value"].get<std::string>();
				//		}
				//	}
				//}
				
			}

			// Add parsed info to the map
			if (hitboxPresent) this->tileHitboxes[tileId] = hitbox;
			if (interactionPresent) this->tileInteractions[tileId] = interaction;
			if (entityPresent) this->entity_objects[tileId] = entityData;
		}	

		// Parse animation
		if (tile_node.find("animation") != tile_node.end()) { // ["animation"] is present => parse animation

			std::vector<AnimationFrame> frames;

			nlohmann::json animation_array_node = tile_node["animation"];
			for (const auto& frame_node : animation_array_node) {
				const int frameTileId = frame_node["tileid"].get<int>();
				const int tilePosX = frameTileId % this->size.x;
				const int tilePosY = frameTileId / this->size.x;
				const int tileSize = natural::TILE_SIZE;
				const srcRect frameRect = {
					tilePosX * tileSize, tilePosY * tileSize,
					tileSize, tileSize
				};

				const double frameDuration = frame_node["duration"].get<double>();

				frames.push_back(AnimationFrame{ frameRect, frameDuration });
			}

			this->tileAnimations[tileId] = Animation(*this->texture, frames);
		}
	}

}

TileHitboxRect Tileset::parse_as_hitboxrect(const nlohmann::json& object_node) {
	// Parse rect
	const auto hitboxRect = dRect(
		Vector2(object_node["x"].get<int>(), object_node["y"].get<int>()),
		Vector2(object_node["width"].get<int>(), object_node["height"].get<int>())
	);

	// Determine if it's a platform
	bool isPlatform = false;

	nlohmann::json properties_array_node = object_node["properties"];
	for (const auto& property_node : properties_array_node) {
		const auto propertyName = property_node["name"].get<std::string>();
		if (propertyName == "is_platform") {
			isPlatform = property_node["value"].get<bool>();
		}
	}

	return { hitboxRect, isPlatform };
}

TileInteraction Tileset::parse_as_interaction(const nlohmann::json& object_node) {
	TileInteraction interaction;

	// Parse rect
	interaction.actionbox = dRect(
		Vector2(object_node["x"].get<int>(), object_node["y"].get<int>()),
		Vector2(object_node["width"].get<int>(), object_node["height"].get<int>())
	);

	// Parse interactive type
	nlohmann::json properties_array_node = object_node["properties"];
	for (const auto& property_node : properties_array_node) {
		if (property_node["name"].get<std::string>() == "interactive_type") {
			interaction.interactive_type = property_node["value"].get<std::string>();
		}
	}

	return interaction;
}

EntitySpawnData Tileset::parse_as_entity(const nlohmann::json& object_node) {
	// Parse entity type
	const std::string entityType = tags::getSuffix(object_node["type"].get<std::string>());

	// Parse entity name
	std::string entityName;

	nlohmann::json properties_array_node = object_node["properties"];
	for (const auto& property_node : properties_array_node) {
		if (property_node["name"].get<std::string>() == "[name]") {
			entityName = property_node["value"].get<std::string>();
		}
	}

	// Parse entity position
	const auto positionInTile = Vector2d(object_node["x"].get<double>(), object_node["y"].get<double>());

	return { entityType, entityName, positionInTile };
}

// General tile getters
srcRect Tileset::get_tile_source_rect(int tileId) const {
	const int tileSize = natural::TILE_SIZE;
	return make_srcRect(
		Vector2((tileId % this->size.x) * tileSize, (tileId / this->size.x) * tileSize),
		Vector2(tileSize, tileSize)
	);
}
// Hitbox getters
bool Tileset::has_tile_hitbox(int tileId) const { return this->tileHitboxes.count(tileId); }

TileHitbox Tileset::get_tile_hitbox(int tileId) const { return this->tileHitboxes.at(tileId); }

// Animation getters
bool Tileset::has_tile_animation(int tileId) const { return this->tileAnimations.count(tileId); }

Animation Tileset::get_tile_animation(int tileId) const { return this->tileAnimations.at(tileId); }

// Actionbox getters
bool Tileset::has_tile_interaction(int tileId) const { return this->tileInteractions.count(tileId); }

TileInteraction Tileset::get_tile_interaction(int tileId) const { return this->tileInteractions.at(tileId); }

// Entity getters
bool Tileset::has_entity_spawn_data(int tileId) const { return this->entity_objects.count(tileId); }
const EntitySpawnData& Tileset::get_entity_spawn_data(int tileId) const { return this->entity_objects.at(tileId); }

// Tileset Getters
std::string Tileset::tileset_get_filename() const { return this->filename; }

sf::Texture* Tileset::tileset_get_texture() const { return this->texture; }



// # TilesetStorage #
const TilesetStorage* TilesetStorage::READ;
TilesetStorage* TilesetStorage::ACCESS;

TilesetStorage::TilesetStorage() {
	this->READ = this;
	this->ACCESS = this;
}

const Tileset& TilesetStorage::getTileset(const std::string &fileName) {
	if (!this->loadedTilesets.count(fileName)) { // tileset is not already loaded => loaded
		this->loadedTilesets[fileName] = Tileset(fileName);
	}

	return this->loadedTilesets[fileName];
}