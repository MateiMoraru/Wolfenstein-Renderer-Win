#ifndef BUTTON_H
#define BUTTON_H

#include <stdbool.h>

#include "window.h"
#include "text.h"
#include "utils.h"

typedef struct Button
{
	int id;

	int x;
	int y;
	int width;
	int height;
	SDL_Rect rect;

	bool pressed;
	bool hovered;

	SDL_Color color;

	char* string;
	Font* font;
	int font_size;

	int text_x;
	int text_y;

	Window* window;
} Button;

Button button_init(Window* window, Font* font, const char* string, int font_size, int x, int y, int w, int h, SDL_Color color, const int id);
void button_draw(Button* button);
bool button_update(Button* button);

#endif