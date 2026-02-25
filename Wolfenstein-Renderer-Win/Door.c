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

bool doors_unlock(uint8_t id, char** map, Door* doors, Player* player)
{
    int px = (int)player->x;
    int py = (int)player->y;
    bool opened = false;

    for (int i = 0; i < MAX_DOORS; i++)
    {
        int dist = dist_sq(doors[i].x, doors[i].y, px, py);

        if (dist < MIN_UNLOCK_DISTANCE && doors[i].id == id)
        {
            map[doors[i].y][doors[i].x] = ' ';
            opened = true;
        }
    }

    return opened;
}