#pragma once

#include <SFML/Graphics.hpp>

#include <unordered_map> // related type
#include <memory> // 'unique_ptr' type
#include <string> // related type

#include "geometry_utils.h" // geometry types
#include "launch_info.h" // 'LaunchInfo' class
#include "gui.h" // 'Gui' module
#include "camera.h" // 'Camera' module



class Gui; // forward declare graphics subsystems
class Camera;



// # Graphics #
// - Can be accessed wherever #include'ed through static 'READ' and 'ACCESS' fields
// - Handles window creation, rendering and loading of images
// - Only one instance at a time should exits (creation of new instances however is not controlled in any way)
class Graphics {
public:
	Graphics() = delete;

	Graphics(int width, int height, sf::Uint32 style);

	~Graphics() = default;

	static const Graphics* READ; // used for aka 'global' access
	static Graphics* ACCESS;

	std::unique_ptr<Camera> camera;
	std::unique_ptr<Gui> gui;

	sf::Texture& getTexture(const std::string &filePath);
	// Cases of getTexture() (convenience thing)
	sf::Texture& getTexture_Entity(const std::string &name);
	sf::Texture& getTexture_Item(const std::string &name);
	sf::Texture& getTexture_Tileset(const std::string &name);
	sf::Texture& getTexture_Background(const std::string &name);
	sf::Texture& getTexture_GUI(const std::string &name);


	
	void window_clear();                         // 1) Clear window
	void window_draw_sprite(sf::Sprite &sprite); // 2) Draw all sprites through Cameta and Gui
	void window_display();                       // 3) Display drawn sprites
	
	int width() const;
	int height() const;
	double scaling_factor() const;


	sf::RenderWindow window;
private:

	sf::Image icon;

	int rendering_width;
	int rendering_height;
	double rendering_scaling_factor; // == <renderingresolution> / <natural resolution>

	std::unordered_map<std::string, sf::Texture> loadedTextures; // all loaded images are saved here

	///friend Game;
};

