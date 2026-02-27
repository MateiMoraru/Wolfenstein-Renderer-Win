/*
	MENU

	Handles all the menus, for now just the main menu :0

*/


#ifndef MENU_H
#define MENU_H

#include "window.h"
#include "button.h"

#define MENU_MAX_TEXT_OBJ 64

typedef struct TextObj
{
	int x;
	int y;
	float size;
	SDL_Color color;
	const char* text;
	Font* font;
} TextObj;

typedef struct Menu
{
	Window* window;
	Button* buttons;
	int buttons_len;
	int buttons_max;

	int text_len;
	TextObj text[MENU_MAX_TEXT_OBJ];

	SDL_Color color;
} Menu;

// Nothing to explain here pretty sure
Menu menu_init(Window* window, int buttons_len, SDL_Color color_background);

// Adds a button to the list of buttons in the menu
void menu_add_button(Menu* menu, Button* button);

// Adds a text object to the list of text objs
void menu_add_text(Menu* menu, const char* text, int x, int y, SDL_Color color, Font* font, float font_size);

// Draws as well as updates
int menu_draw(Menu* menu);

// Free everything
void menu_destroy(Menu* menu);

// Initializing main menu, as basic as possible
Menu main_menu_init(Window* window, Font* font, const int QUIT, const int START);

// Classic just decreases the len of the buttons
void menu_remove_last_button(Menu* menu);

// Shifts everything
void menu_remove_button_at(Menu* menu, int index);

#endif