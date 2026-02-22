#ifndef SPRITE_H
#define SPRITE_H

#include <SDL.h>

#include "asset_drawer.h"
#include "Window.h"

typedef struct
{
	uint32_t* column;
} SpriteColorColumn;

typedef struct
{
	float x;
	float y;

	int width;
	int height;

	float shade;

	SDL_Texture* texture;
	SpriteColorColumn* columns;
	SDL_Texture** tex_columns;
} Sprite;

Sprite sprite_load(SDL_Renderer* renderer, const char* filename, float x, float y);
void sprite_draw(Window* window, Sprite* sprite, float scale);

#endif