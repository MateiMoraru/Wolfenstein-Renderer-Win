/*
        RENDERER

        This is the literal engine

        Here, the rays from the RayCaster are turned into columns so that they are displayed to look like 3D

        Hell yeah
*/



#ifndef RENDERER_H
#define RENDERER_H

#include "Window.h"
#include "RayCaster.h"
#include "Player.h"
#include "Enemy.h"

#define HEIGHT_MULT 2.0f
#define COLOR_MULT 1.6f
#define MAX_SEE_DISTANCE 200
#define MAX_HEIGHT 250
#define MIN_DIST_MULT 0.0000f

#define MAX_SMOKE_PUFFS 64

extern const Color4 COLOR_BACKGROUND;
extern const Color4 COLOR_FLOOR;
extern const Color4 COLOR_WALL;
extern const Color4 COLOR_SEPARATOR;
extern const Color4 COLOR_DOOR_YELLOW;
extern const Color4 COLOR_DOOR_RED;
extern const Color4 COLOR_DOOR_GREEN;
extern const Color4 COLOR_DOOR_BLUE;


// Still developing this as i couldnt make it work
typedef struct Smoke_puff
{
    float x;
    float y;
    float age;
    float ttl;
    int alive;
    int ray_idx;
} Smoke_puff;


// Column struct so that the renderer_draw function looks decent
typedef struct
{
    Color4 color;
    int height;
    char hit_id;
} Column;

typedef struct
{
    Player* player;
    RayCaster* ray_caster;
    Window* window;

    Column pixels[NUMBER_RAYS];
} Renderer;

// Not used yet, probably will never use as its too much for me
void smoke_spawn(Smoke_puff* puffs, float x, float y);
void smoke_update(Smoke_puff* puffs, float dt);
float wrap_angle(float a);
float angle_diff(float a, float b);
void shoot_and_spawn_smoke(char** map, Player* player, Smoke_puff* puffs); 
void smoke_draw_all(Window* window, RayCaster* ray_caster, Smoke_puff* puffs, Sprite* smoke_sprite);

// Compute each column
Column compute_column(int max_height, Ray* ray);

// Basic functions
Renderer* renderer_init(Player* player, RayCaster* ray_caster, Window* window);
void renderer_update(Renderer* renderer);

// Draws the world as 3D
void renderer_draw(Renderer* renderer, Window* window, Enemy* enemy, Sprite* keys, Sprite* chest, Sprite* ammo, float y_offset);

#endif