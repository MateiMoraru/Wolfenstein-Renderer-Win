#include "asset_drawer.h"

SDL_Color BLACK = {0, 0, 0, 255};
SDL_Color WHITE = {255, 255, 255, 255};
SDL_Color RED = {255, 0, 0, 255};

Color4 CBLACK = {0, 0, 0, 255};
Color4 CWHITE = {255, 255, 255, 255};
Color4 CRED = {255, 0, 0, 255};
Color4 CNULL = {0, 0, 0, 0};
Color4 BACKGROUND_COLOR = {173, 152, 125, 255};

int width;
int height;
int scale;
int initial_scale = 10;

int zoom_offset_x = 0;
int zoom_offset_y = 0;

char* rgba_to_hex(Color4 *color)
{
    char *hex = malloc(11);
    if (!hex) return NULL;

    sprintf(hex, "0x%02X%02X%02X%02X", color->r, color->g, color->b, color->a);

    return hex;
}

Color4 hex_to_color(char* hexc)
{
    uint32_t hex = (uint32_t) strtoul(hexc, NULL, 0);
    
    Color4 color;

    color.r = (hex >> 24) & 0xFF;
    color.g = (hex >> 16) & 0xFF;
    color.b = (hex >> 8) & 0xFF;
    color.a = hex & 0xFF;

    return color;
}

Color4 **alloc_pixels(int width, int height)
{
    Color4 **p = malloc(height * sizeof(Color4 *));
    if (!p) return NULL;

    for (int y = 0; y < height; y++)
    {
        p[y] = malloc(width * sizeof(Color4));
        if (!p[y]) return NULL;
    }
    return p;
}

void free_pixels(Color4 **pixels, int height)
{
    for (int y = 0; y < height; y++)
        free(pixels[y]);
    free(pixels);
}

void init_pixels(Color4 **pixels, int width, int height)
{
    for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++)
            pixels[y][x] = CNULL;
}

void save_pixels(Color4 **pixels, int width, int height, const char* filename)
{
    FILE *fout = fopen(filename, "w");
    if (!fout) return;

    fprintf(fout, "%d %d\n", width, height);

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            char *hex = rgba_to_hex(&pixels[y][x]);
            fprintf(fout, "%s ", hex);
            free(hex);
        }
        fprintf(fout, "\n");
    }
    fclose(fout);
}

void load_pixels(Color4 **pixels, int width, int height, const char* filename)
{
    FILE *fin = fopen(filename, "r");
    if (!fin) return;

    int w, h;
    if (fscanf(fin, "%d %d", &w, &h) != 2)
    {
        fclose(fin);
        return;
    }

    if (w != width || h != height)
    {
        printf("Size mismatch: file %dx%d, expected %dx%d\n", w, h, width, height);
        fclose(fin);
        return;
    }

    char color_str[11];
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            fscanf(fin, "%10s", color_str);
            pixels[y][x] = hex_to_color(color_str);
        }
    }

    fclose(fin);
}


void load_palette(Color4* palette, const char* filename)
{
    FILE* file = fopen(filename, "rb");
    if (!file)
    {
        printf("Failed to open %s\n", filename);
        return;
    }

    int r, g, b;
    Color4 color;
    color.a = 255;
    for (int i = 0; i < PALETTE_SIZE; i++)
    {
        if(fscanf(file, "%d %d %d", &r, &g, &b) != 3) 
        {
            fprintf(stderr, "Failed to read color %d\n", i);
            break;
        }
        
        color.r = r;
        color.g = g;
        color.b = b;

        palette[i] = color;
        
    }

    fclose(file);

}

void load_file(char* argv[], Color4 **pixels)
{
    FILE *fin = fopen(argv[4], "rb");

    if (!fin)
    {
        printf("Failed to open %s\n", argv[4]);
        return;
    }

    int w, h;
    fscanf(fin, "%d %d", &w, &h);

    fclose(fin);

    if(w != width || h != height)
    {
        printf("Expected %dx%d but found %dx%d\n", width, height, w, h);
        return;
        //init_pixels(pixels);
    }
    
    load_pixels(pixels, 128, 128, argv[4]);
    printf("Loaded file %s successfully\n", argv[4]);
    
    
}

void reinit_window(SDL_Window* window, SDL_Renderer* renderer, Color4 **pixels, const char* filename)
{
    load_file(filename, pixels);

    if(window) SDL_DestroyWindow(window);
    if(renderer) SDL_DestroyRenderer(renderer);

    window = SDL_CreateWindow("Asset drawer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width * scale + PALETTE_WIDTH, height * scale, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    compute_initial_scale();
    scale = initial_scale;
}

void compute_initial_scale()
{
    int scale_w = MAX_WINDOW_WIDTH / (width + PALETTE_WIDTH / initial_scale);
    int scale_h = MAX_WINDOW_HEIGHT / height;

    int s = scale_w < scale_h ? scale_w : scale_h;
    if (s < 1) s = 1;

    initial_scale = s;
}