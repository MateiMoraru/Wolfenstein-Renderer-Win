/*
        PLAYER

        the player is not correlated to the grid:)
*/


#ifndef PLAYER_H
#define PLAYER_H

#include <math.h>
#include "RayCaster.h"

#define DIR_OFFSET 50
#define ACCELERATION 5.0f
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

    int ammo;

    float died;

    bool found_key_yellow;
    bool found_key_red;
    bool found_key_green;
    bool found_key_blue;
    int found_keys;

    bool end;

    bool seen_enemy;
} Player;

void player_set_position(char** map, Player* player, RayCaster* ray_caster, float x, float y);

void player_move(char** map, Player* player, RayCaster* ray_caster, float forward, float strafe, float delta_time);

char player_check_keys(Player* player, char** map);

#endif