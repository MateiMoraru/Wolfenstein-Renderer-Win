#include "RayCaster.h"

#ifndef SENSITIVITY
#define SENSITIVITY 190
#endif

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
    Ray ray;
    for (int i = 0; i < NUMBER_RAYS; i++)
    {
        float angle_deg = i * ((float)fov / (float)NUMBER_RAYS) + direction;
        float angle_rad = DEG_TO_RAD(angle_deg);

        ray.angle = angle_rad;
        ray.x = x;
        ray.y = y;
        ray_caster->rays[i] = ray;
    }
}

void ray_increase_angle(Ray* ray, float angle)
{
    ray->angle += angle;

    if (ray->angle < 0)
        ray->angle += 2 * M_PI;
    else if (ray->angle > 2 * M_PI)
        ray->angle -= 2 * M_PI;
}

void ray_caster_rotate(RayCaster* ray_caster, float dx)
{
    if (dx == 0)
        return;

    float angle = dx * (M_PI / 180.0f);

    for (int i = 0; i < NUMBER_RAYS; i++)
    {
        ray_increase_angle(&ray_caster->rays[i], angle);
    }
}


void draw_ray(Window* window, Ray* ray, char** map)
{

    float distance = ray_hits_wall(map, ray);

    if (distance < 0)
    {
        distance = RAY_LEN;
    }

    ray->len = distance;

    // ray->slope = tan(ray->angle);

    // float direction = 1 / ray->angle;

    float dx = distance * cos(ray->angle) * SCALE;
    float dy = distance * sin(ray->angle) * SCALE;


    float x0 = ray->x * SCALE;
    float y0 = ray->y * SCALE;

    float x1 = x0 + dx;
    float y1 = y0 + dy;

    SDL_SetRenderDrawColor(window->renderer, 0, 255, 0, 255);
    SDL_RenderDrawLine(window->renderer, x0, y0, x1, y1);
}

void draw_rays(Window* window, Ray* rays, char** map)
{
    for (int i = 0; i < NUMBER_RAYS; i++)
    {
        draw_ray(window, &rays[i], map);
    }
}

float ray_hits_wall(char** map, Ray* ray)
{
    float x = ray->x;
    float y = ray->y;

    float dx = cos(ray->angle);
    float dy = sin(ray->angle);

    int mapx = (int)x;
    int mapy = (int)y;

    float side_dist_x;
    float side_dist_y;

    float delta_dist_x = (dx == 0) ? 1e30f : fabs(1.0f / dx);
    float delta_dist_y = (dy == 0) ? 1e30f : fabs(1.0f / dy);

    int stepx = (dx < 0) ? -1 : 1;
    int stepy = (dy < 0) ? -1 : 1;

    if (dx < 0)
    {
        side_dist_x = (x - mapx) * delta_dist_x;
    }
    else
    {
        side_dist_x = (mapx + 1.0f - x) * delta_dist_x;
    }

    if (dy < 0)
    {
        side_dist_y = (y - mapy) * delta_dist_y;
    }
    else
    {
        side_dist_y = (mapy + 1.0f - y) * delta_dist_y;
    }

    float distance = RAY_LEN;
    int side = 0;
    bool hit = false;

    while (!hit && distance-- > 0)
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

        if (map[mapy][mapx] == '#')
        {
            hit = true;
        }
    }

    if (!hit)
        return -1;

    if (side == 0)
        return (mapx - x + (1 - stepx) / 2.0f) / dx;

    return (mapy - y + (1 - stepy) / 2.0f) / dy;
}