#pragma once

#include <SDL.h> // rendering
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

	Graphics(const LaunchInfo &launchInfo);

	~Graphics(); // frees loaded textures

	static const Graphics* READ; // used for aka 'global' access
	static Graphics* ACCESS;

	std::unique_ptr<Camera> camera;
	std::unique_ptr<Gui> gui;

	SDL_Texture* getTexture(const std::string &filePath);
	// Cases of getTexture() (convenience thing)
	SDL_Texture* getTexture_Entity(const std::string &name);
	SDL_Texture* getTexture_Item(const std::string &name);
	SDL_Texture* getTexture_Tileset(const std::string &name);
	SDL_Texture* getTexture_Background(const std::string &name);
	SDL_Texture* getTexture_GUI(const std::string &name);

	void unloadImages(); // clears the map of loaded images

	void rendererToWindow(); // draws content of backbuffer (renderer) to screen
	void rendererClear(); // clears content of renderer (used each frame)

	void copyTextureToRenderer(SDL_Texture* texture, const SDL_Rect* sourceRect, const SDL_Rect* destRect);
		// copies sourceRect from given texture to destinationRect on renderer
	void copyTextureToRendererEx(SDL_Texture* texture, const SDL_Rect* sourceRect, const SDL_Rect* destRect, double angle, SDL_RendererFlip flip = SDL_FLIP_NONE);
		// same as above but allows rotation and flips

	SDL_Renderer* getRenderer() const; // returns renderer	

	int width() const;
	int height() const;
	double scaling_factor() const;
private:
	SDL_Window* window;
	SDL_Renderer* renderer;

	int rendering_width;
	int rendering_height;
	double rendering_scaling_factor; // == <renderingresolution> / <natural resolution>

	std::unordered_map<std::string, SDL_Texture*> loadedImages; // all loaded images are saved here as SDL_Surface
};