#include "Player.h"


void player_set_position(char** map, Player* player, RayCaster* ray_caster, float x, float y)
{
    map[(int)player->y][(int)player->x] = ' ';

    player->x = x;
    player->y = y;

    map[(int)player->y][(int)player->x] = ' ';

    ray_caster_move(ray_caster, x, y);
}

void player_move(char** map, Player* player, RayCaster* ray_caster, float forward, float strafe, float delta_time)
{
    float dir = DEG_TO_RAD(player->direction + DIR_OFFSET);

    float dx = cosf(dir) * forward - sinf(dir) * strafe * SPEED_MULT;
    float dy = sinf(dir) * forward + cosf(dir) * strafe * SPEED_MULT;

    player->vx += dx * ACCELERATION * delta_time;
    player->vy += dy * ACCELERATION * delta_time;

    player->vx *= 1.0f - ACCELERATION / 2 * delta_time;
    player->vy *= 1.0f - ACCELERATION / 2 * delta_time;

    float len = sqrtf(player->vx * player->vx + player->vy * player->vy);
    if (len > player->speed)
    {
        player->vx = player->vx / len * player->speed;
        player->vy = player->vy / len * player->speed;
    }

    float new_x = player->x + player->vx * delta_time;
    float new_y = player->y + player->vy * delta_time;

    if (map[(int)new_y][(int)new_x] == ' ')
    {
        player->x = new_x;
        player->y = new_y;
    }
    else
    {
        if (map[(int)player->y][(int)new_x] == ' ')
            player->x = new_x;
        if (map[(int)new_y][(int)player->x] == ' ')
            player->y = new_y;
    }

    ray_caster_set_position(ray_caster, player->x, player->y);
}