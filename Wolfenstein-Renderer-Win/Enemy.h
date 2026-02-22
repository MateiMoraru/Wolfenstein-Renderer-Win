#ifndef ENEMY_H
#define ENEMY_H

#include <SDL.h>
#include <stdbool.h>

#include "Window.h"
#include "Sprite.h"
#include "AStar.h"

#define ENEMY_MAX_PATH 512

typedef struct
{
	float x;
	float y;
	
	int map_x;
	int map_y;

	float scale;

	bool drawn;
	bool active;

	int hits;

	Sprite sprite;

	float speed;
	float tile_size;
	int path[ENEMY_MAX_PATH];
	int path_len;
	int path_pos;
	float repath_accum;
	float repath_interval;
	int last_goal_cell;

} Enemy;

Enemy enemy_init(Window* window, const char* filename, float x, float y, float scale);

void enemy_draw(Window* window, Enemy* enemy);
void enemy_draw_pos_size(Window* window, Enemy* enemy, float x, float y, float scale);

void enemy_set_sprite(Enemy* enemy, Sprite* sprite);

void enemy_update(Enemy* enemy, float dt, char** map, int rows, int cols, float target_x, float target_y);

#endif