#include "Player.h"

static const int directions[8][2] = {
    {-1, -1}, {0, -1}, {1, -1},
    {-1,  0},          {1,  0},
    {-1,  1}, {0,  1}, {1,  1}
};


// Player init, also used for restart
//      Resets the raycaster as well :)
void player_init(Player* player, RayCaster* ray_caster, char** map, int keys)
{
    *player = (Player){
        .x = 1,
        .y = 1,
        .vx = 0,
        .vy = 0,
        .speed = 7.0f,
        .accel = 100,
        .direction = 45,
        .fov = PLAYER_MIN_FOV,
        .ammo = 13,
        .died = false,
        .found_key_yellow = false,
        .found_key_red = false,
        .found_key_green = false,
        .found_key_blue = false,
        .found_keys = keys,
        .end = false,
        .seen_enemy = false,
        .hit_entity_x = -1,
        .hit_entity_y = -1,
        .y_offset = 0,
        .y_offset_velocity = 1,
        .fov_increase_timer = 0,
        .fov_increase = 0
    };

    if (keys >= 1)
        player->found_key_yellow = true;
    if (keys >= 2)
        player->found_key_red = true;
    if (keys >= 3)
        player->found_key_green = true;
    if (keys >= 4)
        player->found_key_blue = true;

    ray_caster_init(player->x, player->y, player->direction, player->fov, ray_caster);

    player_set_position(map, player, ray_caster, 4, 4);

    ray_caster_set_position(ray_caster, player->x, player->y);
}



void player_set_position(char** map, Player* player, RayCaster* ray_caster, float x, float y)
{
    map[(int)player->y][(int)player->x] = ' ';

    player->x = x;
    player->y = y;

    map[(int)player->y][(int)player->x] = ' ';

    ray_caster_move(ray_caster, x, y);
}

static bool is_solid_cell(char c)
{
    return c != ' ';
}

static int circle_hits_wall(char** map, float x, float y, float r)
{
    int min_x = (int)floorf(x - r);
    int max_x = (int)floorf(x + r);
    int min_y = (int)floorf(y - r);
    int max_y = (int)floorf(y + r);

    if (min_x < 0) min_x = 0;
    if (min_y < 0) min_y = 0;
    if (max_x >= MAP_WIDTH) max_x = MAP_WIDTH - 1;
    if (max_y >= MAP_HEIGHT) max_y = MAP_HEIGHT - 1;

    for (int ty = min_y; ty <= max_y; ty++)
    {
        for (int tx = min_x; tx <= max_x; tx++)
        {
            if (!is_solid_cell(map[ty][tx])) continue;

            float nearest_x = x;
            if (nearest_x < (float)tx) nearest_x = (float)tx;
            if (nearest_x > (float)(tx + 1)) nearest_x = (float)(tx + 1);

            float nearest_y = y;
            if (nearest_y < (float)ty) nearest_y = (float)ty;
            if (nearest_y > (float)(ty + 1)) nearest_y = (float)(ty + 1);

            float dx = x - nearest_x;
            float dy = y - nearest_y;

            if (dx * dx + dy * dy < r * r) return 1;
        }
    }

    return 0;
}

void player_handle_y(float move)
{

}

