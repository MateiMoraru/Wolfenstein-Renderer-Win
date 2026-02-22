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