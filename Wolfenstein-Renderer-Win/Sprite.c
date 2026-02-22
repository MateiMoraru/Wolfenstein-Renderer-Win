#include "Sprite.h"

float clamp01(float v)
{
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

Sprite sprite_load(SDL_Renderer* renderer, const char* filename, float x, float y)
{
    Sprite sprite = (Sprite){ 0 };

    sprite.x = x;
    sprite.y = y;
    sprite.width = 16;
    sprite.height = 16;
    sprite.shade = 1.0f;

    FILE* fin = fopen(filename, "r");
    fscanf(fin, "%d %d", &sprite.width, &sprite.height);
    fclose(fin);

    Color4** pixels = NULL;
    load_pixels(&pixels, &sprite.width, &sprite.height, filename);
    if (!pixels) return sprite;

    sprite.tex_columns = malloc(sizeof(SDL_Texture*) * sprite.width);
    if (!sprite.tex_columns) {
        free_pixels(pixels, sprite.height);
        return sprite;
    }

    uint32_t* full_buffer = malloc(sizeof(uint32_t) * sprite.width * sprite.height);
    if (!full_buffer) {
        free(sprite.tex_columns);
        sprite.tex_columns = NULL;
        free_pixels(pixels, sprite.height);
        return sprite;
    }

    for (int xcol = 0; xcol < sprite.width; xcol++)
        sprite.tex_columns[xcol] = NULL;

    for (int xcol = 0; xcol < sprite.width; xcol++)
    {
        uint32_t* column_buffer = malloc(sizeof(uint32_t) * sprite.height);
        if (!column_buffer) continue;

        for (int yrow = 0; yrow < sprite.height; yrow++)
        {
            Color4 c = pixels[yrow][xcol];

            uint32_t packed =
                ((uint32_t)c.r << 24) |
                ((uint32_t)c.g << 16) |
                ((uint32_t)c.b << 8) |
                ((uint32_t)c.a);

            column_buffer[yrow] = packed;
            full_buffer[yrow * sprite.width + xcol] = packed;
        }

        SDL_Texture* tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, 1, sprite.height);
        if (tex)
        {
            SDL_UpdateTexture(tex, NULL, column_buffer, sizeof(uint32_t));
            SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
        }

        sprite.tex_columns[xcol] = tex;
        free(column_buffer);
    }

    sprite.texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, sprite.width, sprite.height);

    if (sprite.texture)
    {
        SDL_UpdateTexture(sprite.texture, NULL, full_buffer, sprite.width * (int)sizeof(uint32_t));
        SDL_SetTextureBlendMode(sprite.texture, SDL_BLENDMODE_BLEND);
    }

    free(full_buffer);
    free_pixels(pixels, sprite.height);

    return sprite;
}

void sprite_set_shade(Sprite* sprite, float shade)
{
    if (!sprite) return;
    sprite->shade = clamp01(shade);
}

void sprite_set_position(Sprite* sprite, float x, float y)
{
    sprite->x = x;
    sprite->y = y;
}

void sprite_draw(Window* window, Sprite* sprite, float scale)
{
    Uint8 m = (Uint8)(255.0f * clamp01(sprite->shade));
    SDL_SetTextureColorMod(sprite->texture, m, m, m);

    SDL_Rect dst = { (int)sprite->x, (int)sprite->y, (int)(sprite->width * scale), (int)(sprite->height * scale) };
    SDL_RenderCopy(window->renderer, sprite->texture, NULL, &dst);
}