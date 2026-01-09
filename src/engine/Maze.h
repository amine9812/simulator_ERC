#pragma once

#include <QPair>
#include <QString>
#include <QStringList>
#include <QVector>

#include "engine/Direction.h"

namespace hadak {

struct Cell {
  bool north = false;
  bool east = false;
  bool south = false;
  bool west = false;
};

class Maze {
 public:
  static Maze *fromFile(const QString &path, QString *error);
  static Maze *fromMapLines(const QStringList &lines, QString *error);
  static Maze *fromNumLines(const QStringList &lines, QString *error);
  static QStringList toNumLines(const Maze &maze);

  Maze(int width, int height);

  int width() const;
  int height() const;

  bool inBounds(int x, int y) const;
  const Cell &cell(int x, int y) const;
  Cell &cell(int x, int y);

  bool isWall(int x, int y, Direction dir) const;
  void setWall(int x, int y, Direction dir, bool present);

  QVector<QVector<int>> distancesToCenter() const;
  bool isCenter(int x, int y) const;
  static QVector<QPair<int, int>> centerCells(int width, int height);

  bool isValid(QString *error) const;

 private:
  int m_width = 0;
  int m_height = 0;
  QVector<QVector<Cell>> m_cells;

  static bool isRectangular(const QVector<QVector<Cell>> &cells);
  static bool isEnclosed(const QVector<QVector<Cell>> &cells);
  static bool isConsistent(const QVector<QVector<Cell>> &cells);
};

}  // namespace hadak
