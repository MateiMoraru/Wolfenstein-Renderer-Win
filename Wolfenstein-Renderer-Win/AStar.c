#include "AStar.h"

int abs_i(int x) 
{ 
    return x < 0 ? -x : x; 
}

int manhattan(point a, point b) 
{ 
    return abs_i(a.r - b.r) + abs_i(a.c - b.c); 
}

int idx(int r, int c, int cols) 
{ 
    return r * cols + c; 
}

int in_bounds(int r, int c, int rows, int cols) 
{ 
    return (unsigned)r < (unsigned)rows && (unsigned)c < (unsigned)cols; 
}

int is_walkable(char ch) 
{ 
    return ch == ' '; 
}

int reconstruct_path(int start_i, int goal_i, const int* parent, int cols, char* grid_out, int rows, int cols2) 
{
    int cur = goal_i;
    int steps = 0;
    while (cur != -1 && cur != start_i) 
    {
        int r = cur / cols;
        int c = cur % cols;
        if (grid_out[idx(r, c, cols2)] != 'S' && grid_out[idx(r, c, cols2)] != 'G') grid_out[idx(r, c, cols2)] = '*';
        cur = parent[cur];
        steps++;
    }
    return (cur == start_i) ? steps : -1;
}

int astar_grid(const char* grid, int rows, int cols, point start, point goal, char* grid_out) 
{
    int n = rows * cols;

    int* g = (int*)malloc((size_t)n * sizeof(int));
    int* f = (int*)malloc((size_t)n * sizeof(int));

    int* parent = (int*)malloc((size_t)n * sizeof(int));

    unsigned char* in_open = (unsigned char*)calloc((size_t)n, 1);
    unsigned char* closed = (unsigned char*)calloc((size_t)n, 1);

    if (!g || !f || !parent || !in_open || !closed) 
    {
        free(g); free(f); free(parent); free(in_open); free(closed);
        return -2;
    }

    for (int i = 0; i < n; i++) 
    {
        g[i] = INT_MAX / 4;
        f[i] = INT_MAX / 4;
        parent[i] = -1;
        grid_out[i] = grid[i];
    }

    int s = idx(start.r, start.c, cols);
    int t = idx(goal.r, goal.c, cols);

    if (!in_bounds(start.r, start.c, rows, cols) || !in_bounds(goal.r, goal.c, rows, cols)) 
    {
        free(g); free(f); free(parent); free(in_open); free(closed);
        return -3;
    }
    if (!is_walkable(grid[s]) || !is_walkable(grid[t])) 
    {
        free(g); free(f); free(parent); free(in_open); free(closed);
        return -4;
    }

    g[s] = 0;
    f[s] = manhattan(start, goal);
    in_open[s] = 1;

    const int dr[4] = { -1,  1,  0,  0 };
    const int dc[4] = { 0,  0, -1,  1 };

    while (1) 
    {
        int current = -1;
        int best_f = INT_MAX;

        for (int i = 0; i < n; i++) 
        {
            if (in_open[i] && !closed[i] && f[i] < best_f) 
            {
                best_f = f[i];
                current = i;
            }
        }

        if (current == -1) 
        {
            free(g); free(f); free(parent); free(in_open); free(closed);
            return 0;
        }

        if (current == t) 
        {
            int steps = reconstruct_path(s, t, parent, cols, grid_out, rows, cols);
            free(g); free(f); free(parent); free(in_open); free(closed);
            return steps >= 0 ? 1 : 0;
        }

        in_open[current] = 0;
        closed[current] = 1;

        int cr = current / cols;
        int cc = current % cols;

        for (int k = 0; k < 4; k++) 
        {
            int nr = cr + dr[k];
            int nc = cc + dc[k];

            if (!in_bounds(nr, nc, rows, cols)) 
                continue;

            int ni = idx(nr, nc, cols);

            if (closed[ni]) continue;

            if (!is_walkable(grid[ni])) 
                continue;

            int tentative_g = g[current] + 1;

            if (tentative_g < g[ni]) 
            {
                parent[ni] = current;
                g[ni] = tentative_g;
                point np = (point){ nr, nc };

                f[ni] = tentative_g + manhattan(np, goal);
                in_open[ni] = 1;
            }
        }
    }
}