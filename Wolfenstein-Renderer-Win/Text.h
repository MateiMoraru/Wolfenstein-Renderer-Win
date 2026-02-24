/*
		TEXT

		Custom font objects and text drawing, as i didnt want to use SDL_ttf.h

		When using FONT_SIZE, it is scaled by 8 so for instance, for a font size of 16, use FONT_SIZE = 2
*/


#ifndef TEXT_H
#define TEXT_H

#include <SDL.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>


// font8x8 defined in hex in the 
extern char font8x8_basic[128][8];

typedef struct
{
	int width;
	int height;
	const uint8_t(*data)[8];

	// textures for chars as ~8x8 draw calls are extremely slow
	SDL_Texture* glyphs[128];
} Font;

// Call this after creating a Font object with the correct args like {8, 8, font8x8_basic}

void font_init(SDL_Renderer* renderer, Font* font, const uint8_t data[128][8], int font_size, SDL_Color color);
void font_destroy(Font* font);

void text_draw(SDL_Renderer* renderer, Font* font, int x, int y, const char* text, int font_size, SDL_Color color);
void text_draw_shadow(SDL_Renderer* renderer, Font* font, int x, int y, const char* text, int font_size, SDL_Color color);
#endif
