/*
        RAYCASTER

        Used DDA >:(
*/


#ifndef RAYCASTER_H
#define RAYCASTER_H

#include "Window.h"

#define NUMBER_RAYS 300
#define RAY_LEN 512

#define SENSITIVITY 1 / 50

#define SCALE 3.0f

#define M_PI 3.14156

#ifndef DEG_TO_RAD
#define DEG_TO_RAD(angle) ((angle) * M_PI / 180.0f)
#endif

#ifndef MAP_WIDTH
#define MAP_WIDTH 128
#define MAP_HEIGHT 128
#endif

#define RAY_CHECK_SKIP_INTERVAL 3

#define RAY_DISPLAY_INTERVAL 5

#define MIN_ENEMY_SEE_DISTANCE 15

typedef struct
{
    // Kind of grid dependent?
    float x;
    float y;    
    float angle;

    float len;

    // Wall detection
    char  hit_id;
    int   hit_side;
    float hit_x;
    float hit_y;

    // Enemy detection
    // Key detection
    char  hit_entity;
    float hit_entity_distance;
    float hit_entity_x;
    float hit_entity_y;
    float hit_entity_u;

} Ray;

typedef struct
{
    float x;
    float y;
    float angle;
    float fov;
    Ray rays[NUMBER_RAYS];
} RayCaster;

// Creates rays :)

void ray_caster_init(int x, int y, int direction, int fov, RayCaster* ray_caster);

void ray_caster_set_position(RayCaster* ray_caster, float x, float y);
void ray_caster_move(RayCaster* ray_caster, float x, float y);

void ray_increase_angle(Ray* ray, float angle);
void ray_caster_rotate(RayCaster* ray_caster, float dx);

void draw_ray(Window* window, Ray* ray, char** map, float distance);
void draw_rays(Window* window, Ray* rays, char** map, int x, int y);

float ray_hits_wall(char** map, Ray* ray);
void ray_caster_cast_all(RayCaster* ray_caster, char** map);


// This function checks if the enemy is on screen
bool ray_caster_hit_enemy(RayCaster* ray_caster);


#endif