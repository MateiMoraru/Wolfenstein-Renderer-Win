#include "AStar.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    int node;
    int f;
} heap_item;

static int abs_i(int x)
{
    return x < 0 ? -x : x;
}

static int manhattan(point a, point b)
{
    return abs_i(a.r - b.r) + abs_i(a.c - b.c);
}

static int idx(int r, int c, int cols)
{
    return r * cols + c;
}

static int in_bounds(int r, int c, int rows, int cols)
{
    return (unsigned)r < (unsigned)rows &&
        (unsigned)c < (unsigned)cols;
}

static int is_walkable(char ch)
{
    return ch == ' ' || ch == 'E';
}

static int* g_cost;
static int* f_cost;
static int* parent_node;
static unsigned char* closed_set;

static heap_item* heap;
static int heap_size;
static int capacity;

static void ensure_capacity(int n)
{
    if (capacity >= n)
        return;

    g_cost = realloc(g_cost, n * sizeof(int));
    f_cost = realloc(f_cost, n * sizeof(int));
    parent_node = realloc(parent_node, n * sizeof(int));
    closed_set = realloc(closed_set, n * sizeof(unsigned char));
    heap = realloc(heap, n * sizeof(heap_item));

    capacity = n;
}

static void heap_swap(heap_item* a, heap_item* b)
{
    heap_item tmp = *a;
    *a = *b;
    *b = tmp;
}

static void heap_push(heap_item item)
{
    int i = heap_size++;
    heap[i] = item;

    while (i > 0)
    {
        int parent = (i - 1) / 2;
        if (heap[parent].f <= heap[i].f)
            break;

        heap_swap(&heap[parent], &heap[i]);
        i = parent;
    }
}

static heap_item heap_pop(void)
{
    heap_item top = heap[0];
    heap[0] = heap[--heap_size];

    int i = 0;
    while (1)
    {
        int left = 2 * i + 1;
        int right = 2 * i + 2;
        int smallest = i;

        if (left < heap_size &&
            heap[left].f < heap[smallest].f)
            smallest = left;

        if (right < heap_size &&
            heap[right].f < heap[smallest].f)
            smallest = right;

        if (smallest == i)
            break;

        heap_swap(&heap[i], &heap[smallest]);
        i = smallest;
    }

    return top;
}

int astar_grid(
    const char* grid,
    int rows,
    int cols,
    point start,
    point goal,
    char* grid_out
)
{
    int n = rows * cols;

    ensure_capacity(n);

    for (int i = 0; i < n; i++)
    {
        g_cost[i] = INT_MAX;
        f_cost[i] = INT_MAX;
        parent_node[i] = -1;
        closed_set[i] = 0;

        if (grid_out)
            grid_out[i] = grid[i];
    }

    if (!in_bounds(start.r, start.c, rows, cols) ||
        !in_bounds(goal.r, goal.c, rows, cols))
        return 0;

    int s = idx(start.r, start.c, cols);
    int t = idx(goal.r, goal.c, cols);

    if (!is_walkable(grid[s]) ||
        !is_walkable(grid[t]))
        return 0;

    heap_size = 0;

    g_cost[s] = 0;
    f_cost[s] = manhattan(start, goal);

    heap_push((heap_item) { s, f_cost[s] });

    const int dr[4] = { -1, 1, 0, 0 };
    const int dc[4] = { 0, 0, -1, 1 };

    while (heap_size > 0)
    {
        heap_item current_item = heap_pop();
        int current = current_item.node;

        if (closed_set[current])
            continue;

        if (current == t)
        {
            int cur = t;
            while (cur != -1 && cur != s)
            {
                if (grid_out)
                {
                    int r = cur / cols;
                    int c = cur % cols;
                    if (grid_out[idx(r, c, cols)] != 'S' &&
                        grid_out[idx(r, c, cols)] != 'G')
                        grid_out[idx(r, c, cols)] = '*';
                }
                cur = parent_node[cur];
            }
            return 1;
        }

        closed_set[current] = 1;

        int cr = current / cols;
        int cc = current % cols;

        for (int k = 0; k < 4; k++)
        {
            int nr = cr + dr[k];
            int nc = cc + dc[k];

            if (!in_bounds(nr, nc, rows, cols))
                continue;

            int ni = idx(nr, nc, cols);

            if (closed_set[ni])
                continue;

            if (!is_walkable(grid[ni]))
                continue;

            int tentative_g = g_cost[current] + 1;

            if (tentative_g < g_cost[ni])
            {
                parent_node[ni] = current;
                g_cost[ni] = tentative_g;

                point np = { nr, nc };
                f_cost[ni] = tentative_g + manhattan(np, goal);

                heap_push((heap_item) { ni, f_cost[ni] });
            }
        }
    }

    return 0;
}