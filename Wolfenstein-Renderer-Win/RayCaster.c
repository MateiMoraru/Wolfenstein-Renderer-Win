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
        ray_caster->rays[i].x = (float)x;
        ray_caster->rays[i].y = (float)y;
        ray_caster->rays[i].angle = angle;
        ray_caster->rays[i].hit_id = ' ';
        ray_caster->rays[i].hit_entity = ' ';
        ray_caster->rays[i].hit_entity_distance = 1e30f;
        ray_caster->rays[i].len = 1e30f;
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

void ray_caster_cast_all(RayCaster* ray_caster, char** map)
{
    float view_angle = ray_caster->rays[NUMBER_RAYS / 2].angle;

    for (int i = 0; i < NUMBER_RAYS; i++)
    {
        float distance = ray_hits_wall(map, &ray_caster->rays[i]);
        if (distance < 0.0f) distance = RAY_LEN + 1.0f;

        float theta = ray_caster->rays[i].angle - view_angle;
        while (theta > (float)M_PI) theta -= 2.0f * (float)M_PI;
        while (theta < -(float)M_PI) theta += 2.0f * (float)M_PI;

        ray_caster->rays[i].len = distance * cosf(theta);
        if (ray_caster->rays[i].len < 0.0001f) ray_caster->rays[i].len = 0.0001f;
    }
}

float ray_hits_wall(char** map, Ray* ray)
{
    ray->hit_id = ' ';
    ray->hit_side = 0;
    ray->len = 0.000001f;
    ray->hit_x = -1.0f;
    ray->hit_y = -1.0f;

    ray->hit_entity = ' ';
    ray->hit_entity_distance = 1e30f;
    ray->hit_entity_x = -1.0f;
    ray->hit_entity_y = -1.0f;
    ray->hit_entity_u = 0.0f;

    float x = ray->x;
    float y = ray->y;

    float dx = cosf(ray->angle);
    float dy = sinf(ray->angle);

    int mapx = (int)x;
    int mapy = (int)y;

    float delta_dist_x = (dx == 0.0f) ? 1e30f : fabsf(1.0f / dx);
    float delta_dist_y = (dy == 0.0f) ? 1e30f : fabsf(1.0f / dy);

    int stepx = (dx < 0.0f) ? -1 : 1;
    int stepy = (dy < 0.0f) ? -1 : 1;

    float side_dist_x = (dx < 0.0f) ? (x - mapx) * delta_dist_x : (mapx + 1.0f - x) * delta_dist_x;
    float side_dist_y = (dy < 0.0f) ? (y - mapy) * delta_dist_y : (mapy + 1.0f - y) * delta_dist_y;

    int side = 0;

    while (1)
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

        if ((unsigned)mapx >= (unsigned)MAP_WIDTH || (unsigned)mapy >= (unsigned)MAP_HEIGHT)
            return -1.0f;

        char cell = map[mapy][mapx];

        float t = (side == 0)
            ? (mapx - x + (1 - stepx) / 2.0f) / dx
            : (mapy - y + (1 - stepy) / 2.0f) / dy;

        if (t <= 0.0f)
            continue;

        if (cell == 'E' || cell == 'Y' || cell == 'R' || cell == 'G' || cell == 'B' || cell == 'C' || cell == 'A')
        {
            if (t < ray->hit_entity_distance)
            {
                ray->hit_entity = cell;
                ray->hit_entity_distance = t;
                ray->hit_entity_x = (float)mapx + 0.5f;
                ray->hit_entity_y = (float)mapy + 0.5f;
            }
        }

        if (cell == '#' || cell == '0' || cell == '1' || cell == '2' || cell == '3' || cell == '4')
        {
            ray->hit_id = cell;
            ray->hit_side = side;
            ray->hit_x = x + dx * t;
            ray->hit_y = y + dy * t;
            ray->len = t;

            if (ray->hit_entity != ' ' && ray->hit_entity_distance >= t)
            {
                ray->hit_entity = ' ';
                ray->hit_entity_distance = 1e30f;
                ray->hit_entity_x = -1.0f;
                ray->hit_entity_y = -1.0f;
            }

            return t;
        }
    }
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
        if (i % RAY_DISPLAY_INTERVAL == 0)
            draw_ray(window, &rays[i], map, offset_x, offset_y, rays[i].len);
    }
}


bool ray_caster_hit_enemy(RayCaster* ray_caster)
{
    for (int i = 0; i < NUMBER_RAYS; i += RAY_CHECK_SKIP_INTERVAL)
    {
        if (ray_caster->rays[i].hit_entity == 'E')
        {
            return true;
        }
    }

    return false;
}