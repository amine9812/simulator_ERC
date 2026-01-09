#pragma once

#include <QChar>
#include <QPair>
#include <QVector>

namespace hadak {

enum class Direction { North = 0, East = 1, South = 2, West = 3 };

enum class SemiDirection {
  East = 0,
  NorthEast = 1,
  North = 2,
  NorthWest = 3,
  West = 4,
  SouthWest = 5,
  South = 6,
  SouthEast = 7,
};

QVector<Direction> cardinalDirections();
QVector<SemiDirection> semiDirections();

Direction rotateLeft(Direction dir);
Direction rotateRight(Direction dir);

SemiDirection rotateLeft45(SemiDirection dir);
SemiDirection rotateRight45(SemiDirection dir);
SemiDirection rotateLeft90(SemiDirection dir);
SemiDirection rotateRight90(SemiDirection dir);
SemiDirection rotate180(SemiDirection dir);

bool isDiagonal(SemiDirection dir);

bool toCardinal(SemiDirection dir, Direction *out);

QPair<int, int> deltaFor(SemiDirection dir);

bool directionFromChar(QChar c, Direction *out);
QChar directionToChar(Direction dir);

}  // namespace hadak
