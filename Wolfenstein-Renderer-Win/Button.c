#include "Button.h"

Button button_init(Window* window, Font* font, const char* string, int font_size, int x, int y, int w, int h, SDL_Color color, const int id)
{
    Button button;

    button.id = id;

    button.x = x;
    button.y = y;
    button.width = w;
    button.height = h;
    button.rect = (SDL_Rect){ x, y, w, h };

    button.color = color;

    button.pressed = false;
    button.hovered = false;
    button.font = font;
    button.string = string;
    button.font_size = font_size;

    int text_w = 0;
    int text_h = 0;

    if (font && string)
    {
        int lines = 1;
        int line_len = 0;
        int max_line_len = 0;

        for (const char* p = string; *p; p++)
        {
            if (*p == '\n')
            {
                if (line_len > max_line_len) max_line_len = line_len;
                line_len = 0;
                lines++;
                continue;
            }
            line_len++;
        }

        if (line_len > max_line_len) max_line_len = line_len;

        text_w = max_line_len * font->width;
        text_h = lines * font->height;
    }

    button.text_x = x + (w - text_w) / 2;
    button.text_y = y + (h - text_h) / 2;

    button.window = window;

    return button;
}

void button_draw(Button* button)
{
	int r = button->color.r;
	int g = button->color.g;
	int b = button->color.b;

	if (button->hovered)
	{
		r *= 0.7;
		g *= 0.7;
		b *= 0.7;
	}
	SDL_SetRenderDrawColor(button->window->renderer, r, g, b, button->color.a);
	SDL_RenderFillRect(button->window->renderer, &button->rect);


	text_draw_shadow(button->window->renderer, button->font, button->text_x, button->text_y, button->string, button->font_size, (SDL_Color) { 255, 255, 255, 255 });
}

bool button_update(Button* button)
{
	int mouse_x;
	int mouse_y;

	Uint32 buttons = SDL_GetMouseState(&mouse_x, &mouse_y);

	bool hovered = intersect_point_rect(mouse_x, mouse_y, button->x, button->y, button->width, button->height);
	button->hovered = hovered;

	if ((buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) && hovered)
	{
		button->pressed = true;
		return true;
	}

	return false;
}