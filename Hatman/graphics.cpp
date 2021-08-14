#include "graphics.h"

#include <SDL_image.h> // loading of texture from image files
#include "globalconsts.hpp" // natural consts

// # Graphics #
const Graphics* Graphics::READ;
Graphics* Graphics::ACCESS;

// Construction and creation of a window and renderer
Graphics::Graphics(const LaunchInfo &launchInfo) :
	rendering_width(launchInfo.window_width),
	rendering_height(launchInfo.window_height),
	rendering_scaling_factor(static_cast<double>(this->rendering_width) / natural::WIDTH)
{
	Graphics::READ = this; // init global access
	Graphics::ACCESS = this;

	SDL_Init(SDL_INIT_VIDEO);

	SDL_CreateWindowAndRenderer(launchInfo.window_width, launchInfo.window_height, launchInfo.window_flag, &this->window, &this->renderer);

	SDL_RenderSetLogicalSize(this->renderer, this->rendering_width, this->rendering_height); // rendering size is the same as window size

	///SDL_RenderSetScale(this->renderer, launchInfo.window_renderingScaleX, launchInfo.window_renderingScaleY);
	// Cement the fact that we want CRISP INTEGER SCALING!
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
	SDL_RenderSetIntegerScale(this->renderer, SDL_TRUE);

	SDL_SetRenderDrawColor(this->renderer, 0, 0, 0, 0); // set SDL_RenderClear() color to transparent, necessary for proper blending
	SDL_SetWindowTitle(this->window, "Hatman Adventure");

	this->camera = std::make_unique<Camera>();
	this->gui = std::make_unique<Gui>();
}
Graphics::~Graphics() {
	this->unloadImages();
	SDL_DestroyRenderer(this->renderer);
	SDL_DestroyWindow(this->window);
}

// Image loading
SDL_Texture* Graphics::getTexture(const std::string &filePath) {
	if (!this->loadedImages.count(filePath)) { // image is not loaded => load it, add to the map
		SDL_Surface* loadedSurface = IMG_Load(filePath.c_str()); // IMG_Load() accepts only C-string
		this->loadedImages[filePath] = SDL_CreateTextureFromSurface(this->renderer, loadedSurface);
		SDL_FreeSurface(loadedSurface);
	}
	return this->loadedImages.at(filePath);
}
SDL_Texture* Graphics::getTexture_Entity(const std::string &name) {
	return this->getTexture("content/textures/entities/" + name);
}

SDL_Texture* Graphics::getTexture_Item(const std::string &name) {
	return this->getTexture("content/textures/items/" + name);
}
SDL_Texture* Graphics::getTexture_Tileset(const std::string &name) {
	return this->getTexture("content/textures/tilesets/" + name);
}
SDL_Texture* Graphics::getTexture_Background(const std::string &name) {
	return this->getTexture("content/textures/backgrounds/" + name);
}
SDL_Texture* Graphics::getTexture_GUI(const std::string &name) {
	return this->getTexture("content/textures/gui/" + name);
}
void Graphics::unloadImages() { 
	for (auto& element : this->loadedImages) {
		SDL_DestroyTexture(element.second);
	}
}

// Rendering
SDL_Renderer* Graphics::getRenderer() const { return this->renderer; } 
void Graphics::rendererToWindow() {	SDL_RenderPresent(this->renderer); }
void Graphics::rendererClear() {
	SDL_SetRenderTarget(Graphics::ACCESS->getRenderer(), NULL); // take rendering target

	SDL_RenderClear(this->renderer); 
}
void Graphics::copyTextureToRenderer(SDL_Texture* texture, const SDL_Rect* sourceRect, const SDL_Rect* destRect) {
	SDL_SetRenderTarget(Graphics::ACCESS->getRenderer(), NULL); // take rendering target

	SDL_RenderCopy(this->renderer, texture, sourceRect, destRect);
}
void Graphics::copyTextureToRendererEx(SDL_Texture* texture, const SDL_Rect* sourceRect, const SDL_Rect* destRect, double angle, SDL_RendererFlip flip) {
	SDL_SetRenderTarget(Graphics::ACCESS->getRenderer(), NULL); // take rendering target

	SDL_RenderCopyEx(this->renderer, texture, sourceRect, destRect, angle, NULL, flip);
}

int Graphics::width() const { return this->rendering_width; }
int Graphics::height() const { return this->rendering_height; }
double Graphics::scaling_factor() const { return this->rendering_scaling_factor; }