#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

typedef struct 
{ 
	int r; 
	int c; 
} point;

int abs_i(int x);
int manhattan(point a, point b);
int idx(int r, int c, int cols);
int in_bounds(int r, int c, int rows, int cols);
int is_walkable(char ch);
int reconstruct_path(int start_i, int goal_i, const int* parent, int cols, char* grid_out, int rows, int cols2);
int astar_find_path(const char* grid, int rows, int cols, int start_r, int start_c, int goal_r, int goal_c, int* out_path, int max_path_len);