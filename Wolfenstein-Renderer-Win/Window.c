#include "Window.h"

Window* window_create(int width, int height, char* name)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return NULL;
    }

    Window* window = malloc(sizeof(Window));
    if (!window) {
        printf("Failed to malloc Window struct\n");
        SDL_Quit();
        return NULL;
    }

    bool fullscreen = false;

    if (width == WINDOW_FULLSCREEN)
    {
        fullscreen = true;
        SDL_DisplayMode DM;
        if (SDL_GetCurrentDisplayMode(0, &DM) != 0) {
            printf("SDL_GetCurrentDisplayMode failed: %s\n", SDL_GetError());
            return NULL;
        }
        width = DM.w;
        height = DM.h;
    }

    window->width = width;
    window->height = height;
    window->x = SDL_WINDOWPOS_CENTERED;
    window->y = SDL_WINDOWPOS_CENTERED;
    window->name = name;
    window->running = true;

    window->delta_time = 0;
    window->last_time = SDL_GetTicks();

    window->FPS = 0;
    window->FPS_counter = 0;
    window->FPS_timer = 0.0f;

    if (fullscreen)
        window->window = SDL_CreateWindow(name, window->x, window->y, window->width, window->height, SDL_WINDOW_FULLSCREEN_DESKTOP);
    else
        window->window = SDL_CreateWindow(name, window->x, window->y, window->width, window->height, SDL_WINDOW_SHOWN);

    if (!window->window) {
        printf("Failed to create SDL_Window: %s\n", SDL_GetError());
        free(window);
        SDL_Quit();
        return NULL;
    }

    window->renderer = SDL_CreateRenderer(window->window, -1, SDL_RENDERER_ACCELERATED);
    if (!window->renderer) {
        printf("Failed to create SDL_Renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window->window);
        free(window);
        SDL_Quit();
        return NULL;
    }

    return window;
}

void window_terminate(Window* window)
{
    SDL_DestroyRenderer(window->renderer);
    SDL_DestroyWindow(window->window);
    SDL_Quit();
    free(window);
}

void window_quit(Window* window)
{
    window->running = false;
}


void window_clear(Window* window)
{
    window->current_time = SDL_GetTicks();
    window->delta_time = (window->current_time - window->last_time) / 1000.0f;
    window->last_time = window->current_time;
    window->FPS_counter++;
    window->FPS_timer += window->delta_time;

    if (window->FPS_timer > 1.0f)
    {
        window->FPS = window->FPS_counter;
        window->FPS_counter = 0;
        window->FPS_timer = 0;
    }

    SDL_RenderClear(window->renderer);
}

void window_show(Window* window)
{
    SDL_RenderPresent(window->renderer);
}

void window_draw_rect(Window* window, SDL_Rect* rect)
{
    SDL_RenderDrawRect(window->renderer, rect);
}

void window_set_draw_color(Window* window, int hex)
{
    window->draw_color = hex_to_int(hex);
    SDL_SetRenderDrawColor(window->renderer, window->draw_color.r, window->draw_color.g, window->draw_color.b, window->draw_color.a);
}


bool window_poll_event(Window* window)
{
    return SDL_PollEvent(&window->event);
}

SDL_Event window_get_event(Window* window)
{
    return window->event;
}

int window_get_key(Window* window)
{
    if (window->event.type == SDL_KEYDOWN || window->event.type == SDL_KEYUP)
    {
        return window->event.key.keysym.sym;
    }
    return SDLK_UNKNOWN;
}

void window_set_fps(Window* window, int fps)
{
    window->target_FPS = fps;

    if (fps > 0)
        window->frame_delay = 1000.0f / fps;
    else
        window->frame_delay = 0;
}

void window_delay_fps(Window* window)
{
    if (window->target_FPS <= 0) return;

    Uint32 frame_time = SDL_GetTicks() - window->last_time;
    if (frame_time < window->frame_delay)
    {
        SDL_Delay(window->frame_delay - frame_time);
    }
}
Color4 hex_to_int(int hex)
{
    Color4 color;
    color.r = (hex >> 24) & 0xFF;
    color.g = (hex >> 16) & 0xFF;
    color.b = (hex >> 8) & 0xFF;
    color.a = (hex >> 0) & 0xFF;

    return color;
}

int color_to_hex(const Color4* color)
{
    return ((color->r << 24) | (color->g << 16) | (color->b << 8) | color->a);
}