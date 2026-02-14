#ifndef RAYCASTER_H
#define RAYCASTER_H

#include "Window.h"

#define NUMBER_RAYS 1200
#define RAY_LEN 400

#define SENSITIVITY 190

#define SCALE 2

#define M_PI 3.14156

#ifndef DEG_TO_RAD
#define DEG_TO_RAD(angle) ((angle) * M_PI / 180.0f)
#endif

#ifndef MAP_WIDTH
#define MAP_WIDTH 128
#define MAP_HEIGHT 128
#endif

#define RAY_DISPLAY_INTERVAL 10

typedef struct
{
    float x;
    float y;
    float angle;
    float slope;

    float len;
    // y -y0 =slope(x - x0) + y0
    // f(x) = slope(x - this->x) + y

    bool collided;

    char hit_id;
} Ray;

typedef struct
{
    float x;
    float y;
    float angle;
    float fov;
    Ray rays[NUMBER_RAYS];
} RayCaster;

void ray_caster_init(int x, int y, int direction, int fov, RayCaster* ray_caster);

void ray_caster_set_position(RayCaster* ray_caster, float x, float y);
void ray_caster_move(RayCaster* ray_caster, float x, float y);

void ray_increase_angle(Ray* ray, float angle);
void ray_caster_rotate(RayCaster* ray_caster, float dx);

void draw_ray(Window* window, Ray* ray, char** map, float distance);
void draw_rays(Window* window, Ray* rays, char** map, int x, int y);

float ray_hits_wall(char** map, Ray* ray);

#endif