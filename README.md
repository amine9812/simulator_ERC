<<<<<<< HEAD
# Hadak Micromouse Studio (v1.11.34)
=======
# Hadak Micromouse Studio (v2)

A fresh UI + architecture for a Micromouse simulator that preserves the same
controller protocol and maze formats as the original mms project.

## Features

- Same bot/controller API and maze formats as the original simulator
- New UI layout with live debug panel, overlays, and event log
- Step/Play/Pause controls + speed slider
- Load/save mazes in `.map` / `.num` formats
- Random maze generation with deterministic seed
- Click-to-edit maze walls
- Example flood-fill bot
- Core engine tests (movement, collision, parsing, generation)
>>>>>>> ea7b6d2 (Add Hadak micromouse simulator v2)

## Prerequisites (Ubuntu)

- Qt 6 (qmake + Qt Widgets)
- g++ and build essentials
- OpenGL headers (libgl1-mesa-dev)

Example (apt):

```bash
sudo apt-get update
sudo apt-get install -y qt6-base-dev qt6-base-dev-tools libgl1-mesa-dev
```

If you installed Qt via the Qt installer, make sure the correct `qmake` is on your PATH,
for example:

```bash
export PATH=$HOME/Qt/6.10.1/gcc_64/bin:$PATH
```

## Build (Ubuntu)

```bash
cd /home/amine/Documents/hadak_1/hadak_mice
qmake
make
```

Binary will be at:

```
/home/amine/Documents/hadak_1/hadak_mice/bin/hadak_mice
```

## Run

```bash
/home/amine/Documents/hadak_1/hadak_mice/bin/hadak_mice
```

## Tests

```bash
cd /home/amine/Documents/hadak_1/hadak_mice/tests
qmake
make
../bin/tests
```

<<<<<<< HEAD
## Controller API 
=======
## Controller API (same as original)
>>>>>>> ea7b6d2 (Add Hadak micromouse simulator v2)

Bots communicate via stdin/stdout. Valid commands include:

- `mazeWidth`, `mazeHeight`
<<<<<<< HEAD
=======
- `goalCount`, `goalCell <index>`, `isGoal`
>>>>>>> ea7b6d2 (Add Hadak micromouse simulator v2)
- `wallFront [N]`, `wallLeft [N]`, `wallRight [N]`, `wallBack [N]`
- `wallFrontLeft [N]`, `wallFrontRight [N]`, `wallBackLeft [N]`, `wallBackRight [N]`
- `moveForward [N]`, `moveForwardHalf [N]`
- `turnLeft`, `turnRight`, `turnLeft45`, `turnRight45`
- `setWall x y d`, `clearWall x y d`
- `setColor x y c`, `clearColor x y`, `clearAllColor`
- `setText x y text`, `clearText x y`, `clearAllText`
- `wasReset`, `ackReset`
- `getStat <stat>`

Commands that return responses should wait for the response line before sending
another command.

## Writing a bot (example)

There is an example flood-fill bot:

```
/home/amine/Documents/hadak_1/hadak_mice/controller/bots/flood_fill.py
```

In the UI, set:

- Command: `python3 controller/bots/flood_fill.py`
- Working directory: `/home/amine/Documents/hadak_1/hadak_mice`

Then click **Start Bot** and **Play**.

## Maze formats

<<<<<<< HEAD
- `.map` ASCII format 
=======
- `.map` ASCII format (same as the original mms)
>>>>>>> ea7b6d2 (Add Hadak micromouse simulator v2)
- `.num` format: `X Y N E S W` (1 = wall, 0 = open)

## Project Structure

- `src/engine`: maze, mouse, movement rules, stats
- `src/controller`: bot process + command protocol
- `src/ui`: rendering and widgets
- `tests`: engine tests
- `controller/bots`: example bot scripts

<<<<<<< HEAD

=======
## Notes on Compatibility

This simulator follows the same movement rules and command protocol as the
original mms project, including half-step movement and diagonal turns. Maze
parsing logic and wall consistency checks are modeled after the original.

## UI Tips

- Use **Edit action** to switch between wall editing, setting the start cell, or
  setting the goal cell by clicking on the maze.
- The **Bot Runner** dropdown lists all scripts in `controller/bots`. Pick one
  and the command/directory fields are filled automatically.
- The simulator pauses and stops the bot when the goal is reached (success log).

## Included Bots

- `controller/bots/flood_fill.py`
- `controller/bots/pledge.py`
- `controller/bots/right_wall.py`
- `controller/bots/left_wall.py`
- `controller/bots/random_walk.py`

Use the Bot Runner dropdown to pick one; the command and directory fields fill in automatically.
>>>>>>> ea7b6d2 (Add Hadak micromouse simulator v2)
