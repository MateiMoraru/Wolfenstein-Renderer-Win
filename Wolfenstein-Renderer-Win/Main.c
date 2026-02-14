#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "Window.h"
#include "RayCaster.h"
#include "Player.h"
#include "Renderer.h"
#include "Text.h"
#include "Sound.h"


#define WIDTH -1
#define HEIGHT -1

#define MAP_WIDTH 128
#define MAP_HEIGHT 128

#define DEG_TO_RAD(angle) ((angle) * M_PI / 180.0f)

#define NUM_FOOTSTEPS 4

static float footstep_timer = 0.0f;
float footstep_interval = 0.5f;

Sound footstep_sounds[NUM_FOOTSTEPS];

float random_float(float min, float max)
{
    return min + (max - min) * (rand() / (float)RAND_MAX);
}

void footstep_init()
{
    footstep_sounds[0] = sound_load("assets/sfx/footstep.wav");
    footstep_sounds[1] = sound_load("assets/sfx/footstep2.wav");
    footstep_sounds[2] = sound_load("assets/sfx/footstep3.wav");
    footstep_sounds[3] = sound_load("assets/sfx/footstep4.wav");
}

void footstep_play()
{
    int idx = rand() % NUM_FOOTSTEPS;
    float vol = random_float(0.7f, 0.9f);
    sound_play_modify(&footstep_sounds[idx], vol);
}

void footstep_free()
{
    for (int i = 0; i < NUM_FOOTSTEPS; i++)
        sound_free(&footstep_sounds[i]);
    sound_close();
}

char** map_init()
{
    char** map = malloc(MAP_HEIGHT * sizeof(char*));
    for (int y = 0; y < MAP_HEIGHT; y++)
    {
        map[y] = malloc(MAP_WIDTH * sizeof(char));
        if (!map[y])
        {
            printf("Failed to malloc map[%d]\n", y);
        }
    }

    if (!map)
    {
        printf("Failed to malloc map\n");
        return NULL;
    }

    for (int y = 0; y < MAP_HEIGHT; y++)
    {
        for (int x = 0; x < MAP_WIDTH; x++)
        {
            map[y][x] = ' ';
        }
    }

    return map;
}

void map_put(char** map, int x, int y, char c)
{
    if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT)
        return;
    map[y][x] = c;
}


void map_put_rect(char** map, int x, int y, int w, int h, char c)
{
    for (int dy = y; dy < y + h; dy++)
    {
        for (int dx = x; dx < x + w; dx++)
        {
            map_put(map, dx, dy, c);
        }
    }
}

void draw_map(SDL_Texture* map_tex, Window* window, int sx, int sy)
{
    SDL_Rect dst = { sx, sy, MAP_WIDTH * SCALE, MAP_HEIGHT * SCALE };
    SDL_RenderCopy(window->renderer, map_tex, NULL, &dst);
}

void handle_mouse(Mouse* mouse)
{
    int xrel, yrel;
    mouse->button = SDL_GetRelativeMouseState(&xrel, &yrel);
    mouse->dx = xrel;
    mouse->dy = yrel;
}

void handle_keys(char** map, Player* player, RayCaster* ray_caster, float dt)
{
    const Uint8* keys = SDL_GetKeyboardState(NULL);

    float forward = 0.0f;
    float strafe = 0.0f;

    float mult = 3.0f;
    if (keys[SDL_SCANCODE_LSHIFT])
    {
        mult = 5.0f;
        footstep_interval = 0.3f;
    }
    else
    {
        footstep_interval = 0.5f;
    }
    if (keys[SDL_SCANCODE_W]) forward += mult;
    if (keys[SDL_SCANCODE_S]) forward -= mult;
    if (keys[SDL_SCANCODE_A]) strafe -= mult;
    if (keys[SDL_SCANCODE_D]) strafe += mult;

    if (forward != 0.0f || strafe != 0.0f)
    {
        player_move(map, player, ray_caster, forward, strafe, dt);

        footstep_timer -= dt;
        if (footstep_timer <= 0.0f)
        {
            footstep_play();
            footstep_timer = footstep_interval;
        }
    }
}


void map_load(char** map, const char* map_load_file)
{
    FILE* fin = fopen(map_load_file, "r");
    if (!fin)
    {
        printf("Failed to open map file\n");
        return;
    }

    char buffer[1024];
    int y = 0;

    while (fgets(buffer, sizeof(buffer), fin) && y < MAP_HEIGHT)
    {
        int x = 0;
        while (buffer[x] != '\0' && buffer[x] != '\n' && x < MAP_WIDTH)
        {
            map[y][x] = buffer[x];
            x++;
        }

        while (x < MAP_WIDTH)
        {
            map[y][x] = ' ';
            x++;
        }

        y++;
    }

    fclose(fin);

}

void map_save(char** map, const char* filename)
{
    FILE* fout = fopen(filename, "w");
    if (!fout)
    {
        printf("Failed to open write file\n");
        return;
    }

    for (int y = 0; y < MAP_HEIGHT; y++)
    {
        for (int x = 0; x < MAP_WIDTH; x++)
        {
            fputc(map[y][x], fout);
        }

        fputc('\n', fout);
    }

    fclose(fout);
}

