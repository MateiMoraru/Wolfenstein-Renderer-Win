#ifndef MENU_H
#define MENU_H

#include "window.h"
#include "button.h"

typedef struct Menu
{
	Window* window;
	Button* buttons;
	int buttons_len;
	int buttons_max;

	SDL_Color color;
} Menu;

Menu menu_init(Window* window, int buttons_len, SDL_Color color_background);
void menu_add_button(Menu* menu, Button* button);
int menu_draw(Menu* menu);
void menu_destroy(Menu* menu);

Menu main_menu_init(Window* window, Font* font, const int QUIT, const int START);

#endif