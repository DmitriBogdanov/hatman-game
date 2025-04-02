#include "graphics/graphics.h"

#include <SFML/Graphics.hpp>

#include <iostream>

#include "utility/globalconsts.hpp" // natural consts

// # Graphics #
const Graphics* Graphics::READ;
Graphics* Graphics::ACCESS;


// Construction and creation of a window and renderer
Graphics::Graphics(int width, int height, sf::Uint32 style) :
	rendering_width(width),
	rendering_height(height),
	rendering_scaling_factor(static_cast<double>(width) / natural::WIDTH) // cast to double or we get integer division
{
	std::cout << "Creating window and renderer...\n";

	Graphics::READ = this; // init global access
	Graphics::ACCESS = this;

	// When borderless window has the exact same resolution as the screen
	// some OSs (notably Windows 10) perform "fullscreen optimization" that
	// replaces borderless fullscreen window with regular fullscreen. To avoid it
	// we can "trick" the system by increasing vertical size by 1 pixel, which
	// keeps a proper borderless window wint no visual difference
	if (style == sf::Style::None && sf::VideoMode::getDesktopMode() == sf::VideoMode(width, height)) {
		std::cout
			<< "Borderless configuration matches desktop video mode, "
			<< "size increased by 1 to avoid fullscreen optimization.\n";
		++height;
	}

	// If window screen mode was chosed, black out resize button so it can't break internal scaling
	if (style == sf::Style::Default) {
		style = sf::Style::Titlebar | sf::Style::Close;
	}

	// Create window
	this->window.create(sf::VideoMode(width, height), "Hatman Adventure", style);

	// Set icon
	this->icon.loadFromFile("icon.png");
	this->window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());

	this->window.setFramerateLimit(200);

	this->camera = std::make_unique<Camera>();
	this->gui = std::make_unique<Gui>();
}

// Image loading
sf::Texture& Graphics::getTexture(const std::string &filePath) {
	if (!this->loadedTextures.count(filePath)) { // image is not loaded => load it, add to the map
		sf::Texture texture;
		texture.loadFromFile(filePath);
		/// ADD ERROR HANDLING

		this->loadedTextures[filePath] = std::move(texture);
	}

	return this->loadedTextures.at(filePath);
}
sf::Texture& Graphics::getTexture_Entity(const std::string &name) {
	return this->getTexture("content/textures/entities/" + name);
}

sf::Texture& Graphics::getTexture_Item(const std::string &name) {
	return this->getTexture("content/textures/items/" + name);
}
sf::Texture& Graphics::getTexture_Tileset(const std::string &name) {
	return this->getTexture("content/textures/tilesets/" + name);
}
sf::Texture& Graphics::getTexture_Background(const std::string &name) {
	return this->getTexture("content/textures/backgrounds/" + name);
}
sf::Texture& Graphics::getTexture_GUI(const std::string &name) {
	return this->getTexture("content/textures/gui/" + name);
}

// Rendering
void Graphics::window_clear() {
	this->window.clear();
}
void Graphics::window_draw_sprite(sf::Sprite &sprite) {
	this->window.draw(sprite);
}
void Graphics::window_display() {
	this->window.display();
}

int Graphics::width() const { return this->rendering_width; }
int Graphics::height() const { return this->rendering_height; }
double Graphics::scaling_factor() const { return this->rendering_scaling_factor; }