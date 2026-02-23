#include "Door.h"

Door door_init(uint8_t id, int x, int y)
{
	Door door =
	{
		.x = x,
		.y = y,
		.id = id
	};

	return door;
}

void doors_unlock(uint8_t id, char** map, Door* doors)
{
	for (int i = 0; i < MAX_DOORS; i++)
	{
		if (doors[i].id == id)
		{
			map[doors[i].y][doors[i].x] = ' ';
		}
	}
}