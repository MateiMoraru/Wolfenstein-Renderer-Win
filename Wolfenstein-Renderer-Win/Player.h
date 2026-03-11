/*
        PLAYER

        the player is not correlated to the grid:)
*/


#ifndef PLAYER_H
#define PLAYER_H

#include <math.h>
#include "RayCaster.h"

#define DIR_OFFSET 45

#define ACCELERATION 5.0f
#define SPEED_MULT 2.0f
#define Y_OFFSET_ACCELERATION 47.0f


#define PLAYER_MAX_FOV 130
#define PLAYER_MIN_FOV 90
#define PLAYER_FOV_INCREASE_COOLDOWN 500.0f
#define PLAYER_FOV_INCREASE ((PLAYER_MAX_FOV - PLAYER_MIN_FOV) / PLAYER_FOV_INCREASE_COOLDOWN)
#define PLAYER_FOV_CHANGE_COOLDOWN_SEC 0.01f

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

    int hit_entity_x;
    int hit_entity_y;


    // For smoother movement
    float y_offset;
    float y_offset_velocity;

    float fov_increase_timer;
    float fov_increase;
} Player;

void player_init(Player* player, RayCaster* ray_caster, char** map, int keys);

void player_set_position(char** map, Player* player, RayCaster* ray_caster, float x, float y);

void player_move(char** map, Player* player, RayCaster* ray_caster, float forward, float strafe, float delta_time, bool moving);

char player_check_keys(Player* player, char** map);

#endif