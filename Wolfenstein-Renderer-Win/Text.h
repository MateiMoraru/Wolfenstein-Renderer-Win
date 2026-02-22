#ifndef TEXT_H
#define TEXT_H

#include <SDL.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

extern char font8x8_basic[128][8];

typedef struct
{
	int width;
	int height;
	const uint8_t(*data)[8];
	SDL_Texture* glyphs[128];
} Font;

void font_init(SDL_Renderer* renderer, Font* font, const uint8_t data[128][8], int font_size, SDL_Color color);
void font_destroy(Font* font);

void text_draw(SDL_Renderer* renderer, Font* font, int x, int y, const char* text, int font_size, SDL_Color color);
void text_draw_shadow(SDL_Renderer* renderer, Font* font, int x, int y, const char* text, int font_size, SDL_Color color);
#endif