void player_move(char** map, Player* player, RayCaster* ray_caster, float forward, float strafe, float delta_time, bool running)
{
    // Handles all the directions and all that :)
    float dir = DEG_TO_RAD(player->direction + DIR_OFFSET);

    float ax = cosf(dir) * forward - sinf(dir) * strafe * SPEED_MULT;
    float ay = sinf(dir) * forward + cosf(dir) * strafe * SPEED_MULT;

    player->vx += ax * ACCELERATION * delta_time;
    player->vy += ay * ACCELERATION * delta_time;

    float damp = 1.0f - (ACCELERATION * 0.5f) * delta_time;
    if (damp < 0.0f) damp = 0.0f;
    player->vx *= damp;
    player->vy *= damp;

    float vlen = sqrtf(player->vx * player->vx + player->vy * player->vy);
    if (vlen > player->speed)
    {
        float s = player->speed / vlen;
        player->vx *= s;
        player->vy *= s;
    }

    float r = 0.25f;

    float target_x = player->x + player->vx * delta_time;
    float target_y = player->y + player->vy * delta_time;

    float new_x = player->x;
    float new_y = player->y;

    // Basic collision

    if (!circle_hits_wall(map, target_x, new_y, r)) new_x = target_x;
    if (!circle_hits_wall(map, new_x, target_y, r)) new_y = target_y;

    player->x = new_x;
    player->y = new_y;


    // Handles Y offset for smoother movement :)
    if (vlen > 0.1f)
    {
        float step = player->y_offset_velocity * Y_OFFSET_ACCELERATION * delta_time;

        if (running)
            step *= 2.0f;

        player->y_offset += step;

        // Handles the bounce ig
        if (player->y_offset > 10.0f)
        {
            player->y_offset = 10.0f;
            player->y_offset_velocity *= -1.0f;
        }
        else if (player->y_offset < -10.0f)
        {
            player->y_offset = -10.0f;
            player->y_offset_velocity *= -1.0f;
        }
    }
    else
    {
        player->y_offset = 0.0f;
    }

    // Increase the FOV of the player when running

    if (running)
    {
        player->fov_increase_timer += delta_time;

        if (player->fov <= PLAYER_MIN_FOV)
        {
            player->fov = PLAYER_MIN_FOV;
            player->fov_increase = 0;
        }
        else
        {
            player->fov_increase = ((player->fov - PLAYER_MIN_FOV) / PLAYER_FOV_INCREASE);
        }

        if (player->fov < PLAYER_MAX_FOV && player->fov_increase_timer > PLAYER_FOV_CHANGE_COOLDOWN_SEC / PLAYER_FOV_INCREASE_COOLDOWN)
        {
            player->fov_increase_timer = 0.0f;
            player->fov_increase++;
            player->fov = PLAYER_MIN_FOV + PLAYER_FOV_INCREASE * player->fov_increase;

            if (player->fov > PLAYER_MAX_FOV)
            {
                player->fov = PLAYER_MAX_FOV;
                player->fov_increase = PLAYER_FOV_INCREASE_COOLDOWN;
            }

            ray_caster_init(player->x, player->y, player->direction - ((PLAYER_FOV_INCREASE * player->fov_increase) / 2.0f), player->fov, ray_caster);
        }
    }
    else
    {
        player->fov_increase_timer += delta_time;

        if (player->fov >= PLAYER_MAX_FOV)
        {
            player->fov = PLAYER_MAX_FOV;
            player->fov_increase = PLAYER_FOV_INCREASE_COOLDOWN;
        }
        else
        {
            player->fov_increase = ((player->fov - PLAYER_MIN_FOV) / PLAYER_FOV_INCREASE);
        }

        if (player->fov > PLAYER_MIN_FOV && player->fov_increase_timer > PLAYER_FOV_CHANGE_COOLDOWN_SEC / PLAYER_FOV_INCREASE_COOLDOWN)
        {
            player->fov_increase_timer = 0.0f;
            player->fov_increase--;
            player->fov = PLAYER_MIN_FOV + PLAYER_FOV_INCREASE * player->fov_increase;

            if (player->fov < PLAYER_MIN_FOV)
            {
                player->fov = PLAYER_MIN_FOV;
                player->fov_increase = 0;
            }

            ray_caster_init(player->x, player->y, player->direction - ((PLAYER_FOV_INCREASE * player->fov_increase) / 2.0f), player->fov, ray_caster);
        }
    }

    // Updates the ray_caster position so theres no need to do it outside the player struct calls :)
    ray_caster_set_position(ray_caster, player->x, player->y);

    // Not needed if will decide to keep the FOV change
    //ray_caster_set_position(ray_caster, player->x, player->y);
}


char player_check_keys(Player* player, char** map)
{
    int x = player->x;
    int y = player->y;

    for (int i = 0; i < 8; i++)                                                             
    {
        int dx = x - directions[i][0];
        int dy = y - directions[i][1];

        if (dx < 0 && dy < 0 && dx > MAP_WIDTH && dy > MAP_HEIGHT)
            continue;

        char cell = map[dy][dx];
        if (cell == 'Y' || cell == 'R' || cell == 'G' || cell == 'B')
        {
            map[dy][dx] = ' ';
            return cell;
        }
    }
}

