#include "Menu.h"

Menu menu_init(Window* window, int buttons_len, SDL_Color color_background)
{
	Menu menu;
	menu.window = window;
	menu.buttons = malloc(buttons_len * sizeof(Button));
	menu.buttons_len = 0;
	menu.buttons_max = buttons_len;

	menu.color = color_background;

	return menu;
}

void menu_add_button(Menu* menu, Button* button)
{
	if (menu->buttons_len == menu->buttons_max)
	{
		return;
	}
	menu->buttons[menu->buttons_len++] = *button;
}

int menu_draw(Menu* menu)
{
	SDL_SetRenderDrawColor(menu->window->renderer, menu->color.r, menu->color.g, menu->color.b, 255);
	SDL_RenderFillRect(menu->window->renderer, &(SDL_Rect) { 0, 0, menu->window->width, menu->window->height });

	for (int i = 0; i < menu->buttons_len; i++)
	{
		button_draw(&menu->buttons[i]);

		if (button_update(&menu->buttons[i]))
			return menu->buttons[i].id;
	}

	return -1;
}

void menu_destroy(Menu* menu)
{
	free(menu->buttons);
}

Menu main_menu_init(Window* window, Font* font, const int QUIT, const int START)
{
	Menu menu = menu_init(window, 2, (SDL_Color) { 30, 30, 30, 255 });

	int button_width = 300;
	int button_height = 60;
	int spacing = 20;
	int font_size = 3;

	int total_height = button_height * 2 + spacing;

	int start_x = (window->width - button_width) / 2;
	int start_y = (window->height - total_height) / 2;

	Button start_button = button_init(window, font, "START", font_size, start_x, start_y, button_width, button_height, (SDL_Color) { 70, 130, 180, 255 }, START);
	Button quit_button = button_init(window, font, "QUIT", font_size, start_x, start_y + button_height + spacing, button_width, button_height, (SDL_Color) { 180, 70, 70, 255 }, QUIT);

	menu_add_button(&menu, &start_button);
	menu_add_button(&menu, &quit_button);

	return menu;
}