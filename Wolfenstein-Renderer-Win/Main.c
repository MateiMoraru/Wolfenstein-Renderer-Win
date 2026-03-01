/*
    A copy of the Wolfenstein renderer

    - A* pathfinding
    - Ray casting
    - 3D


    Keys in the map.txt:
        0->4       = LOCKED WALL
        Y, R, G, B = KEY


*/

/*
    TODO: Add ammo
*/



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

#define TARGET_FPS 0

#define WIDTH -1
#define HEIGHT -1

#define MAP_WIDTH 128
#define MAP_HEIGHT 128

#define DEG_TO_RAD(angle) ((angle) * M_PI / 180.0f)

#define NUMBER_FOOTSTEPS 4
#define NUMBER_SOUNDS 16

#define FONT_SIZE 16

#define ID_BUTTON_QUIT 1
#define ID_BUTTON_START 2

#define ID_STATE_MAINMENU 1
#define ID_STATE_INGAME 2
#define ID_STATE_END 3

#define ID_DOOR_YELLOW 1
#define ID_DOOR_RED 2
#define ID_DOOR_GREEN 3
#define ID_DOOR_BLUE 4

#define BUFFER_LEN 128

#define MIN_DISTANCE_DEATH 3.0f
#define MIN_DISTANCE_ENEMY_FOLLOW 3700

#define DOOR_OPEN_SFX_COOLDOWN 3.0f
#define SHOOT_COOLDOWN 1.5f

#define JUMPSCARE_COOLDOWN 1.0f

typedef struct RectScare
{
    int x, y;
    int w, h;
    bool active;
    float timer;
    SDL_Color color;
} RectScare;

// Display message buffers
char* buffer_start = "Look for all the keys and look for the chest!";
char* buffer_found_key_yellow = "Yellow key found! 3 More to go.";
char* buffer_found_key_red = "Red key found! 2 More to go.";
char* buffer_found_key_green = "Green key found! 1 More to go.";
char* buffer_found_key_blue = "Finally done! Now look for the treasure";
char* buffer_no_bullets = "Uh oh!!!!!! No bullets? Try to avoid the ghost:).";

// Enemy spawn position
int enemy_spawn_pos[4][2] = {
    {32, 20},
    {57, 42},                   // Second spawn
    {73, 58},
    {96, 16}
};

// Self explainatory
float footstep_timer = 0.0f;
float footstep_interval = 0.8f;

static Sound footstep_sounds[NUMBER_FOOTSTEPS];
static Sound all_sounds[NUMBER_SOUNDS];


// Simplest random float function
float random_float(float min, float max)
{
    return min + (max - min) * (rand() / (float)RAND_MAX);
}

// Init for the footsteps sounds
void footstep_init()
{
    footstep_sounds[0] = sound_load("assets/sfx/footstep.wav");
    footstep_sounds[1] = sound_load("assets/sfx/footstep2.wav");
    footstep_sounds[2] = sound_load("assets/sfx/footstep3.wav");
    footstep_sounds[3] = sound_load("assets/sfx/footstep4.wav");
}

// Play a random footstep at a set volume
void footstep_play_volume(float volume)
{
    int idx = rand() % NUMBER_FOOTSTEPS;
    sound_play_modify(&footstep_sounds[idx], volume);
}

// Play a random footstep at a random volume
void footstep_play()
{
    float vol = random_float(0.7f, 0.9f);
    footstep_play_volume(vol);
}

// Destroy func
void footstep_free()
{
    for (int i = 0; i < NUMBER_FOOTSTEPS; i++)
        sound_free(&footstep_sounds[i]);
    sound_close();
}

void sounds_free()
{
    footstep_free();

    for (int i = 0; i < NUMBER_SOUNDS; i++)
    {
        sounds_free(&all_sounds[i]);
    }
}

// For calculating delta X basically
void handle_mouse(Mouse* mouse)
{
    int xrel, yrel;
    mouse->button = SDL_GetRelativeMouseState(&xrel, &yrel);
    mouse->dx = xrel;
    mouse->dy = yrel;
}

