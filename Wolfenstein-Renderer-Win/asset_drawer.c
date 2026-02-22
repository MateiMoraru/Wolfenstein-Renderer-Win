#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "asset_drawer.h"

int width;
int height;
int scale;
int initial_scale = 10;

int zoom_offset_x = 0;
int zoom_offset_y = 0;

int hover_px = 0;
int hover_py = 0;

SDL_Color BLACK = { 0, 0, 0, 255 };
SDL_Color WHITE = { 255, 255, 255, 255 };
SDL_Color RED = { 255, 0, 0, 255 };

Color4 CBLACK = { 0, 0, 0, 255 };
Color4 CWHITE = { 255, 255, 255, 255 };
Color4 CRED = { 255, 0, 0, 255 };
Color4 CNULL = { 0, 0, 0, 0 };
Color4 BACKGROUND_COLOR = { 173, 152, 125, 255 };

void char_to_bits(char c, int bits[8])
{
    for (int i = 0; i < 8; i++)
    {
        bits[7 - i] = (c >> i) & 1;
    }
}

void char_to_binary_string(unsigned char c, char bits[9]) {
    for (int i = 7; i >= 0; i--) {
        bits[7 - i] = ((c >> i) & 1) + '0';
    }
    bits[8] = '\0';
}

Pair find_longest_match(char* buffer, int pos, int window_size, int max_look_ahead)
{
    if (pos == 0)
    {
        return (Pair) { 0, 0 };
    }
    int best_len = 0;
    int best_offset = 0;
    int buffer_len = strlen(buffer);

    int window_start = (pos > window_size) ? pos - window_size : 0;

    for (int i = window_start; i < pos; i++)
    {
        int len = 0;

        while (pos + len < buffer_len && len < max_look_ahead && buffer[i + len] == buffer[pos + len])
        {
            len++;
        }

        if (len > best_len && len >= MIN_LZ)
        {
            best_len = len;
            best_offset = pos - i;
        }
    }

    return (Pair) { best_len, best_offset };
}

void compress_string(char* buffer, char* compressed_buffer)
{
    int buffer_len = strlen(buffer);

    compressed_buffer[0] = '\0';
    int pos = 0;

    Pair match = { -1, -1 };

    char bits[9];
    char temp[50];

    while (pos < buffer_len)
    {

        int look_ahead_len = buffer_len - pos;
        if (look_ahead_len > LOOK_AHEAD_WINDOW)
        {
            look_ahead_len = LOOK_AHEAD_WINDOW;
        }

        match = find_longest_match(buffer, pos, SEARCH_WINDOW, LOOK_AHEAD_WINDOW);

        if (match.length >= MIN_LZ)
        {
            sprintf(temp, "1 %d %d ", match.offset, match.length);
            strcat(compressed_buffer, temp);

            pos += match.length;
        }
        else
        {
            char_to_binary_string(buffer[pos], bits);
            strcat(compressed_buffer, bits);
            strcat(compressed_buffer, " ");

            char single_char[2] = { buffer[pos], '\0' };

            pos++;
        }
    }
}

void decompress_string(char* compressed, char* decompressed)
{
    decompressed[0] = '\0';
    char* token = strtok(compressed, " ");
    int pos = 0;

    while (token != NULL)
    {
        if (strcmp(token, "1") == 0)
        {
            char* offset_str = strtok(NULL, " ");
            char* length_str = strtok(NULL, " ");

            if (!offset_str || !length_str)
            {
                break;
            }

            int offset = atoi(offset_str);
            int length = atoi(length_str);

            for (int i = 0; i < length; i++)
            {
                decompressed[pos + i] = decompressed[pos - offset + i];
            }
            decompressed[pos + length] = '\0';
            pos += length;
        }
        else
        {
            char c = 0;

            for (int i = 0; i < 8; i++)
            {
                c = (c << 1) | (token[i] - '0');
            }

            decompressed[pos] = c;
            pos++;
            decompressed[pos] = '\0';
        }

        token = strtok(NULL, " ");
    }
}

char* rgba_to_hex(Color4* color)
{
    char* hex = malloc(11);
    if (!hex) return NULL;

    sprintf(hex, "0x%02X%02X%02X%02X", color->r, color->g, color->b, color->a);

    return hex;
}

Color4 hex_to_color(char* hexc)
{
    uint32_t hex = (uint32_t)strtoul(hexc, NULL, 0);

    Color4 color;

    color.r = (hex >> 24) & 0xFF;
    color.g = (hex >> 16) & 0xFF;
    color.b = (hex >> 8) & 0xFF;
    color.a = hex & 0xFF;

    return color;
}

Color4** alloc_pixels(int width, int height)
{
    Color4** p = malloc(height * sizeof(Color4*));
    if (!p) return NULL;

    for (int y = 0; y < height; y++)
    {
        p[y] = malloc(width * sizeof(Color4));
        if (!p[y])
        {
            for (int i = 0; i < y; i++) free(p[i]);
            free(p);
            return NULL;
        }
    }
    return p;
}

