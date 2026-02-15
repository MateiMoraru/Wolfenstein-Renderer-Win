#include "RayCaster.h"

void ray_caster_set_position(RayCaster* ray_caster, float x, float y)
{
    ray_caster->x = x;
    ray_caster->y = y;
    for (int i = 0; i < NUMBER_RAYS; i++)
    {
        ray_caster->rays[i].x = x;
        ray_caster->rays[i].y = y;
    }
}

void ray_caster_move(RayCaster* ray_caster, float x, float y)
{
    ray_caster_set_position(ray_caster, ray_caster->x + x, ray_caster->y + y);
}

void ray_caster_init(int x, int y, int direction, int fov, RayCaster* ray_caster)
{
    for (int i = 0; i < NUMBER_RAYS; i++)
    {
        float angle = DEG_TO_RAD(direction + ((float)i * fov / (float)NUMBER_RAYS));
        ray_caster->rays[i].x = x;
        ray_caster->rays[i].y = y;
        ray_caster->rays[i].angle = angle;
    }
}

void ray_increase_angle(Ray* ray, float angle)
{
    ray->angle += angle;
    if (ray->angle < 0) ray->angle += 2 * M_PI;
    if (ray->angle > 2 * M_PI) ray->angle -= 2 * M_PI;
}

void ray_caster_rotate(RayCaster* ray_caster, float dx)
{
    if (dx == 0) return;
    float a = dx * (M_PI / 180.0f);
    for (int i = 0; i < NUMBER_RAYS; i++)
        ray_increase_angle(&ray_caster->rays[i], a);
}

float ray_hits_wall(char** map, Ray* ray)
{
    ray->hit_enemy = ' ';
    ray->hit_enemy_x = -1;
    ray->hit_enemy_y = -1;

    float x = ray->x;
    float y = ray->y;

    float dx = cos(ray->angle);
    float dy = sin(ray->angle);

    int mapx = (int)x;
    int mapy = (int)y;

    float delta_dist_x = (dx == 0) ? 1e30f : fabs(1.0f / dx);
    float delta_dist_y = (dy == 0) ? 1e30f : fabs(1.0f / dy);

    int stepx = (dx < 0) ? -1 : 1;
    int stepy = (dy < 0) ? -1 : 1;

    float side_dist_x = (dx < 0) ? (x - mapx) * delta_dist_x : (mapx + 1 - x) * delta_dist_x;
    float side_dist_y = (dy < 0) ? (y - mapy) * delta_dist_y : (mapy + 1 - y) * delta_dist_y;

    int side = 0;

    bool hit = false;

    while (!hit)
    {
        if (side_dist_x < side_dist_y)
        {
            side_dist_x += delta_dist_x;
            mapx += stepx;
            side = 0;
        }
        else
        {
            side_dist_y += delta_dist_y;
            mapy += stepy;
            side = 1;
        }

        if (mapx < 0 || mapx >= MAP_WIDTH || mapy < 0 || mapy >= MAP_HEIGHT)
            return -1;
        if (map[mapy][mapx] == '#' || map[mapy][mapx] == '0')
        {
            hit = true;
            ray->hit_id = map[mapy][mapx];
        }
        else if (map[mapy][mapx] == 'E')
        {
            ray->hit_enemy = 'E';

            float hit_dist;
            if (side == 0)
                hit_dist = (mapx - x + (1 - stepx) / 2.0f) / dx;
            else
                hit_dist = (mapy - y + (1 - stepy) / 2.0f) / dy;

            ray->hit_enemy_distance = hit_dist;

            // world hit position (NOT tile index)
            float hit_world_x = x + dx * hit_dist;
            float hit_world_y = y + dy * hit_dist;

            ray->hit_enemy_x = hit_world_x;
            ray->hit_enemy_y = hit_world_y;
        }
    }
    if (side == 0) 
        return (mapx - x + (1 - stepx) / 2.0f) / dx;
    return (mapy - y + (1 - stepy) / 2.0f) / dy;
}

void draw_ray(Window* window, Ray* ray, char** map, int offset_x, int offset_y, float distance)
{
    if (distance < 0) distance = 0;
    float dx = distance * cos(ray->angle) * SCALE;
    float dy = distance * sin(ray->angle) * SCALE;
    float x0 = ray->x * SCALE;
    float y0 = ray->y * SCALE;
    float x1 = x0 + dx;
    float y1 = y0 + dy;
    SDL_SetRenderDrawColor(window->renderer, 70, 107, 235, 255);
    SDL_RenderDrawLine(window->renderer, x0 + offset_x, y0 + offset_y, x1 + offset_x, y1 + offset_y);
}

void draw_rays(Window* window, Ray* rays, char** map, int offset_x, int offset_y)
{
    for (int i = 0; i < NUMBER_RAYS; i++)
    {
        float distance = ray_hits_wall(map, &rays[i]);
        rays[i].len = distance;
        if (i % RAY_DISPLAY_INTERVAL == 0)
            draw_ray(window, &rays[i], map, offset_x, offset_y, distance);
    }
}