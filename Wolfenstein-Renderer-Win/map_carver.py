import random
from collections import deque

width, height = 128, 128
map_grid = [['#' for _ in range(width)] for _ in range(height)]

start_x, start_y = 2, 2

rooms = []

def add_room(x, y, w, h):
    for i in range(y, min(y + h, height - 1)):
        row = map_grid[i]
        for j in range(x, min(x + w, width - 1)):
            row[j] = ' '

def room_overlaps(x, y, w, h, pad=2):
    for rx, ry, rw, rh in rooms:
        if x < rx + rw + pad and x + w + pad > rx and y < ry + rh + pad and y + h + pad > ry:
            return True
    return False

def add_corridor(x1, y1, x2, y2, thickness=3):
    if x1 == x2 or y1 == y2:
        y0 = max(min(y1, y2) - thickness // 2, 0)
        y1b = min(max(y1, y2) + thickness // 2 + 1, height)
        x0 = max(min(x1, x2) - thickness // 2, 0)
        x1b = min(max(x1, x2) + thickness // 2 + 1, width)
        for i in range(y0, y1b):
            row = map_grid[i]
            for j in range(x0, x1b):
                row[j] = ' '
        return
    mid_x = x2
    mid_y = y1
    add_corridor(x1, y1, mid_x, mid_y, thickness)
    add_corridor(mid_x, mid_y, x2, y2, thickness)

def connect_room_to_nearest(idx, thickness=3):
    x, y, w, h = rooms[idx]
    cx = x + w // 2
    cy = y + h // 2
    best_j = None
    best_d = 10**18
    for j, (rx, ry, rw, rh) in enumerate(rooms):
        if j == idx:
            continue
        rcx = rx + rw // 2
        rcy = ry + rh // 2
        d = (cx - rcx) * (cx - rcx) + (cy - rcy) * (cy - rcy)
        if d < best_d:
            best_d = d
            best_j = j
    if best_j is None:
        return
    rx, ry, rw, rh = rooms[best_j]
    rcx = rx + rw // 2
    rcy = ry + rh // 2
    add_corridor(cx, cy, rcx, rcy, thickness=thickness)

def carve_maze(mask, step=4):
    cell_w = (width - 2) // step
    cell_h = (height - 2) // step
    visited = [[False for _ in range(cell_w)] for _ in range(cell_h)]
    stack = [(random.randrange(cell_w), random.randrange(cell_h))]
    visited[stack[0][1]][stack[0][0]] = True

    def center(cx, cy):
        return (1 + cx * step, 1 + cy * step)

    def paint_cell(cx, cy):
        x, y = center(cx, cy)
        for yy in range(y - 1, y + 2):
            row = mask[yy]
            for xx in range(x - 1, x + 2):
                row[xx] = True

    def paint_corridor(a, b):
        ax, ay = a
        bx, by = b
        if ax == bx:
            y0, y1 = (ay, by) if ay <= by else (by, ay)
            for y in range(y0, y1 + 1):
                for x in range(ax - 1, ax + 2):
                    mask[y][x] = True
            return
        x0, x1 = (ax, bx) if ax <= bx else (bx, ax)
        for x in range(x0, x1 + 1):
            for y in range(ay - 1, ay + 2):
                mask[y][x] = True

    while stack:
        cx, cy = stack[-1]
        paint_cell(cx, cy)
        neighbors = []
        for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            nx, ny = cx + dx, cy + dy
            if 0 <= nx < cell_w and 0 <= ny < cell_h and not visited[ny][nx]:
                neighbors.append((nx, ny))
        if not neighbors:
            stack.pop()
            continue
        nx, ny = random.choice(neighbors)
        visited[ny][nx] = True
        a = center(cx, cy)
        b = center(nx, ny)
        paint_corridor(a, b)
        stack.append((nx, ny))

def apply_mask_to_grid(mask):
    for y in range(1, height - 1):
        row = map_grid[y]
        mrow = mask[y]
        for x in range(1, width - 1):
            if mrow[x]:
                row[x] = ' '

def carve_rooms_into_mask(mask):
    for rx, ry, rw, rh in rooms:
        for y in range(ry, min(ry + rh, height - 1)):
            mrow = mask[y]
            for x in range(rx, min(rx + rw, width - 1)):
                mrow[x] = True
        cx = rx + rw // 2
        cy = ry + rh // 2
        for y in range(max(1, cy - 1), min(height - 1, cy + 2)):
            mrow = mask[y]
            for x in range(max(1, cx - 1), min(width - 1, cx + 2)):
                mrow[x] = True

def connect_rooms_via_mask(mask, connections_per_room=1):
    idxs = list(range(len(rooms)))
    random.shuffle(idxs)
    for idx in idxs:
        for _ in range(connections_per_room):
            x, y, w, h = rooms[idx]
            cx = x + w // 2
            cy = y + h // 2
            best = None
            best_d = 10**18
            for j, (rx, ry, rw, rh) in enumerate(rooms):
                if j == idx:
                    continue
                rcx = rx + rw // 2
                rcy = ry + rh // 2
                d = (cx - rcx) * (cx - rcx) + (cy - rcy) * (cy - rcy)
                if d < best_d:
                    best_d = d
                    best = (rcx, rcy)
            if best is None:
                continue
            add_corridor(cx, cy, best[0], best[1], thickness=3)

start_room_w = random.randint(6, 12)
start_room_h = random.randint(6, 12)
start_room_x = max(1, start_x - random.randint(0, start_room_w - 1))
start_room_y = max(1, start_y - random.randint(0, start_room_h - 1))

add_room(start_room_x, start_room_y, start_room_w, start_room_h)
rooms.append((start_room_x, start_room_y, start_room_w, start_room_h))

num_rooms = 20
attempts = 700
for _ in range(attempts):
    if len(rooms) >= num_rooms:
        break
    rw = random.randint(6, 12)
    rh = random.randint(6, 12)
    rx = random.randint(1, width - rw - 2)
    ry = random.randint(1, height - rh - 2)
    if room_overlaps(rx, ry, rw, rh, pad=3):
        continue
    add_room(rx, ry, rw, rh)
    rooms.append((rx, ry, rw, rh))

maze_mask = [[False for _ in range(width)] for _ in range(height)]
carve_maze(maze_mask, step=5)
carve_rooms_into_mask(maze_mask)
apply_mask_to_grid(maze_mask)

for i in range(1, len(rooms)):
    x1 = rooms[i - 1][0] + rooms[i - 1][2] // 2
    y1 = rooms[i - 1][1] + rooms[i - 1][3] // 2
    x2 = rooms[i][0] + rooms[i][2] // 2
    y2 = rooms[i][1] + rooms[i][3] // 2
    add_corridor(x1, y1, x2, y2, thickness=3)

extra_connections = max(20, len(rooms) // 2)
for _ in range(extra_connections):
    idx = random.randrange(len(rooms))
    connect_room_to_nearest(idx, thickness=3)

connect_rooms_via_mask(maze_mask, connections_per_room=1)

map_grid[start_y][start_x] = ' '

file_name = "assets/data/map.txt"
with open(file_name, "w", encoding="utf-8") as f:
    for row in map_grid:
        f.write("".join(row) + "\n")