void free_pixels(Color4** pixels, int height)
{
    for (int y = 0; y < height; y++)
        free(pixels[y]);
    free(pixels);
}

void init_pixels(Color4** pixels, int width, int height)
{
    for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++)
            pixels[y][x] = CNULL;
}

void save_pixels(Color4** pixels, int width, int height, const char* filename)
{
    size_t plain_cap = (size_t)width * (size_t)height * 11 + (size_t)height + 64;
    char* plain = (char*)malloc(plain_cap);
    if (!plain)
        return;

    size_t pos = 0;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            Color4 c = pixels[y][x];
            int wrote = snprintf(plain + pos, plain_cap - pos, "0x%02X%02X%02X%02X ", c.r, c.g, c.b, c.a);
            if (wrote <= 0 || pos + (size_t)wrote >= plain_cap)
            {
                free(plain);
                return;
            }
            pos += (size_t)wrote;
        }

        if (pos + 2 >= plain_cap)
        {
            free(plain);
            return;
        }

        plain[pos++] = '\n';
        plain[pos] = '\0';
    }

    size_t comp_cap = plain_cap * 32 + 1024;
    char* compressed = (char*)malloc(comp_cap);
    if (!compressed)
    {
        free(plain);
        return;
    }

    compressed[0] = '\0';
    compress_string(plain, compressed);

    FILE* fout = fopen(filename, "wb");
    if (!fout)
    {
        printf("save_pixels: failed to open '%s'\n", filename);
        free(compressed);
        free(plain);
        return;
    }

    fprintf(fout, "%d %d\n", width, height);

    size_t clen = strlen(compressed);
    size_t wrote = fwrite(compressed, 1, clen, fout);
    fclose(fout);

    if (wrote != clen)
        printf("save_pixels: short write '%s' (%zu/%zu)\n", filename, wrote, clen);
    else
        printf("save_pixels: wrote '%s' (%zu bytes payload)\n", filename, clen);

    free(compressed);
    free(plain);
}

void load_pixels(Color4*** pixels, int* width, int* height, const char* filename)
{
    FILE* fin = fopen(filename, "rb");
    if (!fin)
    {
        printf("load_pixels: failed to open '%s'\n", filename);
        return;
    }

    int file_w = 0;
    int file_h = 0;
    if (fscanf(fin, "%d %d", &file_w, &file_h) != 2)
    {
        printf("load_pixels: bad header '%s'\n", filename);
        fclose(fin);
        return;
    }

    int ch = fgetc(fin);
    while (ch != EOF && ch != '\n')
        ch = fgetc(fin);

    if (*pixels != NULL) {
        free_pixels(*pixels, *height);
    }

    *pixels = alloc_pixels(file_w, file_h);
    if (!*pixels) {
        printf("load_pixels: failed to allocate memory\n");
        fclose(fin);
        return;
    }

    init_pixels(*pixels, file_w, file_h);

    fseek(fin, 0, SEEK_END);
    long end_pos = ftell(fin);
    fseek(fin, 0, SEEK_SET);

    fscanf(fin, "%*d %*d");
    ch = fgetc(fin);
    while (ch != EOF && ch != '\n')
        ch = fgetc(fin);

    long payload_start = ftell(fin);
    size_t payload_len = (size_t)(end_pos - payload_start);

    char* compressed = (char*)malloc(payload_len + 1);
    if (!compressed)
    {
        free_pixels(*pixels, file_h);
        fclose(fin);
        return;
    }

    size_t r = fread(compressed, 1, payload_len, fin);
    fclose(fin);
    compressed[r] = '\0';

    size_t decomp_cap = (size_t)file_w * (size_t)file_h * 11 + (size_t)file_h + 128;
    char* decompressed = (char*)malloc(decomp_cap);
    if (!decompressed)
    {
        free(compressed);
        free_pixels(*pixels, file_h);
        return;
    }

    decompressed[0] = '\0';
    decompress_string(compressed, decompressed);

    char* tok = strtok(decompressed, " \t\r\n");

    for (int y = 0; y < file_h; y++)
    {
        for (int x = 0; x < file_w; x++)
        {
            if (!tok)
            {
                printf("load_pixels: truncated data '%s'\n", filename);
                free(decompressed);
                free(compressed);
                free_pixels(*pixels, file_h);
                return;
            }

            (*pixels)[y][x] = hex_to_color(tok);
            tok = strtok(NULL, " \t\r\n");
        }
    }

    //*width = file_w;
    //*height = file_h;

    printf("load_pixels: loaded '%s' (%dx%d)\n", filename, file_w, file_h);

    free(decompressed);
    free(compressed);
}