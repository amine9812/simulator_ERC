#include "engine/MazeGenerator.h"

#include <QRandomGenerator>
#include <QStack>

namespace hadak {

struct CellVisit {
  int x;
  int y;
};

Maze *MazeGenerator::generate(int width, int height, quint32 seed) {
  if (width <= 0 || height <= 0) {
    return nullptr;
  }

  Maze *maze = new Maze(width, height);

  // Start with all walls present.
  for (int x = 0; x < width; ++x) {
    for (int y = 0; y < height; ++y) {
      maze->setWall(x, y, Direction::North, true);
      maze->setWall(x, y, Direction::East, true);
      maze->setWall(x, y, Direction::South, true);
      maze->setWall(x, y, Direction::West, true);
    }
  }

  QRandomGenerator rng(seed);
  QVector<QVector<bool>> visited(width, QVector<bool>(height, false));
  QStack<CellVisit> stack;

  int startX = rng.bounded(width);
  int startY = rng.bounded(height);
  visited[startX][startY] = true;
  stack.push({startX, startY});

  while (!stack.isEmpty()) {
    CellVisit current = stack.top();

    QVector<Direction> neighbors;
    if (current.y + 1 < height && !visited[current.x][current.y + 1]) {
      neighbors.append(Direction::North);
    }
    if (current.x + 1 < width && !visited[current.x + 1][current.y]) {
      neighbors.append(Direction::East);
    }
    if (current.y - 1 >= 0 && !visited[current.x][current.y - 1]) {
      neighbors.append(Direction::South);
    }
    if (current.x - 1 >= 0 && !visited[current.x - 1][current.y]) {
      neighbors.append(Direction::West);
    }

    if (neighbors.isEmpty()) {
      stack.pop();
      continue;
    }

    int pick = rng.bounded(neighbors.size());
    Direction dir = neighbors.at(pick);

    int nx = current.x;
    int ny = current.y;
    if (dir == Direction::North) {
      ny += 1;
    } else if (dir == Direction::East) {
      nx += 1;
    } else if (dir == Direction::South) {
      ny -= 1;
    } else if (dir == Direction::West) {
      nx -= 1;
    }

    // Knock down walls between current and next.
    maze->setWall(current.x, current.y, dir, false);
    Direction opposite = rotateLeft(rotateLeft(dir));
    maze->setWall(nx, ny, opposite, false);

    visited[nx][ny] = true;
    stack.push({nx, ny});
  }

  return maze;
}

}  // namespace hadak
