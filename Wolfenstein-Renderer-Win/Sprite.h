#ifndef SPRITE_H
#define SPRITE_H

#include <SDL.h>

#include "asset_drawer.h"
#include "Window.h"

typedef struct
{
	float x;
	float y;

	int width;
	int height;

	SDL_Texture* texture;

} Sprite;

Sprite sprite_load(SDL_Renderer* renderer, const char* filename, float x, float y);
void sprite_draw(Window* window, Sprite* sprite, float scale);

#endif