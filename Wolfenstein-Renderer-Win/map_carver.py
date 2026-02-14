import random

width, height = 128, 128
map_grid = [['#' for _ in range(width)] for _ in range(height)]

start_x, start_y = 2, 2

rooms = []

def add_room(x, y, w, h):
    for i in range(y, min(y+h, height-1)):
        for j in range(x, min(x+w, width-1)):
            map_grid[i][j] = ' '

def room_overlaps(x, y, w, h):
    for rx, ry, rw, rh in rooms:
        if (x < rx + rw + 1 and x + w + 1 > rx and
            y < ry + rh + 1 and y + h + 1 > ry):
            return True
    return False

def add_corridor(x1, y1, x2, y2, thickness=3):
    if x1 == x2 or y1 == y2:
        for i in range(max(min(y1,y2)-thickness//2,0), min(max(y1,y2)+thickness//2+1, height)):
            for j in range(max(min(x1,x2)-thickness//2,0), min(max(x1,x2)+thickness//2+1, width)):
                map_grid[i][j] = ' '
    else:
        mid_x = x2
        mid_y = y1
        add_corridor(x1, y1, mid_x, mid_y, thickness)
        add_corridor(mid_x, mid_y, x2, y2, thickness)

start_room_w = random.randint(10, 30)
start_room_h = random.randint(10, 30)
start_room_x = max(1, start_x - random.randint(0, start_room_w - 1))
start_room_y = max(1, start_y - random.randint(0, start_room_h - 1))

add_room(start_room_x, start_room_y, start_room_w, start_room_h)
rooms.append((start_room_x, start_room_y, start_room_w, start_room_h))

num_rooms = 30
for _ in range(num_rooms):
    rw = random.randint(10, 30)
    rh = random.randint(10, 30)
    rx = random.randint(1, width - rw - 1)
    ry = random.randint(1, height - rh - 1)
    
    if room_overlaps(rx, ry, rw, rh):
        continue

    add_room(rx, ry, rw, rh)
    rooms.append((rx, ry, rw, rh))

for i in range(1, len(rooms)):
    x1 = rooms[i-1][0] + rooms[i-1][2] // 2
    y1 = rooms[i-1][1] + rooms[i-1][3] // 2
    x2 = rooms[i][0] + rooms[i][2] // 2
    y2 = rooms[i][1] + rooms[i][3] // 2
    add_corridor(x1, y1, x2, y2, thickness=3)

map_grid[start_y][start_x] = ' ' #player pos duuuh

file_name = "assets/data/map.txt"
with open(file_name, "w") as f:
    for row in map_grid:
        f.write("".join(row) + "\n")