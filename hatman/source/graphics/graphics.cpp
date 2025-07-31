#include "graphics/graphics.h"

#include <iostream>

#include "SFML/System/Vector2.hpp"
#include "SFML/Window/VideoMode.hpp"
#include "SFML/Window/WindowStyle.hpp"

#include "utility/geometry.h"
#include "utility/globalconsts.hpp" // natural consts
#include "utility/filepaths.hpp"

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
    
    const sf::VideoMode desktop_mode = sf::VideoMode::getDesktopMode();

    // When borderless window has the exact same resolution as the screen
    // some OSs (notably Windows 10) perform "fullscreen optimization" that
    // replaces borderless fullscreen window with regular fullscreen. To avoid it
    // we can "trick" the system by increasing vertical size by 1 pixel, which
    // keeps a proper borderless window wint no visual difference
    if (style == sf::Style::None && desktop_mode == sf::VideoMode(width, height)) {
    	std::cout
    		<< "Borderless configuration matches desktop video mode, "
    		<< "size increased by 1 to avoid fullscreen optimization.\n";
    	//++height;
    }

	// If window screen mode was chosen, black out resize button so it can't break internal scaling
	if (style == sf::Style::Default) {
		style = sf::Style::Titlebar | sf::Style::Close;
	}

	// Create window
	this->window.create(sf::VideoMode(width, height), "Hatman Adventure", style);
     
    // Move window & borderless to center, some systems will position them weirdly otherwise,
    // not a huge deal for a regular window, but makes borderless unplayable
    if (style == sf::Style::None || style == (sf::Style::Titlebar | sf::Style::Close)) {
        std::cout
            << "Desktop size is {" << desktop_mode.width << ", " << desktop_mode.height << "}, "
            << "window size is {" << width << ", " << height << "}, centering...\n";
        
        const int corner_x = static_cast<int>(desktop_mode.width) / 2 - width / 2;
        const int corner_y = static_cast<int>(desktop_mode.height) / 2 - height / 2;
        
        std::cout << "New window position -> {" << corner_x << ", " << corner_y << "}\n";
    
        this->window.setPosition(sf::Vector2i{corner_x, corner_y});
    }
    
	// Set icon
	this->icon.loadFromFile("icon.png");
	this->window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());

	//this->window.setFramerateLimit(200);

	this->camera = std::make_unique<Camera>();
	this->gui = std::make_unique<Gui>();
}

// Image loading
sf::Texture& Graphics::get_texture(const std::string &filePath) {
	if (!this->loadedTextures.count(filePath)) { // image is not loaded => load it, add to the map
		sf::Texture texture;
		texture.loadFromFile(filePath);
		/// ADD ERROR HANDLING

		this->loadedTextures[filePath] = std::move(texture);
	}

	return this->loadedTextures.at(filePath);
}
sf::Texture& Graphics::getTexture_Entity(const std::string &name) {
	return this->get_texture(paths::TEXTURES_ENTITIES + name);
}

sf::Texture& Graphics::getTexture_Item(const std::string &name) {
	return this->get_texture(paths::TEXTURES_ITEMS + name);
}
sf::Texture& Graphics::getTexture_Tileset(const std::string &name) {
	return this->get_texture(paths::TEXTURES_TILESETS + name);
}
sf::Texture& Graphics::getTexture_Background(const std::string &name) {
	return this->get_texture(paths::TEXTURES_BACKGROUNDS + name);
}
sf::Texture& Graphics::getTexture_GUI(const std::string &name) {
	return this->get_texture(paths::TEXTURES_GUI + name);
}

// Rendering
void Graphics::window_clear() {
	this->window.clear();
}
void Graphics::window_draw_sprite(sf::Sprite &sprite) {
    const auto [x, y] = sprite.getPosition();
    sprite.setPosition(static_cast<int>(x), static_cast<int>(y));
    
	this->window.draw(sprite);
}
void Graphics::window_display() {
	this->window.display();
}

int Graphics::width() const { return this->rendering_width; }
int Graphics::height() const { return this->rendering_height; }
double Graphics::scaling_factor() const { return this->rendering_scaling_factor; }