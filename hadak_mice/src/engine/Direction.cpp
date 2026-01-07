#include "engine/Direction.h"

namespace hadak {

QVector<Direction> cardinalDirections() {
  return {Direction::North, Direction::East, Direction::South, Direction::West};
}

QVector<SemiDirection> semiDirections() {
  return {SemiDirection::East,      SemiDirection::NorthEast,
          SemiDirection::North,     SemiDirection::NorthWest,
          SemiDirection::West,      SemiDirection::SouthWest,
          SemiDirection::South,     SemiDirection::SouthEast};
}

Direction rotateLeft(Direction dir) {
  return static_cast<Direction>((static_cast<int>(dir) + 3) % 4);
}

Direction rotateRight(Direction dir) {
  return static_cast<Direction>((static_cast<int>(dir) + 1) % 4);
}

static SemiDirection rotateSemi(SemiDirection dir, int steps) {
  int idx = static_cast<int>(dir);
  int next = (idx + steps) % 8;
  if (next < 0) {
    next += 8;
  }
  return static_cast<SemiDirection>(next);
}

SemiDirection rotateLeft45(SemiDirection dir) { return rotateSemi(dir, 1); }

SemiDirection rotateRight45(SemiDirection dir) { return rotateSemi(dir, -1); }

SemiDirection rotateLeft90(SemiDirection dir) { return rotateSemi(dir, 2); }

SemiDirection rotateRight90(SemiDirection dir) { return rotateSemi(dir, -2); }

SemiDirection rotate180(SemiDirection dir) { return rotateSemi(dir, 4); }

bool isDiagonal(SemiDirection dir) {
  int idx = static_cast<int>(dir);
  return idx % 2 == 1;
}

bool toCardinal(SemiDirection dir, Direction *out) {
  if (isDiagonal(dir)) {
    return false;
  }
  int idx = static_cast<int>(dir);
  // Map: East=0, North=2, West=4, South=6
  if (idx == 0) {
    *out = Direction::East;
    return true;
  }
  if (idx == 2) {
    *out = Direction::North;
    return true;
  }
  if (idx == 4) {
    *out = Direction::West;
    return true;
  }
  if (idx == 6) {
    *out = Direction::South;
    return true;
  }
  return false;
}

QPair<int, int> deltaFor(SemiDirection dir) {
  switch (dir) {
    case SemiDirection::North:
      return {0, 1};
    case SemiDirection::South:
      return {0, -1};
    case SemiDirection::East:
      return {1, 0};
    case SemiDirection::West:
      return {-1, 0};
    case SemiDirection::NorthEast:
      return {1, 1};
    case SemiDirection::NorthWest:
      return {-1, 1};
    case SemiDirection::SouthEast:
      return {1, -1};
    case SemiDirection::SouthWest:
      return {-1, -1};
    default:
      return {0, 0};
  }
}

bool directionFromChar(QChar c, Direction *out) {
  if (c == 'n' || c == 'N') {
    *out = Direction::North;
    return true;
  }
  if (c == 'e' || c == 'E') {
    *out = Direction::East;
    return true;
  }
  if (c == 's' || c == 'S') {
    *out = Direction::South;
    return true;
  }
  if (c == 'w' || c == 'W') {
    *out = Direction::West;
    return true;
  }
  return false;
}

QChar directionToChar(Direction dir) {
  switch (dir) {
    case Direction::North:
      return 'n';
    case Direction::East:
      return 'e';
    case Direction::South:
      return 's';
    case Direction::West:
      return 'w';
    default:
      return '?';
  }
}

}  // namespace hadak
