
#ifndef PLAYER_H
#define PLAYER_H

#include <math.h>
#include "RayCaster.h"

#define SENSITIVITY 190
#define DIR_OFFSET 19.0f

typedef struct
{
    float x;
    float y;

    float speed;

    float direction;
    float fov;
} Player;

void player_set_position(char** map, Player* player, RayCaster* ray_caster, float x, float y);

void player_move(char** map, Player* player, RayCaster* ray_caster, float dx, float dy);
#endif