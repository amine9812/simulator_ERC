#pragma once

#include <QObject>
#include <QSet>
#include <QString>
#include <QVector>
#include <memory>

#include "engine/Direction.h"
#include "engine/Maze.h"
#include "engine/Mouse.h"
#include "engine/Stats.h"

namespace hadak {

enum class WallState { Unknown, Open, Wall };

enum class Movement {
  None,
  MoveStraight,
  MoveDiagonal,
  TurnLeft45,
  TurnRight45,
  TurnLeft90,
  TurnRight90,
};

struct MovementState {
  Movement movement = Movement::None;
  int halfStepsRemaining = 0;
  bool doomed = false;
};

class Simulation : public QObject {
  Q_OBJECT

 public:
  explicit Simulation(QObject *parent = nullptr);

  void setMaze(std::unique_ptr<Maze> maze);
  Maze *maze() const;

  void reset();
  void requestReset();
  bool wasReset() const;
  void ackReset();

  bool requestMove(int numHalfSteps);
  void requestTurn(Movement movement);

  bool isMoving() const;
  void advanceOneTick();

  bool isWallFront(int halfStepsAhead) const;
  bool isWallLeft(int halfStepsAhead) const;
  bool isWallRight(int halfStepsAhead) const;
  bool isWallBack(int halfStepsAhead) const;
  bool isWallFrontLeft(int halfStepsAhead) const;
  bool isWallFrontRight(int halfStepsAhead) const;
  bool isWallBackLeft(int halfStepsAhead) const;
  bool isWallBackRight(int halfStepsAhead) const;

  const Mouse &mouse() const;
  Mouse &mouse();

  const Stats &stats() const;
  Stats &stats();

  int collisionCount() const;
  int stepCount() const;
  bool goalReached() const;

  QPair<int, int> startCell() const;
  QSet<QPair<int, int>> goalCells() const;
  void setStartCell(int x, int y);
  void setGoalCell(int x, int y);

  WallState knownWall(int x, int y, Direction dir) const;
  void setKnownWall(int x, int y, Direction dir, WallState state);

  bool cellVisited(int x, int y) const;
  const QSet<QPair<int, int>> &visitedCells() const;

  QChar cellColor(int x, int y) const;
  void setCellColor(int x, int y, QChar color);
  void clearCellColor(int x, int y);
  void clearAllColors();

  QString cellText(int x, int y) const;
  void setCellText(int x, int y, const QString &text);
  void clearCellText(int x, int y);
  void clearAllText();

 signals:
  void stateChanged();
  void movementFinished(bool crashed);
  void eventLogged(const QString &message);

 private:
  std::unique_ptr<Maze> m_maze;
  Mouse m_mouse;
  Stats m_stats;
  MovementState m_movement;
  bool m_resetRequested = false;
  bool m_goalReached = false;
  int m_stepCount = 0;
  int m_collisionCount = 0;

  QVector<QVector<QVector<WallState>>> m_knownWalls;
  QSet<QPair<int, int>> m_visitedCells;
  QVector<QVector<QChar>> m_cellColors;
  QVector<QVector<QString>> m_cellText;

  QPair<int, int> m_startCell = {0, 0};
  QSet<QPair<int, int>> m_goalCells;

  void initKnowledge();
  void logEvent(const QString &message);

  bool isWallAt(const SemiPosition &pos, SemiDirection dir) const;
  bool isWallAt(const SemiPosition &pos, SemiDirection dir,
                int halfStepsAhead) const;

  void markVisited();
  void setMouseToStart();
};

}  // namespace hadak
