#!/usr/bin/env python3
"""Simple flood fill bot for Hadak Micromouse Studio."""

from collections import deque
import sys

DIRS = ["N", "E", "S", "W"]
DX = [0, 1, 0, -1]
DY = [1, 0, -1, 0]
INF = 10**9


def log(msg):
    print(msg, file=sys.stderr, flush=True)


def send(cmd, expect_reply=True):
    sys.stdout.write(cmd + "\n")
    sys.stdout.flush()
    if not expect_reply:
        return ""
    line = sys.stdin.readline()
    if not line:
        sys.exit(0)
    return line.strip()


def maze_width():
    return int(send("mazeWidth"))


def maze_height():
    return int(send("mazeHeight"))


def wall_front():
    return send("wallFront") == "true"


def wall_left():
    return send("wallLeft") == "true"


def wall_right():
    return send("wallRight") == "true"


def wall_back():
    return send("wallBack") == "true"


def move_forward():
    return send("moveForward") != "crash"


def turn_left():
    send("turnLeft")


def turn_right():
    send("turnRight")


def was_reset():
    return send("wasReset") == "true"


def ack_reset():
    send("ackReset")


def set_wall(x, y, direction):
    send(f"setWall {x} {y} {direction}", expect_reply=False)


def clear_wall(x, y, direction):
    send(f"clearWall {x} {y} {direction}", expect_reply=False)


def center_cells(width, height):
    xs = [width // 2]
    ys = [height // 2]
    if width % 2 == 0:
        xs = [width // 2 - 1, width // 2]
    if height % 2 == 0:
        ys = [height // 2 - 1, height // 2]
    return {(x, y) for x in xs for y in ys}


def in_bounds(x, y, width, height):
    return 0 <= x < width and 0 <= y < height


def compute_distances(width, height, walls, goals):
    dist = [[INF for _ in range(height)] for _ in range(width)]
    q = deque()
    for gx, gy in goals:
        dist[gx][gy] = 0
        q.append((gx, gy))
    while q:
        x, y = q.popleft()
        for d in range(4):
            if walls[x][y][d] is True:
                continue
            nx, ny = x + DX[d], y + DY[d]
            if not in_bounds(nx, ny, width, height):
                continue
            if dist[nx][ny] > dist[x][y] + 1:
                dist[nx][ny] = dist[x][y] + 1
                q.append((nx, ny))
    return dist


def main():
    width = maze_width()
    height = maze_height()
    goals = center_cells(width, height)

    walls = [[[None for _ in range(4)] for _ in range(height)] for _ in range(width)]

    for x in range(width):
        walls[x][0][2] = True
        walls[x][height - 1][0] = True
        set_wall(x, 0, "s")
        set_wall(x, height - 1, "n")
    for y in range(height):
        walls[0][y][3] = True
        walls[width - 1][y][1] = True
        set_wall(0, y, "w")
        set_wall(width - 1, y, "e")

    x, y = 0, 0
    direction = 0

    while True:
        if was_reset():
            ack_reset()
            x, y = 0, 0
            direction = 0
            continue

        sensed = {
            (direction + 0) % 4: wall_front(),
            (direction + 3) % 4: wall_left(),
            (direction + 1) % 4: wall_right(),
            (direction + 2) % 4: wall_back(),
        }
        for d, is_wall in sensed.items():
            if walls[x][y][d] is None:
                if is_wall:
                    set_wall(x, y, DIRS[d].lower())
                else:
                    clear_wall(x, y, DIRS[d].lower())
            walls[x][y][d] = is_wall
            nx, ny = x + DX[d], y + DY[d]
            if in_bounds(nx, ny, width, height):
                walls[nx][ny][(d + 2) % 4] = is_wall

        if (x, y) in goals:
            log("Reached goal")
            return

        dist = compute_distances(width, height, walls, goals)
        preferences = [direction, (direction + 3) % 4, (direction + 1) % 4, (direction + 2) % 4]
        best_dir = None
        best_dist = INF
        for d in preferences:
            if walls[x][y][d] is True:
                continue
            nx, ny = x + DX[d], y + DY[d]
            if not in_bounds(nx, ny, width, height):
                continue
            if dist[nx][ny] < best_dist:
                best_dist = dist[nx][ny]
                best_dir = d

        if best_dir is None:
            log("No moves left")
            return

        diff = (best_dir - direction) % 4
        if diff == 1:
            turn_right()
        elif diff == 3:
            turn_left()
        elif diff == 2:
            turn_right()
            turn_right()
        direction = best_dir

        if not move_forward():
            log("Crash")
            return

        x += DX[direction]
        y += DY[direction]


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        log(f"Fatal error: {exc}")
        raise
