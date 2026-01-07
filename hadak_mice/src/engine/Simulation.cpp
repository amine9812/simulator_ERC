#include "engine/Simulation.h"

#include <QtMath>

namespace hadak {

Simulation::Simulation(QObject *parent) : QObject(parent) {}

void Simulation::setMaze(std::unique_ptr<Maze> maze) {
  m_maze = std::move(maze);
  reset();
}

Maze *Simulation::maze() const { return m_maze.get(); }

void Simulation::reset() {
  m_mouse.reset();
  m_stats.resetAll();
  m_movement = {};
  m_resetRequested = false;
  m_goalReached = false;
  m_stepCount = 0;
  m_collisionCount = 0;
  m_visitedCells.clear();
  initKnowledge();
  markVisited();
  emit stateChanged();
}

void Simulation::requestReset() { m_resetRequested = true; }

bool Simulation::wasReset() const { return m_resetRequested; }

void Simulation::ackReset() {
  m_mouse.reset();
  m_movement = {};
  m_resetRequested = false;
  m_goalReached = false;
  m_stats.penalizeForReset();
  m_stats.endUnfinishedRun();
  m_stepCount = 0;
  logEvent("Reset acknowledged");
  emit stateChanged();
}

bool Simulation::requestMove(int numHalfSteps) {
  if (!m_maze) {
    return false;
  }
  if (numHalfSteps < 1) {
    return false;
  }
  if (isWallFront(0)) {
    return false;
  }

  int allowed = 1;
  while (allowed < numHalfSteps) {
    if (isWallFront(allowed)) {
      break;
    }
    allowed += 1;
  }

  m_movement.doomed = (allowed != numHalfSteps);
  m_movement.halfStepsRemaining = allowed;
  m_movement.movement = isDiagonal(m_mouse.heading())
                            ? Movement::MoveDiagonal
                            : Movement::MoveStraight;

  QPair<int, int> cell = m_mouse.position().toCell();
  if (cell.first == 0 && cell.second == 0) {
    m_stats.startRun();
  }
  m_stats.addDistance(numHalfSteps);
  logEvent(QString("Move %1 half-steps").arg(numHalfSteps));
  emit stateChanged();
  return true;
}

void Simulation::requestTurn(Movement movement) {
  if (movement == Movement::None || movement == Movement::MoveStraight ||
      movement == Movement::MoveDiagonal) {
    return;
  }
  m_movement.movement = movement;
  m_movement.halfStepsRemaining = 0;
  m_movement.doomed = false;
  m_stats.addTurn();
  logEvent("Turn requested");
  emit stateChanged();
}

bool Simulation::isMoving() const { return m_movement.movement != Movement::None; }

void Simulation::advanceOneTick() {
  if (!m_maze) {
    return;
  }
  if (m_movement.movement == Movement::None) {
    return;
  }

  if (m_movement.movement == Movement::TurnLeft45 ||
      m_movement.movement == Movement::TurnRight45 ||
      m_movement.movement == Movement::TurnLeft90 ||
      m_movement.movement == Movement::TurnRight90) {
    SemiDirection heading = m_mouse.heading();
    if (m_movement.movement == Movement::TurnLeft45) {
      heading = rotateLeft45(heading);
    } else if (m_movement.movement == Movement::TurnRight45) {
      heading = rotateRight45(heading);
    } else if (m_movement.movement == Movement::TurnLeft90) {
      heading = rotateLeft90(heading);
    } else if (m_movement.movement == Movement::TurnRight90) {
      heading = rotateRight90(heading);
    }
    m_mouse.setHeading(heading);
    m_movement.movement = Movement::None;
    emit movementFinished(false);
    emit stateChanged();
    return;
  }

  if (m_movement.halfStepsRemaining > 0) {
    QPair<int, int> delta = deltaFor(m_mouse.heading());
    SemiPosition pos = m_mouse.position();
    pos.x += delta.first;
    pos.y += delta.second;
    m_mouse.setPosition(pos);
    m_movement.halfStepsRemaining -= 1;
    m_stepCount += 1;
    markVisited();
  }

  if (m_movement.halfStepsRemaining == 0) {
    bool crashed = m_movement.doomed;
    if (crashed) {
      m_collisionCount += 1;
      logEvent("Collision");
    }
    m_movement = {};
    emit movementFinished(crashed);
  }

  emit stateChanged();
}

bool Simulation::isWallFront(int halfStepsAhead) const {
  return isWallAt(m_mouse.position(), m_mouse.heading(), halfStepsAhead);
}

bool Simulation::isWallLeft(int halfStepsAhead) const {
  return isWallAt(m_mouse.position(), rotateLeft90(m_mouse.heading()),
                  halfStepsAhead);
}

bool Simulation::isWallRight(int halfStepsAhead) const {
  return isWallAt(m_mouse.position(), rotateRight90(m_mouse.heading()),
                  halfStepsAhead);
}

bool Simulation::isWallBack(int halfStepsAhead) const {
  return isWallAt(m_mouse.position(), rotate180(m_mouse.heading()),
                  halfStepsAhead);
}

bool Simulation::isWallFrontLeft(int halfStepsAhead) const {
  return isWallAt(m_mouse.position(), rotateLeft45(m_mouse.heading()),
                  halfStepsAhead);
}

bool Simulation::isWallFrontRight(int halfStepsAhead) const {
  return isWallAt(m_mouse.position(), rotateRight45(m_mouse.heading()),
                  halfStepsAhead);
}

bool Simulation::isWallBackLeft(int halfStepsAhead) const {
  return isWallAt(m_mouse.position(),
                  rotateLeft90(rotateLeft45(m_mouse.heading())),
                  halfStepsAhead);
}

bool Simulation::isWallBackRight(int halfStepsAhead) const {
  return isWallAt(m_mouse.position(),
                  rotateRight90(rotateRight45(m_mouse.heading())),
                  halfStepsAhead);
}

const Mouse &Simulation::mouse() const { return m_mouse; }
Mouse &Simulation::mouse() { return m_mouse; }

const Stats &Simulation::stats() const { return m_stats; }
Stats &Simulation::stats() { return m_stats; }

int Simulation::collisionCount() const { return m_collisionCount; }
int Simulation::stepCount() const { return m_stepCount; }

bool Simulation::goalReached() const { return m_goalReached; }

WallState Simulation::knownWall(int x, int y, Direction dir) const {
  if (!m_maze || !m_maze->inBounds(x, y)) {
    return WallState::Unknown;
  }
  return m_knownWalls[x][y][static_cast<int>(dir)];
}

void Simulation::setKnownWall(int x, int y, Direction dir, WallState state) {
  if (!m_maze || !m_maze->inBounds(x, y)) {
    return;
  }
  m_knownWalls[x][y][static_cast<int>(dir)] = state;
  emit stateChanged();
}

bool Simulation::cellVisited(int x, int y) const {
  return m_visitedCells.contains({x, y});
}

const QSet<QPair<int, int>> &Simulation::visitedCells() const {
  return m_visitedCells;
}

QChar Simulation::cellColor(int x, int y) const {
  if (!m_maze || !m_maze->inBounds(x, y)) {
    return QChar();
  }
  return m_cellColors[x][y];
}

void Simulation::setCellColor(int x, int y, QChar color) {
  if (!m_maze || !m_maze->inBounds(x, y)) {
    return;
  }
  m_cellColors[x][y] = color;
  emit stateChanged();
}

void Simulation::clearCellColor(int x, int y) {
  if (!m_maze || !m_maze->inBounds(x, y)) {
    return;
  }
  m_cellColors[x][y] = QChar();
  emit stateChanged();
}

void Simulation::clearAllColors() {
  if (!m_maze) {
    return;
  }
  for (int x = 0; x < m_maze->width(); ++x) {
    for (int y = 0; y < m_maze->height(); ++y) {
      m_cellColors[x][y] = QChar();
    }
  }
  emit stateChanged();
}

QString Simulation::cellText(int x, int y) const {
  if (!m_maze || !m_maze->inBounds(x, y)) {
    return QString();
  }
  return m_cellText[x][y];
}

void Simulation::setCellText(int x, int y, const QString &text) {
  if (!m_maze || !m_maze->inBounds(x, y)) {
    return;
  }
  m_cellText[x][y] = text;
  emit stateChanged();
}

void Simulation::clearCellText(int x, int y) {
  if (!m_maze || !m_maze->inBounds(x, y)) {
    return;
  }
  m_cellText[x][y].clear();
  emit stateChanged();
}

void Simulation::clearAllText() {
  if (!m_maze) {
    return;
  }
  for (int x = 0; x < m_maze->width(); ++x) {
    for (int y = 0; y < m_maze->height(); ++y) {
      m_cellText[x][y].clear();
    }
  }
  emit stateChanged();
}

void Simulation::initKnowledge() {
  if (!m_maze) {
    return;
  }
  m_knownWalls.resize(m_maze->width());
  m_cellColors.resize(m_maze->width());
  m_cellText.resize(m_maze->width());
  for (int x = 0; x < m_maze->width(); ++x) {
    m_knownWalls[x].resize(m_maze->height());
    m_cellColors[x].resize(m_maze->height());
    m_cellText[x].resize(m_maze->height());
    for (int y = 0; y < m_maze->height(); ++y) {
      m_knownWalls[x][y].resize(4);
      for (int d = 0; d < 4; ++d) {
        m_knownWalls[x][y][d] = WallState::Unknown;
      }
      m_cellColors[x][y] = QChar();
      m_cellText[x][y].clear();
    }
  }
}

void Simulation::logEvent(const QString &message) {
  emit eventLogged(message);
}

bool Simulation::isWallAt(const SemiPosition &pos, SemiDirection dir) const {
  if (!m_maze) {
    return true;
  }
  int maxX = m_maze->width() * 2;
  int maxY = m_maze->height() * 2;

  if (pos.x < 0 || pos.y < 0 || pos.x > maxX || pos.y > maxY) {
    return true;
  }

  int cellX = pos.toCell().first;
  int cellY = pos.toCell().second;

  bool xEven = (pos.x % 2 == 0);
  bool yEven = (pos.y % 2 == 0);

  if (xEven && yEven) {
    return true;
  }

  if (!xEven && !yEven) {
    if (isDiagonal(dir)) {
      return true;
    }
    Direction cardinal;
    if (toCardinal(dir, &cardinal)) {
      return m_maze->isWall(cellX, cellY, cardinal);
    }
    return true;
  }

  if (xEven && !yEven) {
    if (dir == SemiDirection::North || dir == SemiDirection::South) {
      return true;
    }
    if (dir == SemiDirection::East || dir == SemiDirection::West) {
      return false;
    }
    if (dir == SemiDirection::NorthEast) {
      if (pos.x == maxX) {
        return false;
      }
      return m_maze->isWall(cellX, cellY, Direction::North);
    }
    if (dir == SemiDirection::SouthEast) {
      if (pos.x == maxX) {
        return false;
      }
      return m_maze->isWall(cellX, cellY, Direction::South);
    }
    if (dir == SemiDirection::NorthWest) {
      if (pos.x == 0) {
        return false;
      }
      return m_maze->isWall(cellX - 1, cellY, Direction::North);
    }
    if (dir == SemiDirection::SouthWest) {
      if (pos.x == 0) {
        return false;
      }
      return m_maze->isWall(cellX - 1, cellY, Direction::South);
    }
  }

  if (!xEven && yEven) {
    if (dir == SemiDirection::East || dir == SemiDirection::West) {
      return true;
    }
    if (dir == SemiDirection::North || dir == SemiDirection::South) {
      return false;
    }
    if (dir == SemiDirection::NorthEast) {
      if (pos.y == maxY) {
        return false;
      }
      return m_maze->isWall(cellX, cellY, Direction::East);
    }
    if (dir == SemiDirection::NorthWest) {
      if (pos.y == maxY) {
        return false;
      }
      return m_maze->isWall(cellX, cellY, Direction::West);
    }
    if (dir == SemiDirection::SouthEast) {
      if (pos.y == 0) {
        return false;
      }
      return m_maze->isWall(cellX, cellY - 1, Direction::East);
    }
    if (dir == SemiDirection::SouthWest) {
      if (pos.y == 0) {
        return false;
      }
      return m_maze->isWall(cellX, cellY - 1, Direction::West);
    }
  }

  return true;
}

bool Simulation::isWallAt(const SemiPosition &pos, SemiDirection dir,
                          int halfStepsAhead) const {
  if (isWallAt(pos, dir)) {
    return true;
  }
  SemiPosition cursor = pos;
  for (int i = 1; i <= halfStepsAhead; ++i) {
    QPair<int, int> delta = deltaFor(dir);
    cursor.x += delta.first;
    cursor.y += delta.second;
    if (isWallAt(cursor, dir)) {
      return true;
    }
  }
  return false;
}

void Simulation::markVisited() {
  if (!m_maze) {
    return;
  }
  QPair<int, int> cell = m_mouse.position().toCell();
  if (!m_maze->inBounds(cell.first, cell.second)) {
    return;
  }
  m_visitedCells.insert(cell);
  if (m_maze->isCenter(cell.first, cell.second)) {
    if (!m_goalReached) {
      m_goalReached = true;
      m_stats.finishRun();
      logEvent("Goal reached");
    }
  } else if (cell.first == 0 && cell.second == 0) {
    m_stats.endUnfinishedRun();
  }
}

}  // namespace hadak
