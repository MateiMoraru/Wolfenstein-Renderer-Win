#ifndef RENDERER_H
#define RENDERER_H

#include "Window.h"
#include "RayCaster.h"
#include "Player.h"
#include "Enemy.h"

#define HEIGHT_MULT 7.0f
#define COLOR_MULT 5.0f
#define MAX_SEE_DISTANCE 200
#define MIN_DIST_MULT 0.001f

extern const Color4 COLOR_BACKGROUND;
extern const Color4 COLOR_WALL;
extern const Color4 COLOR_SEPARATOR;

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

Column compute_column(int max_height, Ray* ray);

Renderer* renderer_init(Player* player, RayCaster* ray_caster, Window* window);
void renderer_update(Renderer* renderer);
void renderer_draw(Renderer* renderer, Window* window, Enemy* enemy);

#endif