#include "Renderer.h"

const Color4 COLOR_BACKGROUND = { 10, 10, 20, 255 };
const Color4 COLOR_WALL = { 101, 85, 115, 255 };
const Color4 COLOR_SEPARATOR = { 100, 93, 120, 255 };

Renderer* renderer_init(Player* player, RayCaster* ray_caster, Window* window)
{
    Renderer* renderer = malloc(sizeof(Renderer));

    renderer->player = player;
    renderer->ray_caster = ray_caster;
    renderer->window = window;

    for (int i = 0; i < NUMBER_RAYS; i++)
    {
        renderer->pixels[i] = (Column){ COLOR_BACKGROUND, window->height };
    }

    return renderer;
}

Column compute_column(int max_height, Ray* ray)
{
    Color4 color;
    if (ray->hit_id == '#')
        color = COLOR_WALL;
    else
        color = COLOR_SEPARATOR;

    int height;

    height = (int)((float)max_height / ray->len * HEIGHT_MULT);

    if (height > max_height)
    {
        height = max_height;
    }

    if (height < 2)
        height = 2;
    float mult = (float)COLOR_MULT / (float)ray->len;

    if (mult > .8f)
    {
        mult = .8f;

    }
    color.r *= mult;
    color.g *= mult;
    color.b *= mult;

    if (mult < MIN_DIST_MULT)
    {
        color = COLOR_BACKGROUND;
    }


    Column col = { color, height };
    return col;
}

void renderer_update(Renderer* renderer)
{
    Ray* ray;
    for (int i = 0; i < NUMBER_RAYS; i++)
    {
        ray = &renderer->ray_caster->rays[i];

        if (ray->len >= MAX_SEE_DISTANCE)
            renderer->pixels[i] = (Column){ COLOR_BACKGROUND, renderer->window->height };
        else
            renderer->pixels[i] = compute_column(renderer->window->height, &renderer->ray_caster->rays[i]);
    }
}

void renderer_draw(Renderer* renderer, Window* window, Enemy* enemy)
{
    enemy->drawn = false;
    float column_width = (float)window->width / (float)NUMBER_RAYS;
    SDL_Rect rect = { 0, 0, 0, 0 };
    Ray ray;

    float enemy_screen_x = 0;
    float enemy_screen_y = 0;
    float enemy_scale = 0;

    for (int i = 0; i < NUMBER_RAYS; i++)
    {
        Column col = renderer->pixels[i];

        rect.x = (int)(i * column_width);
        rect.y = window->height / 2 - col.height / 2;
        rect.h = col.height;
        rect.w = (int)((i + 1) * column_width) - rect.x;

        SDL_SetRenderDrawColor(window->renderer, col.color.r, col.color.g, col.color.b, 255);
        SDL_RenderFillRect(window->renderer, &rect);

        ray = renderer->ray_caster->rays[i];

        if (ray.hit_enemy == 'E' && !enemy->drawn)
        {
            enemy_screen_x = i * column_width - i / 32.f;

            float projected_height = (window->height / ray.hit_enemy_distance) * HEIGHT_MULT;

            enemy_screen_y = window->height / 2 - projected_height / 2 + 32;

            enemy_scale = projected_height / enemy->sprite.height;

            enemy->drawn = true;
        }
    }

    if (enemy->drawn)
    {
        enemy_draw_pos_size(window, enemy, enemy_screen_x, enemy_screen_y, enemy_scale);
    }
}