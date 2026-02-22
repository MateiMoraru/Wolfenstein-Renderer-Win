#include "Enemy.h"
Enemy enemy_init(Window* window, const char* filename, float x, float y, float scale)
{
	Enemy enemy = { 0 };

	enemy.x = x;
	enemy.y = y;

	enemy.scale = scale;	

	enemy.sprite = sprite_load(window->renderer, filename, x, y);

	enemy.active = true;

	enemy.hits = 0;

	return enemy;
}

void enemy_draw(Window* window, Enemy* enemy)
{
	sprite_draw(window, &enemy->sprite, enemy->scale);
}

void enemy_draw_pos_size(Window* window, Enemy* enemy, float x, float y, float scale)
{
	enemy->sprite.x = x;
	enemy->sprite.y = y;

	sprite_draw(window, &enemy->sprite, scale);

	enemy->sprite.x = enemy->x;
	enemy->sprite.y = enemy->y;
}

void enemy_set_sprite(Enemy* enemy, Sprite* sprite)
{
	enemy->sprite = *sprite;
}

void enemy_update(Enemy* enemy, float dt, char** map, int rows, int cols, float target_x, float target_y)
{
	if (!enemy || !enemy->active || !map || rows <= 0 || cols <= 0) return;

	int cur_x = enemy->map_x;
	int cur_y = enemy->map_y;

	if ((unsigned)cur_x >= (unsigned)cols || (unsigned)cur_y >= (unsigned)rows) return;

	int goal_x = (int)floorf(target_x);
	int goal_y = (int)floorf(target_y);

	if (goal_x < 0) goal_x = 0;
	if (goal_y < 0) goal_y = 0;
	if (goal_x >= cols) goal_x = cols - 1;
	if (goal_y >= rows) goal_y = rows - 1;

	int n = rows * cols;
	char* flat_in = (char*)malloc((size_t)n);
	char* flat_out = (char*)malloc((size_t)n);
	if (!flat_in || !flat_out) { free(flat_in); free(flat_out); return; }

	for (int y = 0; y < rows; y++) {
		for (int x = 0; x < cols; x++) {
			char ch = map[y][x];
			if (ch == 'E') ch = ' ';
			flat_in[y * cols + x] = ch;
		}
	}

	point start = (point){ cur_y, cur_x };
	point goal = (point){ goal_y, goal_x };

	int found = astar_grid((const char*)flat_in, rows, cols, start, goal, flat_out);

	free(flat_in);

	if (found != 1) { free(flat_out); return; }

	int new_x = cur_x;
	int new_y = cur_y;

	int dr[4] = { -1, 1, 0, 0 };
	int dc[4] = { 0, 0, -1, 1 };

	for (int k = 0; k < 4; k++) {
		int ny = cur_y + dr[k];
		int nx = cur_x + dc[k];
		if ((unsigned)nx >= (unsigned)cols || (unsigned)ny >= (unsigned)rows) continue;

		char ch = flat_out[ny * cols + nx];
		if (ch == '*' || (ny == goal_y && nx == goal_x)) {
			new_y = ny;
			new_x = nx;
			break;
		}
	}

	free(flat_out);

	if (new_x == cur_x && new_y == cur_y) return;

	if (map[cur_y][cur_x] == 'E') map[cur_y][cur_x] = ' ';
	map[new_y][new_x] = 'E';

	enemy->map_x = new_x;
	enemy->map_y = new_y;

	enemy->x = (float)new_x;
	enemy->y = (float)new_y;

	enemy->sprite.x = enemy->x;
	enemy->sprite.y = enemy->y;
}