// Handle the keys related to the player movement
void handle_keys(char** map, Player* player, RayCaster* ray_caster, float dt)
{
    const Uint8* keys = SDL_GetKeyboardState(NULL);

    float forward = 0.0f;
    float strafe = 0.0f;

    float mult = 3.0f;
    if (keys[SDL_SCANCODE_LSHIFT])
    {
        mult = 5.0f;
        footstep_interval = 0.4f;
    }
    else
    {
        footstep_interval = 0.8f;
    }

    if (keys[SDL_SCANCODE_W]) forward += mult;
    if (keys[SDL_SCANCODE_S]) forward -= mult;
    if (keys[SDL_SCANCODE_A]) strafe -= mult;
    if (keys[SDL_SCANCODE_D]) strafe += mult;

    if (forward != 0.0f || strafe != 0.0f)
    {
        footstep_timer += dt;
        if (footstep_timer > footstep_interval)
        {
            footstep_play();
            footstep_timer = 0;
        }
    }
    player_move(map, player, ray_caster, forward, strafe, dt); 
}

// Initialize map
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

// ***** Never used
// Put a pixel in the map
void map_put(char** map, int x, int y, char c)
{
    if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT)
        return;
    map[y][x] = c;
}

// Put a rectangle in the map
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
// *****

// Draw the map texture
void draw_map(SDL_Texture* map_tex, Window* window, int sx, int sy)
{
    SDL_Rect dst = { sx, sy, MAP_WIDTH * SCALE, MAP_HEIGHT * SCALE };
    SDL_RenderCopy(window->renderer, map_tex, NULL, &dst);
}

// Load the map from a file
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
            else if (map[y][x] >= '1' && map[y][x] <= '4')
            {
                if (door_index < MAX_DOORS)
                {
                    int id = map[y][x] - '0';
                    doors[door_index++] = door_init(id, x, y);
                }
                else
                {
                    printf("Too many doors in map (max=%d). Increase MAX_DOORS or reduce doors.\n", door_index);
                }
            }

            x++;
        }

        while (x < MAP_WIDTH) { map[y][x] = ' '; x++; }
        y++;
    }

    fclose(fin);
}

// ***** Never used
// Save the map to a file
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

// For drawing to the map by mouse
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
// *****

