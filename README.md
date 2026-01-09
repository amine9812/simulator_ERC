# Hadak Micromouse Studio (v1.11.34)

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

## Controller API 

Bots communicate via stdin/stdout. Valid commands include:

- `mazeWidth`, `mazeHeight`
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

- `.map` ASCII format 
- `.num` format: `X Y N E S W` (1 = wall, 0 = open)

## Project Structure

- `src/engine`: maze, mouse, movement rules, stats
- `src/controller`: bot process + command protocol
- `src/ui`: rendering and widgets
- `tests`: engine tests
- `controller/bots`: example bot scripts


