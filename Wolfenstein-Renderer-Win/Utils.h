/*
		UTILS

		Useful functions

*/

#pragma once

#include <stdbool.h>

// Classic
bool intersect_point_rect(float px, float py, float x, float y, float w, float h);

// Distance squared (sqrt is slow )
float dist_sq(float x1, float y1, float x2, float y2);