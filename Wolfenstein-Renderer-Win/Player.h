
#ifndef PLAYER_H
#define PLAYER_H

#include <math.h>
#include "RayCaster.h"

#define DIR_OFFSET 26.0f
#define ACCELERATION 50.0f
#define SPEED_MULT 2.0f

typedef struct
{
    float x;
    float y;

    float vx;
    float vy;

    float speed;
    float accel;
    float friction;

    float direction;
    float fov;
} Player;

void player_set_position(char** map, Player* player, RayCaster* ray_caster, float x, float y);

void player_move(char** map, Player* player, RayCaster* ray_caster, float forward, float strafe, float delta_time);
#endif