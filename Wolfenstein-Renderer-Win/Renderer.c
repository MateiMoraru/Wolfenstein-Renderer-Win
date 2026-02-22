#include "Renderer.h"

const Color4 COLOR_BACKGROUND = { 10, 10, 20, 255 };
const Color4 COLOR_WALL = { 101, 85, 115, 255 };
const Color4 COLOR_SEPARATOR = { 100, 93, 120, 255 };

void smoke_spawn(Smoke_puff* puffs, float x, float y)
{
    for (int i = 0; i < MAX_SMOKE_PUFFS; i++)
    {
        if (!puffs[i].alive)
        {
            puffs[i].x = x;
            puffs[i].y = y;
            puffs[i].age = 0.0f;
            puffs[i].ttl = 0.35f;
            puffs[i].alive = 1;
            return;
        }
    }
}

void smoke_update(Smoke_puff* puffs, float dt)
{
    for (int i = 0; i < MAX_SMOKE_PUFFS; i++)
    {
        if (!puffs[i].alive) continue;
        puffs[i].age += dt;
        if (puffs[i].age >= puffs[i].ttl) puffs[i].alive = 0;
    }
}

float wrap_angle(float a)
{
    while (a < 0.0f) a += 2.0f * (float)M_PI;
    while (a >= 2.0f * (float)M_PI) a -= 2.0f * (float)M_PI;
    return a;
}

float angle_diff(float a, float b)
{
    float d = wrap_angle(a) - wrap_angle(b);
    if (d > (float)M_PI) d -= 2.0f * (float)M_PI;
    if (d < -(float)M_PI) d += 2.0f * (float)M_PI;
    return d;
}

void shoot_and_spawn_smoke(char** map, RayCaster* ray_caster, Smoke_puff* puffs)
{
    int mid = NUMBER_RAYS / 2;
    Ray r = ray_caster->rays[mid];

    float t = ray_hits_wall(map, &r);
    if (t <= 0.0f) return;

    float dx = cosf(r.angle);
    float dy = sinf(r.angle);

    float push = 0.01f;
    smoke_spawn(puffs, r.hit_x - dx * push, r.hit_y - dy * push);
}

