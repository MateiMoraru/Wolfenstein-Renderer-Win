#ifndef TEXT_H
#define TEXT_H

#include <SDL.h>
#include <stdint.h>
#include <stdint.h>

extern char font8x8_basic[128][8];

typedef struct {
    int width;
    int height;
    const uint8_t(*data)[8];
} Font;

void text_draw(SDL_Renderer* renderer, Font* font, int x, int y, const char* text, int font_size, SDL_Color color);

#endif