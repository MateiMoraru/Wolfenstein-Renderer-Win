#include "Utils.h"

bool intersect_point_rect(float px, float py, float x, float y, float w, float h)
{
	return (px > x && px < x + w && py > y && py < y + h);
}


float dist_sq(float x1, float y1, float x2, float y2)
{
	float dx = x1 - x2;
	float dy = y1 - y2;

	return (dx * dx + dy * dy);
}