void map_draw_mouse(char** map, Mouse* mouse)
{
    if (mouse->button & SDL_BUTTON(SDL_BUTTON_LEFT))
    {
        printf("DSA\n");
        int x = mouse->x / SCALE;
        int y = mouse->y / SCALE;
        map[y][x] = '#';
    }
}

SDL_Texture* map_build_texture(Window* window, char** map)
{
    int tex_w = MAP_WIDTH * SCALE;
    int tex_h = MAP_HEIGHT * SCALE;

    SDL_Texture* tex = SDL_CreateTexture(window->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, tex_w, tex_h);

    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);

    SDL_SetRenderTarget(window->renderer, tex);

    SDL_SetRenderDrawColor(window->renderer, 0, 0, 0, 0);
    SDL_RenderClear(window->renderer);

    SDL_Rect r = { 0, 0, SCALE, SCALE };

    for (int y = 0; y < MAP_HEIGHT; y++)
    {
        for (int x = 0; x < MAP_WIDTH; x++)
        {

            if (map[y][x] == ' ')
                SDL_SetRenderDrawColor(window->renderer, 18, 0, 61, 255);
            else
                SDL_SetRenderDrawColor(window->renderer, 86, 112, 156, 255);


            r.x = x * SCALE;
            r.y = y * SCALE;
            SDL_RenderFillRect(window->renderer, &r);
        }
    }

    SDL_SetRenderTarget(window->renderer, NULL);

    return tex;
}

int main()
{
    srand(time(NULL));
    Window* window = window_create(-1, -1, "Domm");
    SDL_SetRelativeMouseMode(SDL_TRUE);

    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
        printf("Failed to init SDL: %s\n", SDL_GetError());
        return 1;
    }

    const char* map_load_file = "assets/data/map.txt";

    char** map;

    map = map_init();

    map_load(map, map_load_file);

    Player player = {
        .x = 63,
        .y = 18,
        .vx = 0,
        .vy = 0,
        .speed = 10.0f,
        .accel = 100,
        .direction = 45,
        .fov = 60
    };

    RayCaster ray_caster;

    ray_caster_init(player.x, player.y, player.direction, player.fov, &ray_caster);

    player_set_position(map, &player, &ray_caster, 2, 2);

    ray_caster_set_position(&ray_caster, player.x, player.y);

    Mouse mouse = { 0 };

    mouse.x = WIDTH / 2;
    mouse.y = HEIGHT / 2;

    Font font = {
        .width = 8,
        .height = 8,
        .data = (const uint8_t(*)[8]) font8x8_basic
    };


    Renderer* renderer = renderer_init(&player, &ray_caster, window);

    SDL_Texture* map_texture = map_build_texture(window, map);

    char fps_buffer[64];

    float rotation_velocity = 0.0f;
    float rotation_smoothing = 16.0f;

    footstep_init();

    Sound sfx_background_music = sound_load("assets/sfx/background_music.wav");
    sound_init(&sfx_background_music);
    sound_play_loop(&sfx_background_music);

    window_set_fps(window, 0);

    while (window->running)
    {
        //sound_update(&sfx_background_music);
        while (window_poll_event(window))
        {

            if (window->event.type == WINDOW_QUIT)
            {
                window_quit(window);
            }

            if (window_get_key(window) == SDLK_ESCAPE)
            {
                window_quit(window);
            }
            else if (window_get_key(window) == SDLK_1)
            {
                map_save(map, map_load_file);
            }

        }

        handle_keys(map, &player, &ray_caster, window->delta_time);
        ray_caster_set_position(&ray_caster, player.x, player.y);

        handle_mouse(&mouse);
        float target_rotation = mouse.dx * SENSITIVITY;

        rotation_velocity += (target_rotation - rotation_velocity) * rotation_smoothing * window->delta_time;

        float rotation_step = rotation_velocity * window->delta_time;

        player.direction += rotation_step;
        ray_caster_rotate(&ray_caster, rotation_step);

        renderer_update(renderer);

        //for (int i = 0; i < NUMBER_RAYS; i++) {
        //    renderer->ray_caster->rays[i].len = ray_hits_wall(map, &renderer->ray_caster->rays[i]);
        //}

        window_set_draw_color(window, color_to_hex(&COLOR_BACKGROUND));
        window_clear(window);

        renderer_draw(renderer, window);

        draw_map(map_texture, window, window->width - MAP_WIDTH * SCALE, window->height - MAP_HEIGHT * SCALE);

        draw_rays(window, &ray_caster.rays, map, window->width - MAP_WIDTH * SCALE, window->height - MAP_HEIGHT * SCALE);

        sprintf_s(fps_buffer, sizeof(fps_buffer), "FPS: %d", window->FPS);
        text_draw(window->renderer, &font, 10, 10, fps_buffer, 2, (SDL_Color) { 255, 255, 255, 255 });

        window_show(window);

        window_delay_fps(window);
    }

    for (int y = 0; y < MAP_HEIGHT; y++)
    {
        free(map[y]);
    }

    free(map);

    sound_free(&sfx_background_music);
    footstep_free();

    window_terminate(window);
    return 0;
}