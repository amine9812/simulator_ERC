#include "engine/Maze.h"

#include <algorithm>
#include <QFile>
#include <QQueue>
#include <QTextStream>

namespace hadak {

Maze::Maze(int width, int height) : m_width(width), m_height(height) {
  m_cells.resize(width);
  for (int x = 0; x < width; ++x) {
    m_cells[x].resize(height);
  }
}

int Maze::width() const { return m_width; }
int Maze::height() const { return m_height; }

bool Maze::inBounds(int x, int y) const {
  return x >= 0 && y >= 0 && x < m_width && y < m_height;
}

const Cell &Maze::cell(int x, int y) const { return m_cells[x][y]; }

Cell &Maze::cell(int x, int y) { return m_cells[x][y]; }

bool Maze::isWall(int x, int y, Direction dir) const {
  const Cell &c = cell(x, y);
  switch (dir) {
    case Direction::North:
      return c.north;
    case Direction::East:
      return c.east;
    case Direction::South:
      return c.south;
    case Direction::West:
      return c.west;
  }
  return true;
}

void Maze::setWall(int x, int y, Direction dir, bool present) {
  Cell &c = cell(x, y);
  switch (dir) {
    case Direction::North:
      c.north = present;
      break;
    case Direction::East:
      c.east = present;
      break;
    case Direction::South:
      c.south = present;
      break;
    case Direction::West:
      c.west = present;
      break;
  }
}

Maze *Maze::fromFile(const QString &path, QString *error) {
  if (path.isEmpty()) {
    if (error) {
      *error = "Empty path";
    }
    return nullptr;
  }
  QFile file(path);
  if (!file.open(QFile::ReadOnly)) {
    if (error) {
      *error = "Failed to open file";
    }
    return nullptr;
  }

  QStringList lines;
  QTextStream stream(&file);
  QString line;
  while (stream.readLineInto(&line)) {
    lines.append(line);
  }

  QString mapError;
  Maze *map = fromMapLines(lines, &mapError);
  if (map) {
    return map;
  }

  QString numError;
  Maze *num = fromNumLines(lines, &numError);
  if (num) {
    return num;
  }

  if (error) {
    *error = "Unsupported maze format";
  }
  return nullptr;
}

Maze *Maze::fromMapLines(const QStringList &lines, QString *error) {
  if (lines.isEmpty()) {
    if (error) {
      *error = "No map lines";
    }
    return nullptr;
  }

  QStringList flipped = lines;
  std::reverse(flipped.begin(), flipped.end());

  QVector<QVector<Cell>> cells;
  int height = flipped.size() / 2;
  int width = 0;
  if (height > 0) {
    width = flipped.at(0).size() / 4;
  }
  if (width <= 0 || height <= 0) {
    if (error) {
      *error = "Invalid map dimensions";
    }
    return nullptr;
  }

  cells.resize(width);
  for (int x = 0; x < width; ++x) {
    cells[x].resize(height);
  }

  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      int northLine = 2 * (y + 1);
      int southLine = 2 * y;
      int eastCol = 4 * (x + 1);
      int westCol = 4 * x;

      if (northLine >= flipped.size()) {
        if (error) {
          *error = "Map line out of range";
        }
        return nullptr;
      }
      if (southLine + 1 >= flipped.size()) {
        if (error) {
          *error = "Map line out of range";
        }
        return nullptr;
      }
      if (eastCol >= flipped.at(southLine + 1).size()) {
        if (error) {
          *error = "Map column out of range";
        }
        return nullptr;
      }
      if (westCol + 2 >= flipped.at(northLine).size()) {
        if (error) {
          *error = "Map column out of range";
        }
        return nullptr;
      }

      Cell c;
      c.north = flipped.at(northLine).at(westCol + 2) != ' ';
      c.south = flipped.at(southLine).at(westCol + 2) != ' ';
      c.east = flipped.at(southLine + 1).at(eastCol) != ' ';
      c.west = flipped.at(southLine + 1).at(westCol) != ' ';
      cells[x][y] = c;
    }
  }

  if (!isRectangular(cells) || !isEnclosed(cells) || !isConsistent(cells)) {
    if (error) {
      *error = "Invalid or inconsistent map";
    }
    return nullptr;
  }

  Maze *maze = new Maze(width, height);
  maze->m_cells = cells;
  return maze;
}

Maze *Maze::fromNumLines(const QStringList &lines, QString *error) {
  if (lines.isEmpty()) {
    if (error) {
      *error = "No num lines";
    }
    return nullptr;
  }

  QVector<QVector<Cell>> cells;
  int maxX = -1;
  int maxY = -1;

  for (const QString &line : lines) {
    QStringList tokens = line.split(" ", Qt::SkipEmptyParts);
    if (tokens.size() != 6) {
      if (error) {
        *error = "Invalid num line";
      }
      return nullptr;
    }
    bool ok = true;
    int x = tokens.at(0).toInt(&ok);
    int y = tokens.at(1).toInt(&ok);
    int n = tokens.at(2).toInt(&ok);
    int e = tokens.at(3).toInt(&ok);
    int s = tokens.at(4).toInt(&ok);
    int w = tokens.at(5).toInt(&ok);
    if (!ok) {
      if (error) {
        *error = "Invalid num values";
      }
      return nullptr;
    }
    if (x < 0 || y < 0) {
      if (error) {
        *error = "Negative coordinates";
      }
      return nullptr;
    }
    maxX = std::max(maxX, x);
    maxY = std::max(maxY, y);

    while (cells.size() <= x) {
      cells.append(QVector<Cell>());
    }
    while (cells[x].size() <= y) {
      cells[x].append(Cell());
    }
    Cell c;
    c.north = (n == 1);
    c.east = (e == 1);
    c.south = (s == 1);
    c.west = (w == 1);
    cells[x][y] = c;
  }

  if (maxX < 0 || maxY < 0) {
    if (error) {
      *error = "Empty num maze";
    }
    return nullptr;
  }

  if (!isRectangular(cells) || !isEnclosed(cells) || !isConsistent(cells)) {
    if (error) {
      *error = "Invalid or inconsistent num maze";
    }
    return nullptr;
  }

  Maze *maze = new Maze(maxX + 1, maxY + 1);
  maze->m_cells = cells;
  return maze;
}

