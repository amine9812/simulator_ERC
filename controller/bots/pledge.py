#!/usr/bin/env python3
"""Pledge algorithm bot (right-hand wall follow) for Hadak Micromouse Studio."""

import sys

DX = [0, 1, 0, -1]
DY = [1, 0, -1, 0]


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


def is_goal():
    return send("isGoal") == "true"


def turn_to(direction, target):
    diff = (target - direction) % 4
    if diff == 1:
        turn_right()
        direction = (direction + 1) % 4
    elif diff == 3:
        turn_left()
        direction = (direction + 3) % 4
    elif diff == 2:
        turn_right()
        turn_right()
        direction = (direction + 2) % 4
    return direction


def main():
    width = maze_width()
    height = maze_height()
    max_steps = max(200, width * height * 16)

    # 0=N, 1=E, 2=S, 3=W
    preferred_dir = 0
    direction = 0
    mode = "forward"
    turn_sum = 0  # left = +1, right = -1

    x = 0
    y = 0
    steps = 0

    def advance_position():
        nonlocal x, y, steps
        x += DX[direction]
        y += DY[direction]
        steps += 1
        if steps >= max_steps:
            log("Step limit reached, stopping")
            return True
        return False

    while True:
        if was_reset():
            ack_reset()
            direction = 0
            mode = "forward"
            turn_sum = 0
            x = 0
            y = 0
            steps = 0
            continue

        if is_goal():
            log("Goal reached")
            return

        if mode == "forward":
            # Face preferred direction when possible
            if direction != preferred_dir:
                direction = turn_to(direction, preferred_dir)

            if wall_front():
                mode = "wall_follow"
                turn_sum = 0
                continue

            if not move_forward():
                log("Crash while moving forward")
                return
            if advance_position():
                return
            continue

        # Wall-following mode (right-hand rule)
        if turn_sum == 0 and direction == preferred_dir and not wall_front():
            mode = "forward"
            continue

        if not wall_right():
            turn_right()
            direction = (direction + 1) % 4
            turn_sum -= 1
            if not move_forward():
                log("Crash after right turn")
                return
            if advance_position():
                return
        elif not wall_front():
            if not move_forward():
                log("Crash moving forward")
                return
            if advance_position():
                return
        elif not wall_left():
            turn_left()
            direction = (direction + 3) % 4
            turn_sum += 1
            if not move_forward():
                log("Crash after left turn")
                return
            if advance_position():
                return
        else:
            # Turn around (two rights)
            turn_right()
            turn_right()
            direction = (direction + 2) % 4
            turn_sum -= 2
            if not move_forward():
                log("Crash after U-turn")
                return
            if advance_position():
                return


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        log(f"Fatal error: {exc}")
        raise
