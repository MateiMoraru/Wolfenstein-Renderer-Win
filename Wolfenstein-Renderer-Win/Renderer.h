#ifndef RENDERER_H
#define RENDERER_H

#include "Window.h"
#include "RayCaster.h"
#include "Player.h"
#include "Enemy.h"

#define HEIGHT_MULT 4.0f
#define COLOR_MULT 5.0f
#define MAX_SEE_DISTANCE 200
#define MIN_DIST_MULT 0.001f

#define MAX_SMOKE_PUFFS 64

extern const Color4 COLOR_BACKGROUND;
extern const Color4 COLOR_WALL;
extern const Color4 COLOR_SEPARATOR;

typedef struct Smoke_puff
{
    float x;
    float y;
    float age;
    float ttl;
    int alive;
    int ray_idx;
} Smoke_puff;

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

void smoke_spawn(Smoke_puff* puffs, float x, float y);
void smoke_update(Smoke_puff* puffs, float dt);
float wrap_angle(float a);
float angle_diff(float a, float b);
void shoot_and_spawn_smoke(char** map, Player* player, Smoke_puff* puffs); 
void smoke_draw_all(Window* window, RayCaster* ray_caster, Smoke_puff* puffs, Sprite* smoke_sprite);

Column compute_column(int max_height, Ray* ray);

Renderer* renderer_init(Player* player, RayCaster* ray_caster, Window* window);
void renderer_update(Renderer* renderer);
void renderer_draw(Renderer* renderer, Window* window, Enemy* enemy);

#endif