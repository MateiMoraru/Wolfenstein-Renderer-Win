#include "Sprite.h"

Sprite sprite_load(SDL_Renderer* renderer, const char* filename, float x, float y)
{
    Sprite sprite = { 0 };

    sprite.x = x;
    sprite.y = y;

    sprite.width = 32;
    sprite.height = 32;

    Color4** pixels = alloc_pixels(sprite.width, sprite.height);
    if (!pixels) return sprite;

    load_pixels(pixels, sprite.width, sprite.height, filename);

    uint32_t* buffer = malloc(sprite.width * sprite.height * sizeof(uint32_t));
    if (!buffer) {
        free_pixels(pixels, sprite.height);
        return sprite;
    }

    for (int y = 0; y < sprite.height; y++)
    {
        for (int x = 0; x < sprite.width; x++)
        {
            Color4 c = pixels[y][x];

            buffer[y * sprite.width + x] =
                (c.a << 24) |
                (c.b << 16) |
                (c.g << 8) |
                (c.r);
        }
    }

    sprite.texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, sprite.width, sprite.height);

    if (sprite.texture)
    {
        SDL_UpdateTexture(sprite.texture, NULL, buffer, sprite.width * sizeof(uint32_t));
        SDL_SetTextureBlendMode(sprite.texture, SDL_BLENDMODE_BLEND);
    }

    free(buffer);
    free_pixels(pixels, sprite.height);

    return sprite;
}

void sprite_set_position(Sprite* sprite, float x, float y)
{
    sprite->x = x;
    sprite->y = y;
}

void sprite_draw(Window* window, Sprite* sprite, float scale)
{
    SDL_Rect dst = { sprite->x, sprite->y, (int)(sprite->width * scale), (int)(sprite->height * scale) };
    SDL_RenderCopy(window->renderer, sprite->texture, NULL, &dst);
}