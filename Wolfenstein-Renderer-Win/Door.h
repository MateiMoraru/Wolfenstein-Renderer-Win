/*

		DOOR

		used as a, door :)

*/

#ifndef DOOR_H
#define DOOR_H

#define MAX_DOORS 64

#include <stdint.h>

typedef struct Door
{
	int x;
	int y;
	uint8_t id;
} Door;

Door door_init(uint8_t id, int x, int y);

// This function checks for all the doors registered and unlocks them if id matches
void doors_unlock(uint8_t id, char** map, Door* doors);

#endif