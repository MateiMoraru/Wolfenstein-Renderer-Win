#ifndef WINDOW_H
#define WINDOW_H

#include <SDL.h>
#include <stdbool.h>

#include "asset_drawer.h"

#define WINDOW_FULLSCREEN -1

#define WINDOW_QUIT SDL_QUIT

#define WINDOW_KEY_PRESSED SDL_KEYDOWN


typedef struct Window
{
    int width;
    int height;
    int x;
    int y;
    char* name;
    bool running;

    Color4 draw_color;

    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Event event;
    Uint32 last_time;
    Uint32 current_time;
    float delta_time;

    int FPS_counter;
    int FPS;
    float FPS_timer;

    float target_FPS;
    float frame_delay;
} Window;


typedef struct
{
    int x;
    int y;

    int dx;
    int dy;

    Uint32 button;

} Mouse;


Window* window_create(int width, int height, char* name);
void window_terminate(Window* window);
void window_quit(Window* window);

void window_clear(Window* window);
void window_show(Window* window);
void window_draw_rect(Window* window, SDL_Rect* rect);
void window_set_draw_color(Window* window, int hex);

bool window_poll_event(Window* window);
SDL_Event window_get_event(Window* window);
int window_get_key(Window* window);

void window_set_fps(Window* window, int fps);
void window_delay_fps(Window* window);

Color4 hex_to_int(int hex);
int color_to_hex(const Color4* color);

#endif