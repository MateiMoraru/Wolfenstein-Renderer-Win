#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "Window.h"
#include "RayCaster.h"
#include "Player.h"
#include "Renderer.h"
#include "Text.h"
#include "Sound.h"
#include "Enemy.h"
#include "Menu.h"
#include "Door.h"


#define WIDTH -1
#define HEIGHT -1

#define MAP_WIDTH 128
#define MAP_HEIGHT 128

#define DEG_TO_RAD(angle) ((angle) * M_PI / 180.0f)

#define NUM_FOOTSTEPS 4

#define FONT_SIZE 16

#define ID_BUTTON_QUIT 1
#define ID_BUTTON_START 2

#define ID_STATE_MAINMENU 3
#define ID_STATE_INGAME 4

#define ID_DOOR_YELLOW 5

float footstep_timer = 0.0f;
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

void footstep_play_volume(float volume)
{
    int idx = rand() % NUM_FOOTSTEPS;
    sound_play_modify(&footstep_sounds[idx], volume);
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
        footstep_timer -= dt;
        if (footstep_timer <= 0.0f)
        {
            footstep_play();
            footstep_timer = footstep_interval;
        }
    }
    player_move(map, player, ray_caster, forward, strafe, dt); 
}


void map_load(char** map, const char* map_load_file, Enemy* enemy, Door* doors)
{
    FILE* fin = fopen(map_load_file, "r");
    if (!fin)
    {
        printf("Failed to open map file\n");
        return;
    }

    char buffer[1024];
    int y = 0;

    int door_index = 0;

    while (fgets(buffer, sizeof(buffer), fin) && y < MAP_HEIGHT)
    {
        int x = 0;
        while (buffer[x] != '\0' && buffer[x] != '\n' && x < MAP_WIDTH)
        {
            map[y][x] = buffer[x];
            if (map[y][x] == 'E')
            {
                enemy->map_x = x;
                enemy->map_y = y;
            }
            if (map[y][x] == '1')
            {
                doors[door_index++] = door_init(ID_DOOR_YELLOW, x, y);
            }
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

void sprite_draw_hud(Window* window, Sprite* sprite, float scale)
{
    if (!window || !sprite || !sprite->texture) return;

    int w = (int)(sprite->width * scale);
    int h = (int)(sprite->height * scale);

    SDL_Rect dst;

    dst.x = (window->width - w) / 2;

    dst.y = window->height - h;

    dst.w = w;
    dst.h = h;

    SDL_RenderCopy(window->renderer, sprite->texture, NULL, &dst);
}

void player_init(Player* player, RayCaster* ray_caster, char** map)
{
    *player = (Player){
        .x = 1,
        .y = 1,
        .vx = 0,
        .vy = 0,
        .speed = 15.0f,
        .accel = 100,
        .direction = 45,
        .fov = 50,
        .ammo = 13,
        .died = false,
        .found_key_yellow = false
    };



    ray_caster_init(player->x, player->y, player->direction, player->fov, ray_caster);

    player_set_position(map, player, ray_caster, 4, 4);

    ray_caster_set_position(ray_caster, player->x, player->y);
}


int main()
{
    srand(time(NULL));

    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
        printf("Failed to init SDL: %s\n", SDL_GetError());
        return 1;
    }

    sound_init();

    Window* window = window_create(-1, -1, "Domm");
    SDL_SetRelativeMouseMode(SDL_TRUE);


    const char* map_load_file = "assets/data/map.txt";

    char** map;

    map = map_init();

    Door doors[MAX_DOORS];

    Sprite sprite_empty = sprite_load(window->renderer, "assets/sprites/empty.him", 11, 20);
    Sprite sprite_enemy_ghost_hit = sprite_load(window->renderer, "assets/sprites/enemy_hit.him", 11, 20);
    Sprite sprite_enemy_ghost = sprite_load(window->renderer, "assets/sprites/enemy.him", 11, 20);

    Enemy enemy_ghost = enemy_init(window, "assets/sprites/enemy.him", 101, 11, 20);

    map_load(map, map_load_file, &enemy_ghost, doors);

    Player player;
    RayCaster ray_caster;

    player_init(&player, &ray_caster, map);

    Mouse mouse = { 0 };

    mouse.x = WIDTH / 2;
    mouse.y = HEIGHT / 2;

    Font font = {
        .width = 8,
        .height = 8,
        .data = (const uint8_t(*)[8]) font8x8_basic
    };

    font_init(window->renderer, &font, font8x8_basic, FONT_SIZE / 8, (SDL_Color) { 138, 22, 4, 255 });


    Renderer* renderer = renderer_init(&player, &ray_caster, window);

    SDL_Texture* map_texture = map_build_texture(window, map);

    char fps_buffer[64];
    
    float rotation_velocity = 0.0f;
    float rotation_smoothing = 32.0f;

    footstep_init();

    Sound sfx_background_music = sound_load("assets/sfx/background_music.wav");
    sound_play_loop(&sfx_background_music);
    Sound sfx_gun_explode = sound_load("assets/sfx/gun_fire.wav");
    Sound sfx_die = sound_load("assets/sfx/enemy_die.wav");
    Sound sfx_step = sound_load("assets/sfx/step.wav");
    Sound sfx_player_die = sound_load("assets/sfx/die.wav");
    Sound sfx_unlock = sound_load("assets/sfx/unlock.wav");

    window_set_fps(window, 160);

    Sprite gun_model = sprite_load(window->renderer, "assets/sprites/gun_model.him", window->width / 2, window->height / 2);
    Sprite gun_shoot = sprite_load(window->renderer, "assets/sprites/gun_explode.him", window->width / 2 - 128, window->height / 2 + 200);
    Sprite gun_smoke = sprite_load(window->renderer, "assets/sprites/gun_smoke.him", 0, 0);

    float gun_timer = 2;
    bool shoot = false;

    int middle_ray = NUMBER_RAYS / 2;

    float enemy_move_timer = 0;

    Menu main_menu = main_menu_init(window, &font, ID_BUTTON_QUIT, ID_BUTTON_START);

    int state = ID_STATE_MAINMENU;

    float background_music_timer = 0;
    float display_message_timer = 0;
    
    char message_buffer[64] = "Find all the keys in order to escape.";

    while (window->running)
    {
        background_music_timer += window->delta_time;

        if (background_music_timer >= 37)
        {
            background_music_timer = 0;
            sound_close(&sfx_background_music);
            sound_play_modify(&sfx_background_music, 1.0f);

        }

        if (state == ID_STATE_INGAME)
        {
            handle_mouse(&mouse);
            SDL_SetRelativeMouseMode(SDL_TRUE);
        }
        else
        {

            SDL_SetRelativeMouseMode(SDL_FALSE);
        }

        shoot = false;

     
        while (window_poll_event(window))
        {
            if (window->event.type == WINDOW_QUIT)
            {
                window_quit(window);
            }

            if (window_get_key(window) == SDLK_ESCAPE)
            {
                state = ID_STATE_MAINMENU;

            }
            else if (state == ID_STATE_INGAME && window->event.type == SDL_MOUSEBUTTONDOWN)
            {
                if (window->event.button.button == SDL_BUTTON_LEFT && gun_timer > 1.5f && player.ammo > 0) // 1.5 sec delay? hmmm
                {
                    player.ammo--;
                    shoot = true;
                    gun_timer = 0.0f;

                    sound_play_modify(&sfx_gun_explode, .7f);
                    //shoot_and_spawn_smoke(map, &ray_caster, smoke_puffs);
                }
            }

        }

        if (state == ID_STATE_INGAME)
        {
            enemy_move_timer += window->delta_time;

            if (enemy_ghost.active && enemy_move_timer >= .4f)
            {
                float dx = player.x - enemy_ghost.map_x;
                float dy = player.y - enemy_ghost.map_y;

                float dist = dist_sq(player.x, player.y, enemy_ghost.map_x, enemy_ghost.map_y);

                float rolloff = 0.02f;
                float volume_target = 1.0f / (1.0f + rolloff * dist);

                float min_audible = 0.001f;
                if (volume_target < min_audible) volume_target = 0.0f;
                if (volume_target > 1.0f) volume_target = 1.0f;

                volume_target *= 2;

                footstep_play_volume(volume_target);
                enemy_update(&enemy_ghost, window->delta_time, map, MAP_HEIGHT, MAP_WIDTH, player.x, player.y);
                enemy_move_timer = 0;
            }

            gun_timer += window->delta_time;

            handle_keys(map, &player, &ray_caster, window->delta_time);
            ray_caster_set_position(&ray_caster, player.x, player.y);

            // F this gotta use comments
            // Here it checks if player got key this frame

            if (player_check_keys(&player, map) == 'Y' && !player.found_key_yellow)
            {
                sound_play_modify(&sfx_unlock, 0.7f);
                player.found_key_yellow = true;
                doors_unlock(ID_DOOR_YELLOW, map, doors);

                display_message_timer = 0;
                snprintf(message_buffer, 64, "You got the yellow key! (1/4)");
            }

            float target_rotation = mouse.dx * SENSITIVITY * window->delta_time;

            rotation_velocity += (target_rotation - rotation_velocity) * rotation_smoothing * window->delta_time;

            float rotation_step = rotation_velocity * window->delta_time;

            player.direction += rotation_step;
            ray_caster_rotate(&ray_caster, rotation_step);

            renderer_update(renderer);
            //smoke_update(smoke_puffs, window->delta_time);

            //for (int i = 0; i < NUMBER_RAYS; i++) {
            //    renderer->ray_caster->rays[i].len = ray_hits_wall(map, &renderer->ray_caster->rays[i]);
            //}
        }

        window_set_draw_color(window, color_to_hex(&COLOR_BACKGROUND));
        window_clear(window);

        if (state == ID_STATE_INGAME)
        {
            renderer_draw(renderer, window, &enemy_ghost);

            if (enemy_ghost.active)
            {
                if (dist_sq(enemy_ghost.map_x, enemy_ghost.map_y, player.x, player.y) < 1.41f)
                {
                    state = ID_STATE_MAINMENU;
                    player.died = true;
                    sound_play_modify(&sfx_player_die, 0.8f);
                }

                if (ray_caster.rays[middle_ray].hit_enemy == 'E' && gun_timer < 0.3f)
                {
                    enemy_set_sprite(&enemy_ghost, &sprite_enemy_ghost_hit);

                    if (shoot)
                        enemy_ghost.hits++;
                }
                else
                {
                    enemy_set_sprite(&enemy_ghost, &sprite_enemy_ghost);
                }
            }
            if (enemy_ghost.hits >= 3)
            {

                if (gun_timer > 0.3f && enemy_ghost.active)
                {
                    sound_play_modify(&sfx_die, 1.0f);
                    enemy_ghost.active = false;
                    enemy_set_sprite(&enemy_ghost, &sprite_empty);
                }
            }

            draw_map(map_texture, window, window->width - MAP_WIDTH * SCALE, window->height - MAP_HEIGHT * SCALE);

            draw_rays(window, &ray_caster.rays, map, window->width - MAP_WIDTH * SCALE, window->height - MAP_HEIGHT * SCALE);

            //smoke_draw_all(window, &ray_caster, smoke_puffs, &gun_smoke);

            sprintf_s(fps_buffer, sizeof(fps_buffer), "FPS: %d", window->FPS);
            text_draw(window->renderer, &font, 10, 10, fps_buffer, 2, (SDL_Color) { 255, 255, 255, 255 });

            sprintf_s(fps_buffer, sizeof(fps_buffer), "AMMO: %d", player.ammo);
            text_draw_shadow(window->renderer, &font, window->width / 2 - strlen(fps_buffer) * FONT_SIZE * 8 / 2, window->height - FONT_SIZE, fps_buffer, 1, (SDL_Color) { 255, 255, 255, 255 });

            display_message_timer += window->delta_time;

            if (display_message_timer < 2.0f)
            {

                text_draw_shadow(window->renderer, &font, 0, window->height - FONT_SIZE, message_buffer, 1, (SDL_Color) { 255, 255, 255, 255 });
            }

            if (gun_timer < 0.5f)
            {
                sprite_draw(window, &gun_shoot, 15);
            }

            sprite_draw_hud(window, &gun_model, 15);
        }
        else
        {
            int button = menu_draw(&main_menu);

            if (button == ID_BUTTON_QUIT) window->running = false;
            if (button == ID_BUTTON_START) 
            {
                if (player.died == true)
                {
                    player_init(&player, &ray_caster, map);
                }
                state = ID_STATE_INGAME;
            }
        }

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