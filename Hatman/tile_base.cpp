#include "tile_base.h"

#include <fstream> // parsing from JSON (opening a file)
#include "nlohmann_external.hpp" // parsing from JSON

#include "graphics.h" // access to texture loading (tileset texture loading)
#include "globalconsts.hpp" // natural consts (tile size)



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
	tilesheet(other.tilesheet)
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
	if (tileset.tileHas_Hitbox(id)) {
		this->hitbox = std::make_unique<TileHitbox>(tileset.tileGet_Hitbox(id));

		for (auto& hitboxRect : this->hitbox->rectangles) { hitboxRect.rect.moveBy(this->position); }
	}

	// set animation (if present)
	if (tileset.tileHas_Animation(id)) {
		this->sprite = std::make_unique<AnimatedSprite>(
			this->position,
			false,
			false,
			tileset.tileGet_Animation(id)
			);
	}
	else {
		this->sprite = std::make_unique<StaticSprite>(	
			this->position,
			false,
			false,
			tileset.getTilesheetImage(),
			tileset.tileGet_SourceRect(id)
			);
	}

	// set actionbox (if present)
	if (tileset.tileHas_Interaction(id)) {
		this->interaction = std::make_unique<TileInteraction>(tileset.tileGet_Interaction(id));

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
	this->fileName = tilesetFileName;

	std::string imageFileName = JSON["image"].get<std::string>();
	imageFileName = imageFileName.substr(imageFileName.rfind("/") + 1); // cut before '/'
	imageFileName = imageFileName.substr(imageFileName.rfind("\\") + 1); // cut before '\'
	this->tilesetImage = Graphics::ACCESS->getTexture_Tileset(imageFileName);

	const int columns = JSON["columns"].get<int>();
	const int rows = JSON["tilecount"].get<int>() / columns;
	this->size = Vector2(columns, rows);

	// Parsing tile objects (hitboxes, animations)
	// (this field may not be present, in that case 'for' does 0 iterations)
	nlohmann::json tiles_array_node = JSON["tiles"]; // Array of tile objects (contains hitboxes)
	for (auto const& tile_node : tiles_array_node) {
		const int tileId = tile_node["id"].get<int>();

		// Parse hitbox and interaction
		if (tile_node.find("objectgroup") != tile_node.end()) { // ["objectgroup"] is present => parse hitboxes/actionboxes

			TileHitbox hitbox;
			bool hitboxPresent = false;

			TileInteraction interaction;
			bool interactionPresent = false;

			nlohmann::json objects_node = tile_node["objectgroup"]["objects"];
			for (auto const& object : objects_node) {
				const dRect hitboxRect = dRect(
					Vector2(object["x"].get<int>(), object["y"].get<int>()),
					Vector2(object["width"].get<int>(), object["height"].get<int>())
				);

				bool hitboxRectIsPlatform = false;

				// Parse type of the rectangle (hitbox/actionbox)
				std::string rectType;
				nlohmann::json properties_array_node = object["properties"];
				for (const auto& property_node : properties_array_node) {
					const auto propertyName = property_node["name"].get<std::string>();
					if (propertyName == "type") {
						rectType = property_node["value"].get<std::string>();
					}
					else if (propertyName == "is_platform") {
						hitboxRectIsPlatform = property_node["value"].get<bool>();
					}
				}
				 

				if (rectType == "hitbox") { 
					hitboxPresent = true;
					hitbox.rectangles.push_back({ hitboxRect, hitboxRectIsPlatform });
				}
				else if (rectType == "actionbox") {
					interactionPresent = true;
					interaction.actionbox = hitboxRect;
				} // if there's more than one actionbox => map creator fucked up

				// Parse properties unique for interactions (if interaction is present)
				if (interactionPresent) {
					for (const auto& property_node : properties_array_node) {
						if (property_node["name"].get<std::string>() == "interactive_type") {
							interaction.interactive_type = property_node["value"].get<std::string>();
						}
					}
				}
				
			}


			if (hitboxPresent) { this->tileHitboxes[tileId] = hitbox; }
			if (interactionPresent) { this->tileInteractions[tileId] = interaction; }
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

			this->tileAnimations[tileId] = Animation(this->tilesetImage, frames);
		}
	}

}

// General tile getters
srcRect Tileset::tileGet_SourceRect(int tileId) const {
	const int tileSize = natural::TILE_SIZE;
	return make_srcRect(
		Vector2((tileId % this->size.x) * tileSize, (tileId / this->size.x) * tileSize),
		Vector2(tileSize, tileSize)
	);
}
// Hitbox getters
bool Tileset::tileHas_Hitbox(int tileId) const { return this->tileHitboxes.count(tileId); }
TileHitbox Tileset::tileGet_Hitbox(int tileId) const { return this->tileHitboxes.at(tileId); }
// Animation getters
bool Tileset::tileHas_Animation(int tileId) const { return this->tileAnimations.count(tileId); }
Animation Tileset::tileGet_Animation(int tileId) const { return this->tileAnimations.at(tileId); }
// Actionbox getters
bool Tileset::tileHas_Interaction(int tileId) const { return this->tileInteractions.count(tileId); }
TileInteraction Tileset::tileGet_Interaction(int tileId) const { return this->tileInteractions.at(tileId); }
// Tileset Getters
std::string Tileset::getFileName() const { return this->fileName; }
SDL_Texture* Tileset::getTilesheetImage() const { return this->tilesetImage; }



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