// Creates the map texture :)
SDL_Texture* map_build_texture(Window* window, char** map)
{
    int tex_w = MAP_WIDTH * SCALE;
    int tex_h = MAP_HEIGHT * SCALE;

    SDL_Texture* tex = SDL_CreateTexture(window->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, tex_w, tex_h);
    if (!tex) return NULL;

    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);

    SDL_SetRenderTarget(window->renderer, tex);

    SDL_SetRenderDrawColor(window->renderer, 0, 0, 0, 0);
    SDL_RenderClear(window->renderer);

    SDL_Rect rect = { 0, 0, SCALE, SCALE };

    for (int y = 0; y < MAP_HEIGHT; y++)
    {
        for (int x = 0; x < MAP_WIDTH; x++)
        {
            int r, g, b;
            char c = map[y][x];

            if (c == ' ')
            {
                r = 18; 
                g = 0; 
                b = 61;
            }
            else if (c == '1')
            {
                r = COLOR_DOOR_YELLOW.r; 
                g = COLOR_DOOR_YELLOW.g; 
                b = COLOR_DOOR_YELLOW.b;
            }
            else if (c == '2')
            {
                r = COLOR_DOOR_RED.r;
                g = COLOR_DOOR_RED.g; 
                b = COLOR_DOOR_RED.b;
            }
            else if (c == '3')
            {
                r = COLOR_DOOR_GREEN.r; 
                g = COLOR_DOOR_GREEN.g; 
                b = COLOR_DOOR_GREEN.b;
            }
            else if (c == '4')
            {
                r = COLOR_DOOR_BLUE.r; 
                g = COLOR_DOOR_BLUE.g; 
                b = COLOR_DOOR_BLUE.b;
            }
            else if (c == 'C')
            {
                r = 140;
                g = 54;
                b = 6;
            }
            else
            {
                r = 81;
                g = 112; 
                b = 156;
            }

            SDL_SetRenderDrawColor(window->renderer, r, g, b, 255);

            rect.x = x * SCALE;
            rect.y = y * SCALE;
            SDL_RenderFillRect(window->renderer, &rect);
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

// Player init, also used for restart
//      Resets the raycaster as well :)
void player_init(Player* player, RayCaster* ray_caster, char** map, int keys)
{
    *player = (Player){
        .x = 1,
        .y = 1,
        .vx = 0,
        .vy = 0,
        .speed = 7.0f,
        .accel = 100,
        .direction = 45,
        .fov = 100,
        .ammo = 13,
        .died = false,
        .found_key_yellow = false,
        .found_key_red = false,
        .found_key_green = false,
        .found_key_blue = false,
        .found_keys = keys,
        .end = false,
        .seen_enemy = false,
        .hit_entity_x = -1,
        .hit_entity_y = -1
    };

    if (keys >= 1)
        player->found_key_yellow = true;
    if (keys >= 2)
        player->found_key_red = true;
    if (keys >= 3)
        player->found_key_green = true;
    if (keys >= 4)
        player->found_key_blue = true;

    ray_caster_init(player->x, player->y, player->direction, player->fov, ray_caster);

    player_set_position(map, player, ray_caster, 4, 4);

    ray_caster_set_position(ray_caster, player->x, player->y);
}


// ***** Enemy handling

// Update the enemy, doesnt check for death
void update_enemy(Window* window, Enemy* enemy_ghost, Player* player, float* enemy_move_timer, char** map)
{
    if (enemy_ghost->active && *enemy_move_timer >= ENEMY_MOVE_COOLDOWN)
    {
        float dx = player->x - enemy_ghost->map_x;
        float dy = player->y - enemy_ghost->map_y;

        float dist = dist_sq(player->x, player->y, enemy_ghost->map_x, enemy_ghost->map_y);

        if (dist < MIN_DISTANCE_ENEMY_FOLLOW)
        {
            float rolloff = 0.02f;
            float volume_target = 1.0f / (1.0f + rolloff * dist);

            float min_audible = 0.001f;
            if (volume_target < min_audible) volume_target = 0.0f;
            if (volume_target > 1.0f) volume_target = 1.0f;

            volume_target *= 2;

            footstep_play_volume(volume_target);
            enemy_update(enemy_ghost, window->delta_time, map, MAP_HEIGHT, MAP_WIDTH, player->x, player->y);
            *enemy_move_timer = 0;
        }
    }
}

// Checks for player player death
// Also, changes the sprite of the enemy
void handle_enemy(Enemy* enemy_ghost, RayCaster* ray_caster, Player* player, Sound* sfx_player_die, int* state, int middle_ray, float* gun_timer, bool shoot, Sprite* sprite_enemy_ghost_hit, Sprite* sprite_enemy_ghost)
{
    enemy_ghost->dist_to_player = dist_sq(enemy_ghost->map_x, enemy_ghost->map_y, player->x, player->y);

    if (enemy_ghost->dist_to_player < MIN_GHOST_ACTIVE_DISTANCE)
    {
        enemy_ghost->active = true;
    }
    else
    {
        enemy_ghost->active = false;
    }
        
    if (enemy_ghost->active)
    {
        if (enemy_ghost->dist_to_player < MIN_DISTANCE_DEATH)
        {
            *state = ID_STATE_MAINMENU;
            player->died = true;
            sound_play_modify(sfx_player_die, 0.8f);
        }

        if (ray_caster->rays[middle_ray].hit_entity == 'E' && *gun_timer < 0.3f)
        {
            enemy_set_sprite(enemy_ghost, sprite_enemy_ghost_hit);

            if (shoot)
                enemy_ghost->hits++;
        }
        else
        {
            enemy_set_sprite(enemy_ghost, sprite_enemy_ghost);
        }
    }

}

// Checks wether the enemy got killed
//      Empties its map location
//      Resets its path
//      Basically resets everything :)
//      Dk why i dont create anew one
void handle_enemy_die(Enemy* enemy_ghost, RayCaster* ray_caster, Player* player, Sound* sfx_die, char** map, float* gun_timer)
{
    if (!enemy_ghost->active) return;
    if (enemy_ghost->hits < 3) return;
    if (*gun_timer <= 0.3f) return;
    // Lag after the enemy dies.
    // Probable issue: this code keeps running after it dies continously :)

    sound_play_modify(sfx_die, 1.0f);

    map[enemy_ghost->map_y][enemy_ghost->map_x] = ' ';

    enemy_ghost->deaths++;
    enemy_ghost->hits = 0;

    enemy_ghost->path_len = 0;
    enemy_ghost->path_pos = 0;
    enemy_ghost->repath_accum = 0.0f;
    enemy_ghost->last_goal_cell = -1;
    enemy_ghost->drawn = false;

    for (int i = 0; i < ENEMY_MAX_PATH; i++) enemy_ghost->path[i] = -1;

    int spawn_idx = enemy_ghost->deaths - 1;
    if (spawn_idx < 0) spawn_idx = 0;
    if (spawn_idx > 2) spawn_idx = 2;
    spawn_idx++;

    enemy_ghost->map_x = enemy_spawn_pos[spawn_idx][0];
    enemy_ghost->map_y = enemy_spawn_pos[spawn_idx][1];

    map[enemy_ghost->map_y][enemy_ghost->map_x] = 'E';

    enemy_ghost->x = (float)enemy_ghost->map_x;
    enemy_ghost->y = (float)enemy_ghost->map_y;

    enemy_ghost->sprite.x = enemy_ghost->x;
    enemy_ghost->sprite.y = enemy_ghost->y;

    enemy_ghost->active = false;

    player->seen_enemy = false;

    for (int i = 0; i < NUMBER_RAYS; i++)
    {
        ray_caster->rays[i].hit_entity = ' ';
    }
}

// *****

// Checks whether the player reached a key
// Opens doors
void check_for_keys(Player* player, char**map, int* display_message_timer, char* buffer_message, Sound* sfx_unlock, Sound* sfx_door_open, Door* doors, float* door_open_sfx_timer)
{
    if (player->found_key_yellow || player->found_key_red || player->found_key_green || player->found_key_blue)
    {
        bool opened = false;

        if (player->found_key_yellow || player->found_key_red || player->found_key_green || player->found_key_blue)
        {

            if (player->found_key_yellow)
            {
                if (doors_unlock(ID_DOOR_YELLOW, map, doors, player))
                {
                    opened = opened || true;
                }
            }

            if (player->found_key_red)
            {
                if (doors_unlock(ID_DOOR_RED, map, doors, player))
                {
                    opened = true;
                }
            }

            if (player->found_key_green)
            {
                if (doors_unlock(ID_DOOR_GREEN, map, doors, player))
                {
                    opened = true;
                }
            }

            if (player->found_key_blue)
            {
                if (doors_unlock(ID_DOOR_BLUE, map, doors, player))
                {
                    opened = true;
                }
            }

            if (opened)
            {
                if (*door_open_sfx_timer > DOOR_OPEN_SFX_COOLDOWN)
                {
                    *door_open_sfx_timer = 0.0f;
                    sound_play_modify(sfx_door_open, 0.8f);
                }
            }
        }
    }
    
    char check = player_check_keys(player, map);

    if (check == 'Y' && !player->found_key_yellow)
    {
        player->found_keys++;
        sound_play_modify(sfx_unlock, 0.7f);
        *display_message_timer = 0;
        snprintf(buffer_message, 64, buffer_found_key_yellow);
        player->found_key_yellow = true;
    }
    else if (check == 'R' && !player->found_key_red)
    {
        player->found_keys++;
        sound_play_modify(sfx_unlock, 0.7f);
        *display_message_timer = 0;
        snprintf(buffer_message, 64, buffer_found_key_red);
        player->found_key_red = true;
    }
    else if (check == 'G' && !player->found_key_green)
    {
        player->found_keys++;
        sound_play_modify(sfx_unlock, 0.7f);
        *display_message_timer = 0;
        snprintf(buffer_message, 64, buffer_found_key_green);
        player->found_key_green = true;
    }
    else if (check == 'B' && !player->found_key_blue)
    {
        player->found_keys++;
        sound_play_modify(sfx_unlock, 0.7f);
        *display_message_timer = 0;
        snprintf(buffer_message, 64, buffer_found_key_blue);
        player->found_key_blue = true;
    }
}


void handle_renderer_rotation(Window* window, Renderer* renderer, Mouse* mouse, RayCaster* ray_caster, Player* player)
{
    float rotation_step = mouse->dx * SENSITIVITY;
    player->direction += rotation_step;
    ray_caster_rotate(ray_caster, rotation_step);
    renderer_update(renderer);
}

void draw_hud_text(Window* window, Font* font, int ammo, float* display_message_timer, char* buffer, char* buffer_message)
{
    sprintf_s(buffer, BUFFER_LEN, "FPS: %d", window->FPS);
    text_draw(window->renderer, font, 10, 10, buffer, 2, (SDL_Color) { 255, 255, 255, 255 });

    sprintf_s(buffer, BUFFER_LEN, "AMMO: %d", ammo);
    text_draw_shadow(window->renderer, font, window->width / 2 - 8 * FONT_SIZE * 8 / 2, window->height - FONT_SIZE, buffer, 1, (SDL_Color) { 105, 8, 20, 255 });

    // Handles the temporary display message
    *display_message_timer += window->delta_time;

    if (*display_message_timer < 5.0f)
    {
        text_draw_shadow(window->renderer, font, 0, window->height - FONT_SIZE, buffer_message, 1, (SDL_Color) { 255, 255, 255, 255 });
    }
}

void draw_hud(Window* window, Player* player, Sprite* keys, Sprite* gun_shoot, Sprite* gun_model, Sprite* sprite_crosshair, Sprite* sprite_crosshair_shoot, Sprite* sprite_ammo, float gun_timer)
{
    // Draws the smoke and fire from shooting
    if (gun_timer < 0.5f)
    {
        sprite_draw(window, gun_shoot, 15);
    }

    // Draws the gun
    sprite_draw_hud(window, gun_model, 15);

    // Draws the keys the player has
    if (player->found_keys > 0)
    {
        int width = 64;
        int start_x = window->width / 2 + window->width / 10;
        int start_y = window->height - width;

        for (int i = 0; i < player->found_keys; i++)
        {
            keys[i].x = start_x + width * i;
            keys[i].y = start_y;

            sprite_draw(window, &keys[i], width / keys[i].width);
        }
    }

    // Draws crosshair
    if (gun_timer > 0.4f)
        sprite_draw(window, sprite_crosshair, 2);
    else
        sprite_draw(window, sprite_crosshair_shoot, 2);  

    // Draws the ammo icon
    sprite_draw_pos(window, sprite_ammo, 2, window->width / 2 - 8 * FONT_SIZE * 8 / 2 - 40, window->height - FONT_SIZE * 2);
}

bool check_for_entity(Player* player, char** map, char* c)
{
    int x = player->x;
    int y = player->y;

    int range = 2;

    for (int i = x - range; i < x + range; i++)
    {
        for (int j = y - range; j < y + range; j++)
        {
            if (i < 0 || j < 0 || i >= MAP_WIDTH || j >= MAP_HEIGHT)
                continue;
            if (map[j][i] == c)
            {
                player->hit_entity_x = i;
                player->hit_entity_y = j;
                return true;
            }
        }
    }

    return false;
}

int main()
{
    const char* map_load_file = "assets/data/map.txt";
    
    // Srand, not like the wolfenstein lookup 
    srand(time(NULL));

    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
        printf("Failed to init SDL: %s\n", SDL_GetError());
        return 1;
    }

    sound_init();

    Window* window = window_create(-1, -1, "Doom");

    if (!window)
    {
        printf("Failed to create Window object :(\n");
        return 1;
    }
    window_set_fps(window, TARGET_FPS);
    SDL_SetRelativeMouseMode(SDL_TRUE);

    printf("Window created successfully (width, height: %d, %d)\n", window->width, window->height);

    char** map = map_init();

    if (!map)
    {
        printf("Failed to create map :(\n");
        return 1;
    }

    Door doors[MAX_DOORS] = { 0 };

    printf("\n\t\tLOADING TEXTURES\n");

    Sprite sprite_empty = sprite_load(window->renderer, "assets/sprites/empty.him", 11, 20);
    Sprite sprite_enemy_ghost_hit = sprite_load(window->renderer, "assets/sprites/enemy_hit.him", 11, 20);
    Sprite sprite_enemy_ghost = sprite_load(window->renderer, "assets/sprites/enemy.him", 11, 20);

    Sprite sprite_crosshair = sprite_load(window->renderer, "assets/sprites/crosshair.him", 11, 20);
    sprite_crosshair.x = window->width / 2;
    sprite_crosshair.y = window->height / 2;

    Sprite sprite_crosshair_shoot = sprite_load(window->renderer, "assets/sprites/crosshair_shoot.him", 11, 20);
    sprite_crosshair_shoot.x = window->width / 2;
    sprite_crosshair_shoot.y = window->height / 2;

    Sprite sprite_gun_model = sprite_load(window->renderer, "assets/sprites/gun_model.him", window->width / 2, window->height / 2);
    Sprite sprite_gun_shoot = sprite_load(window->renderer, "assets/sprites/gun_explode.him", window->width / 2 - 128, window->height / 2 + 200);
    Sprite sprite_gun_smoke = sprite_load(window->renderer, "assets/sprites/gun_smoke.him", 0, 0);

    Sprite sprite_chest = sprite_load(window->renderer, "assets/sprites/chest.him", 0, 0);

    Sprite sprite_ammo = sprite_load(window->renderer, "assets/sprites/ammo.him", 0, 0);

    Enemy enemy_ghost = enemy_init(window, "assets/sprites/enemy.him", 101, 11, 20);

    Sprite keys[4] =
    {
        sprite_load(window->renderer, "assets/sprites/key_yellow.him", 0, 0),
        sprite_load(window->renderer, "assets/sprites/key_red.him", 0, 0),
        sprite_load(window->renderer, "assets/sprites/key_green.him", 0, 0),
        sprite_load(window->renderer, "assets/sprites/key_blue.him", 0, 0)
    };

    printf("\n");

    // Loading map after initing everything needed

    map_load(map, map_load_file, &enemy_ghost, doors);

    Player player;
    RayCaster ray_caster;

    player_init(&player, &ray_caster, map, 0);

    printf("Player created\n");

    Mouse mouse = { 0 };

    mouse.x = WIDTH / 2;
    mouse.y = HEIGHT / 2;

    // I hate this font

    Font font = { 0 };
    font.width = 8;
    font.height = 8;
    font.data = (const uint8_t(*)[8])font8x8_basic;

    font_init(window->renderer, &font, font8x8_basic, FONT_SIZE / 8, (SDL_Color) { 255, 255, 255, 255 });

    printf("Font created\n");

    // Not the SDL_Renderer, its just from my renderer.h for drawing the "3D"
    Renderer* renderer = renderer_init(&player, &ray_caster, window);

    // Creating a map texture as drawing the map in a primitive manner everyframe is expensive
    SDL_Texture* map_texture = map_build_texture(window, map);
    
    float rotation_velocity = 0.0f;
    float rotation_smoothing = 64;


    // Sound section

    printf("\n\t\tLOADING SOUNDS\n");
    footstep_init();

    Sound sfx_background_music = sound_load("assets/sfx/background_music.wav");
    sound_play_loop(&sfx_background_music);
    Sound sfx_gun_explode = sound_load("assets/sfx/gun_fire.wav");
    Sound sfx_die = sound_load("assets/sfx/enemy_die.wav");
    Sound sfx_step = sound_load("assets/sfx/step.wav");
    Sound sfx_player_die = sound_load("assets/sfx/die.wav");
    Sound sfx_unlock = sound_load("assets/sfx/unlock.wav");
    Sound sfx_scare = sound_load("assets/sfx/scare.wav");
    Sound sfx_door_open = sound_load("assets/sfx/door_open.wav");
    Sound sfx_no_bullets = sound_load("assets/sfx/empty_gun.wav");
    Sound sfx_reload = sound_load("assets/sfx/reload.wav");

    printf("\n");

    bool shoot = false;
    bool displayed_no_bullets_message = false;

    int middle_ray = NUMBER_RAYS / 2;

    Menu main_menu = main_menu_init(window, &font, ID_BUTTON_QUIT, ID_BUTTON_START);
    Menu end_menu = main_menu_init(window, &font, ID_BUTTON_QUIT, ID_BUTTON_START);
    menu_add_text(&end_menu, "Thanks for playing!!!!!!!!!!!!!!!!!", 100, 100, (SDL_Color) { 230, 39, 25, 255}, &font, FONT_SIZE);
    menu_remove_button_at(&end_menu, 0);

    int state = ID_STATE_MAINMENU;

    float background_music_timer = 0;
    float display_message_timer = 0;
    float enemy_move_timer = 0;
    float gun_timer = 2;
    float door_open_sfx_timer = 10;

    char buffer[BUFFER_LEN];
    char buffer_message[BUFFER_LEN] = "HIHIHIHA";
    snprintf(buffer_message, 64, buffer_start);

    RectScare jumpscare_screen =
    {
        .x = 0,
        .y = 0,
        .w = window->width,
        .h = window->height,
        .active = false,
        .timer = 0,
        .color = (SDL_Color) {255, 255, 255, 255}
    };

    while (window->running)
    {

        // Havent implemented a good sound loop system so this will do
        background_music_timer += window->delta_time;

        if (background_music_timer >= 37)
        {
            background_music_timer = 0;
            sound_close(&sfx_background_music);
            sound_play_modify(&sfx_background_music, 1.0f);

        }

        // Self explainatory, right?

        if (state == ID_STATE_INGAME)
        {
            handle_mouse(&mouse);

            // Enables the mouse (like so it shows) 
            SDL_SetRelativeMouseMode(SDL_TRUE);
        }
        else
        {
            // Disables the mouse cursor
            SDL_SetRelativeMouseMode(SDL_FALSE);
        }

        shoot = false;

        if (player.end == true)
        {
            state = ID_STATE_END;
        }
     
        while (window_poll_event(window))
        {
            // This will almost never happen, good to have if game is not in focus
            if (window->event.type == WINDOW_QUIT)
            {
                window_quit(window);
            }


            else if (window_get_key(window) == SDLK_ESCAPE)
            {
                state = ID_STATE_MAINMENU;
            }

            // Shooting mechanics
            else if (state == ID_STATE_INGAME && window->event.type == SDL_MOUSEBUTTONDOWN)
            {
                if (window->event.button.button == SDL_BUTTON_LEFT && gun_timer > SHOOT_COOLDOWN) 
                {
                    if (player.ammo > 0)
                    {
                        player.ammo--;
                        shoot = true;
                        gun_timer = 0.0f;

                        sound_play_modify(&sfx_gun_explode, .4f);
                    }
                    else
                    {
                        sound_play_modify(&sfx_no_bullets, .4f);
                    }
                    //shoot_and_spawn_smoke(map, &ray_caster, smoke_puffs);
                }
            }

        }


        // Here starts the good part ig
        // Wow this looks better
        if (state == ID_STATE_INGAME)
        {
            enemy_move_timer += window->delta_time;
            gun_timer += window->delta_time;
            door_open_sfx_timer += window->delta_time;

            if (!displayed_no_bullets_message && player.ammo <= 0)
            {
                snprintf(buffer_message, 64, buffer_no_bullets);
                display_message_timer = 0;
                displayed_no_bullets_message = true;
            }

            ray_caster_cast_all(&ray_caster, map);

            update_enemy(window, &enemy_ghost, &player, &enemy_move_timer, map);

            handle_keys(map, &player, &ray_caster, window->delta_time);
            ray_caster_set_position(&ray_caster, player.x, player.y);

            // F this gotta use comments
            // Here it checks if player got any keys this frame (For now only checks for the yellow key)

            check_for_keys(&player, map, &display_message_timer, buffer_message, &sfx_unlock, &sfx_door_open, doors, &door_open_sfx_timer);

            handle_renderer_rotation(window, renderer, &mouse, &ray_caster, &player);

            // Checks whether the player is near the chest
            if (check_for_entity(&player, map, 'C'))
            {
                player.end = true;
            }

            // Checks wheteher the player is near ammo
            if (check_for_entity(&player, map, 'A'))
            {
                map[player.hit_entity_y][player.hit_entity_x] = ' ';
                player.ammo += 5;
                sound_play_modify(&sfx_reload, 0.7f);
            }

            // Checks whether the player has seen the enemy for the first time

            if (!player.seen_enemy)
            {
                if (ray_caster_hit_enemy(&ray_caster))
                {
                    sound_play(&sfx_scare);
                    player.seen_enemy = true;

                    jumpscare_screen.active = true;
                }
            }
        }

        // Starting rendering stuff
        window_set_draw_color(window, color_to_hex(&COLOR_BACKGROUND));
        window_clear(window);

        if (state == ID_STATE_INGAME)
        {
            SDL_SetRenderDrawColor(window->renderer, 0, 0, 0, 255);
            SDL_RenderFillRect(window->renderer, &(SDL_Rect) { 0, 0, window->width, 120 });
            renderer_draw(renderer, window, &enemy_ghost, keys, &sprite_chest, &sprite_ammo);

            handle_enemy(&enemy_ghost, &ray_caster, &player, &sfx_player_die, &state, middle_ray, &gun_timer, shoot, &sprite_enemy_ghost_hit, &sprite_enemy_ghost);
            handle_enemy_die(&enemy_ghost, &ray_caster, &player, &sfx_die, map, &gun_timer);

            // Minimap
            draw_map(map_texture, window, window->width - MAP_WIDTH * SCALE, window->height - MAP_HEIGHT * SCALE);

            draw_rays(window, &ray_caster.rays, map, window->width - MAP_WIDTH * SCALE, window->height - MAP_HEIGHT * SCALE);

            if (jumpscare_screen.active)
            {
                SDL_SetRenderDrawColor(window->renderer, jumpscare_screen.color.r, jumpscare_screen.color.g, jumpscare_screen.color.b, jumpscare_screen.color.a);
                SDL_RenderFillRect(window->renderer, &(SDL_Rect){jumpscare_screen.x, jumpscare_screen.y, jumpscare_screen.w, jumpscare_screen.h});

                jumpscare_screen.color.a -= JUMPSCARE_COOLDOWN * 255 * window->delta_time;

                jumpscare_screen.timer += window->delta_time;
                if (jumpscare_screen.timer > JUMPSCARE_COOLDOWN)
                {
                    jumpscare_screen.timer = 0;
                    jumpscare_screen.color.a = 255;
                    jumpscare_screen.active = false;
                }
            }

            draw_hud_text(window, &font, player.ammo, &display_message_timer, buffer, buffer_message);

            draw_hud(window, &player, keys, &sprite_gun_shoot, &sprite_gun_model, &sprite_crosshair, &sprite_crosshair_shoot, &sprite_ammo, gun_timer);
        }
        else if (state == ID_STATE_MAINMENU)
        {
            // Handling main menu
            int button = menu_draw(&main_menu);

            if (button == ID_BUTTON_QUIT) window->running = false;

            if (button == ID_BUTTON_START)
            {
                // In case the player died, restart the player, not resetting the enemy state
                if (player.died == true)
                {
                    player_init(&player, &ray_caster, map, player.found_keys);

                    sound_stop(&sfx_scare);

                    // Also respawns the enemy
                    int idx = enemy_ghost.deaths;
                    if (idx < 0) idx = 0;
                    if (idx > 3) idx = 3;
                    enemy_set_position(&enemy_ghost, map, enemy_spawn_pos[idx][0], enemy_spawn_pos[idx][1]);

                }
                state = ID_STATE_INGAME;
            }
        }
        else
        {
            int button = menu_draw(&end_menu);

            if (button == ID_BUTTON_QUIT) window->running = false;
        }

        window_show(window);

        window_delay_fps(window);
    }

    // Destroying everything :)

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