#include "Player.h"


void player_set_position(char** map, Player* player, RayCaster* ray_caster, float x, float y)
{
    map[(int)player->y][(int)player->x] = ' ';

    player->x = x;
    player->y = y;

    map[(int)player->y][(int)player->x] = 'p';

    ray_caster_move(ray_caster, x, y);
}
void player_move(char** map, Player* player, RayCaster* ray_caster, float forward, float strafe)
{
    float dir_rad = DEG_TO_RAD(player->direction + DIR_OFFSET);

    float dx = cosf(dir_rad) * forward - sinf(dir_rad) * strafe;
    float dy = sinf(dir_rad) * forward + cosf(dir_rad) * strafe;

    float new_x = player->x + dx;
    float new_y = player->y + dy;

    if (map[(int)new_y][(int)new_x] != '#') {
        player->x = new_x;
        player->y = new_y;
    }
    else {
        if (map[(int)player->y][(int)new_x] != '#')
            player->x = new_x;
        if (map[(int)new_y][(int)player->x] != '#')
            player->y = new_y;
    }

    ray_caster_set_position(ray_caster, player->x, player->y);
}