void smoke_draw_all(Window* window, Player* player, RayCaster* ray_caster, Smoke_puff* puffs, Sprite* smoke_sprite)
{
    float fov_rad = DEG_TO_RAD((float)player->fov);
    float start_angle = ray_caster->rays[0].angle;
    float column_width = (float)window->width / (float)NUMBER_RAYS;

    for (int i = 0; i < MAX_SMOKE_PUFFS; i++)
    {
        if (!puffs[i].alive) continue;

        float vx = puffs[i].x - player->x;
        float vy = puffs[i].y - player->y;

        float dist = sqrtf(vx * vx + vy * vy);
        if (dist <= 0.0001f) continue;

        float a = atan2f(vy, vx);

        float da = wrap_angle(a - start_angle);
        if (da > fov_rad) continue;

        float u = da / fov_rad;
        int ray_idx = (int)(u * (float)(NUMBER_RAYS - 1));
        if (ray_idx < 0) ray_idx = 0;
        if (ray_idx >= NUMBER_RAYS) ray_idx = NUMBER_RAYS - 1;

        float wall_dist = ray_caster->rays[ray_idx].len;
        if (wall_dist <= 0.0f) continue;

        float along = dist * cosf(angle_diff(a, ray_caster->rays[ray_idx].angle));
        if (along > wall_dist - 0.02f) continue;

        float wall_h = (float)window->height / wall_dist * HEIGHT_MULT;
        if (wall_h > (float)window->height) wall_h = (float)window->height;
        if (wall_h < 2.0f) wall_h = 2.0f;

        float smoke_h = wall_h * 0.16f;
        if (smoke_h < 6.0f) smoke_h = 6.0f;
        if (smoke_h > 48.0f) smoke_h = 48.0f;

        float aspect = (float)smoke_sprite->width / (float)smoke_sprite->height;
        float smoke_w = smoke_h * aspect;

        float t = puffs[i].age / puffs[i].ttl;
        float fade = 1.0f - t;
        if (fade < 0.0f) fade = 0.0f;
        if (fade > 1.0f) fade = 1.0f;

        SDL_SetTextureBlendMode(smoke_sprite->texture, SDL_BLENDMODE_BLEND);
        SDL_SetTextureAlphaMod(smoke_sprite->texture, (Uint8)(255.0f * fade));

        int x0 = (int)(ray_idx * column_width);
        int x1 = (int)((ray_idx + 1) * column_width);
        int sx = x0 + (x1 - x0) / 2;

        int wall_top = window->height / 2 - (int)wall_h / 2;

        SDL_Rect dst;
        dst.w = (int)smoke_w;
        dst.h = (int)smoke_h;
        dst.x = sx - dst.w / 2;
        dst.y = wall_top + (int)(wall_h * 0.50f) - dst.h / 2;

        SDL_RenderCopy(window->renderer, smoke_sprite->texture, NULL, &dst);
    }
}

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
    float column_width = (float)window->width / (float)NUMBER_RAYS;
    SDL_Rect rect = { 0,0,0,0 };

    float cx = enemy->x + 0.5f;
    float cy = enemy->y + 0.5f;

    float px = renderer->player->x;
    float py = renderer->player->y;

    float nx = px - cx;
    float ny = py - cy;
    float nlen = sqrtf(nx * nx + ny * ny);
    if (nlen < 1e-6f) nlen = 1e-6f;
    nx /= nlen; ny /= nlen;

    float rx = -ny;
    float ry = nx;

    const float half_w = 0.5f;

    for (int i = 0; i < NUMBER_RAYS; i++)
    {
        Column col = renderer->pixels[i];
        Ray ray = renderer->ray_caster->rays[i];

        rect.x = (int)(i * column_width);
        rect.y = window->height / 2 - col.height / 2;
        rect.h = col.height;
        rect.w = (int)((i + 1) * column_width) - rect.x;

        SDL_SetRenderDrawColor(window->renderer, col.color.r, col.color.g, col.color.b, 255);
        SDL_RenderFillRect(window->renderer, &rect);

        float dx = cosf(ray.angle);
        float dy = sinf(ray.angle);

        float denom = dx * nx + dy * ny;
        if (fabsf(denom) < 1e-6f) continue;

        float t = ((cx - px) * nx + (cy - py) * ny) / denom;
        if (t <= 0) continue;
        if (t >= ray.len) continue;

        float hx = px + dx * t;
        float hy = py + dy * t;

        float off = (hx - cx) * rx + (hy - cy) * ry;

        if (off < -half_w || off > half_w) continue;

        float u = (off + half_w) / (2.0f * half_w);
        int tex_x = (int)(u * enemy->sprite.width);
        if (tex_x < 0) tex_x = 0;
        if (tex_x >= enemy->sprite.width) tex_x = enemy->sprite.width - 1;

        float proj_height = (window->height / t) * HEIGHT_MULT / 2;
        if (proj_height < 2) proj_height = 2;
        if (proj_height > window->height) proj_height = (float)window->height;

        SDL_Texture* tex = enemy->sprite.tex_columns[tex_x];
        if (!tex) continue;

        float mult = (float)COLOR_MULT / (float)t;

        if (mult > 0.8f)
        {
            mult = 0.8f;
        }

        if (mult < MIN_DIST_MULT)
        {
            continue;
        }

        Uint8 mod = (Uint8)(255.0f * mult);

        SDL_SetTextureColorMod(tex, mod, mod, mod);

        SDL_Rect dst;
        dst.x = rect.x;
        dst.w = rect.w;
        dst.h = (int)proj_height;
        dst.y = window->height / 2 - dst.h / 2;

        SDL_RenderCopy(window->renderer, tex, NULL, &dst);
    }
}