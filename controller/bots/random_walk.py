#!/usr/bin/env python3
"""Random walk bot with a mild forward bias."""

import random
import sys


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


def main():
    while True:
        if was_reset():
            ack_reset()
            continue

        if is_goal():
            log("Goal reached")
            return

        options = []
        if not wall_front():
            options.extend(["forward"] * 3)
        if not wall_left():
            options.append("left")
        if not wall_right():
            options.append("right")
        if not options:
            options.append("back")

        choice = random.choice(options)
        if choice == "left":
            turn_left()
        elif choice == "right":
            turn_right()
        elif choice == "back":
            turn_right()
            turn_right()

        if not move_forward():
            log("Crash moving")
            return


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        log(f"Fatal error: {exc}")
        raise
