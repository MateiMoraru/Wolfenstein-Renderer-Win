#ifndef ENEMY_H
#define ENEMY_H

#include <SDL.h>
#include <stdbool.h>

#include "Window.h"
#include "Sprite.h"

typedef struct
{
	float x;
	float y;

	float scale;

	bool drawn;
	bool active;

	Sprite sprite;
} Enemy;

Enemy enemy_init(Window* window, const char* filename, float x, float y, float scale);

void enemy_draw(Window* window, Enemy* enemy);
void enemy_draw_pos_size(Window* window, Enemy* enemy, float x, float y, float scale);

#endif