QStringList Maze::toNumLines(const Maze &maze) {
  QStringList lines;
  for (int x = 0; x < maze.width(); ++x) {
    for (int y = 0; y < maze.height(); ++y) {
      const Cell &c = maze.cell(x, y);
      QString line = QString("%1 %2 %3 %4 %5 %6")
                         .arg(x)
                         .arg(y)
                         .arg(c.north ? 1 : 0)
                         .arg(c.east ? 1 : 0)
                         .arg(c.south ? 1 : 0)
                         .arg(c.west ? 1 : 0);
      lines.append(line);
    }
  }
  return lines;
}

bool Maze::isRectangular(const QVector<QVector<Cell>> &cells) {
  if (cells.isEmpty()) {
    return false;
  }
  int height = cells.at(0).size();
  if (height == 0) {
    return false;
  }
  for (int x = 1; x < cells.size(); ++x) {
    if (cells.at(x).size() != height) {
      return false;
    }
  }
  return true;
}

bool Maze::isEnclosed(const QVector<QVector<Cell>> &cells) {
  int width = cells.size();
  int height = cells.at(0).size();
  for (int x = 0; x < width; ++x) {
    for (int y = 0; y < height; ++y) {
      const Cell &c = cells.at(x).at(y);
      if (x == 0 && !c.west) {
        return false;
      }
      if (y == 0 && !c.south) {
        return false;
      }
      if (x == width - 1 && !c.east) {
        return false;
      }
      if (y == height - 1 && !c.north) {
        return false;
      }
    }
  }
  return true;
}

bool Maze::isConsistent(const QVector<QVector<Cell>> &cells) {
  int width = cells.size();
  int height = cells.at(0).size();
  for (int x = 0; x < width; ++x) {
    for (int y = 0; y < height; ++y) {
      const Cell &c = cells.at(x).at(y);
      if (x > 0 && c.west && !cells.at(x - 1).at(y).east) {
        return false;
      }
      if (y > 0 && c.south && !cells.at(x).at(y - 1).north) {
        return false;
      }
      if (x < width - 1 && c.east && !cells.at(x + 1).at(y).west) {
        return false;
      }
      if (y < height - 1 && c.north && !cells.at(x).at(y + 1).south) {
        return false;
      }
    }
  }
  return true;
}

bool Maze::isValid(QString *error) const {
  if (!isRectangular(m_cells)) {
    if (error) {
      *error = "Maze is not rectangular";
    }
    return false;
  }
  if (!isEnclosed(m_cells)) {
    if (error) {
      *error = "Maze is not enclosed";
    }
    return false;
  }
  if (!isConsistent(m_cells)) {
    if (error) {
      *error = "Maze walls are inconsistent";
    }
    return false;
  }
  return true;
}

QVector<QPair<int, int>> Maze::centerCells(int width, int height) {
  QPair<int, int> a = {(width - 1) / 2, (height - 1) / 2};
  QPair<int, int> b = {width / 2, (height - 1) / 2};
  QPair<int, int> c = {(width - 1) / 2, height / 2};
  QPair<int, int> d = {width / 2, height / 2};

  QVector<QPair<int, int>> positions;
  positions.append(a);
  if (width % 2 == 0 && height % 2 == 0) {
    positions.append(b);
    positions.append(c);
    positions.append(d);
  } else if (width % 2 == 0) {
    positions.append(b);
  } else if (height % 2 == 0) {
    positions.append(c);
  }
  return positions;
}

bool Maze::isCenter(int x, int y) const {
  return centerCells(m_width, m_height).contains({x, y});
}

QVector<QVector<int>> Maze::distancesToCenter() const {
  QVector<QVector<int>> distances;
  distances.resize(m_width);
  for (int x = 0; x < m_width; ++x) {
    distances[x].resize(m_height);
    for (int y = 0; y < m_height; ++y) {
      distances[x][y] = -1;
    }
  }

  QQueue<QPair<int, int>> queue;
  for (const auto &pos : centerCells(m_width, m_height)) {
    distances[pos.first][pos.second] = 0;
    queue.enqueue(pos);
  }

  while (!queue.isEmpty()) {
    QPair<int, int> pos = queue.dequeue();
    int x = pos.first;
    int y = pos.second;
    int base = distances[x][y];
    const Cell &c = cell(x, y);

    if (!c.north && y + 1 < m_height && distances[x][y + 1] == -1) {
      distances[x][y + 1] = base + 1;
      queue.enqueue({x, y + 1});
    }
    if (!c.east && x + 1 < m_width && distances[x + 1][y] == -1) {
      distances[x + 1][y] = base + 1;
      queue.enqueue({x + 1, y});
    }
    if (!c.south && y - 1 >= 0 && distances[x][y - 1] == -1) {
      distances[x][y - 1] = base + 1;
      queue.enqueue({x, y - 1});
    }
    if (!c.west && x - 1 >= 0 && distances[x - 1][y] == -1) {
      distances[x - 1][y] = base + 1;
      queue.enqueue({x - 1, y});
    }
  }

  return distances;
}

}  // namespace hadak
