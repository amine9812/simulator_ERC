#include "controller/SimController.h"

#include <algorithm>

namespace hadak {

SimController::SimController(Simulation *sim, QObject *parent)
    : QObject(parent), m_sim(sim) {
  connect(m_sim, &Simulation::movementFinished, this,
          &SimController::onMovementFinished);
}

void SimController::attachBot(BotProcess *bot) { m_bot = bot; }

void SimController::setPaused(bool paused) {
  m_paused = paused;
  if (!m_paused) {
    processQueue();
  }
}

bool SimController::isPaused() const { return m_paused; }

void SimController::resetState() {
  m_queue.clear();
  m_waitingResponse = false;
}

void SimController::enqueueCommand(const QString &command) {
  m_queue.enqueue(command.trimmed());
  processQueue();
}

void SimController::processQueue() {
  if (m_paused || m_waitingResponse || !m_bot) {
    return;
  }

  while (!m_queue.isEmpty() && !m_waitingResponse) {
    QString command = m_queue.dequeue();
    if (command.isEmpty()) {
      continue;
    }
    QString response;
    bool defer = false;
    bool ok = processCommand(command, &response, &defer);
    if (!ok) {
      handleInvalid(command);
      continue;
    }
    if (defer) {
      m_waitingResponse = true;
      break;
    }
    if (!response.isEmpty()) {
      sendResponse(response);
    }
  }
}

void SimController::sendResponse(const QString &response) {
  if (!m_bot) {
    return;
  }
  m_bot->sendLine(response);
}

void SimController::handleInvalid(const QString &command) {
  emit logMessage(QString("Invalid command: %1").arg(command));
}

void SimController::onMovementFinished(bool crashed) {
  if (!m_waitingResponse) {
    return;
  }
  m_waitingResponse = false;
  sendResponse(crashed ? "crash" : "ack");
  processQueue();
}

bool SimController::processCommand(const QString &command, QString *response,
                                   bool *defer) {
  QStringList tokens = command.split(" ", Qt::SkipEmptyParts);
  if (tokens.isEmpty()) {
    return false;
  }

  QString fn = tokens.at(0);
  if (tokens.size() > 2 && fn != "setText") {
    return false;
  }

  if (fn == "mazeWidth") {
    *response = QString::number(m_sim->maze()->width());
    return true;
  }
  if (fn == "mazeHeight") {
    *response = QString::number(m_sim->maze()->height());
    return true;
  }

  if (fn == "goalCount") {
    *response = QString::number(m_sim->goalCells().size());
    return true;
  }
  if (fn == "goalCell") {
    if (tokens.size() != 2) {
      return false;
    }
    bool okIndex = false;
    int index = tokens.at(1).toInt(&okIndex);
    if (!okIndex) {
      return false;
    }
    QList<QPair<int, int>> goals = m_sim->goalCells().values();
    std::sort(goals.begin(), goals.end(),
              [](const QPair<int, int> &a, const QPair<int, int> &b) {
                if (a.second == b.second) {
                  return a.first < b.first;
                }
                return a.second < b.second;
              });
    if (index < 0 || index >= goals.size()) {
      return false;
    }
    const auto &cell = goals.at(index);
    *response = QString("%1 %2").arg(cell.first).arg(cell.second);
    return true;
  }
  if (fn == "isGoal") {
    QPair<int, int> cell = m_sim->mouse().position().toCell();
    *response = m_sim->goalCells().contains(cell) ? "true" : "false";
    return true;
  }

  auto parseHalfSteps = [&](int defaultValue) -> int {
    if (tokens.size() == 2) {
      return tokens.at(1).toInt();
    }
    return defaultValue;
  };

  if (fn == "wallFront") {
    int halfStepsAway = parseHalfSteps(1);
    int halfStepsAhead = halfStepsAway - 1;
    *response = m_sim->isWallFront(halfStepsAhead) ? "true" : "false";
    return true;
  }
  if (fn == "wallRight") {
    int halfStepsAway = parseHalfSteps(1);
    int halfStepsAhead = halfStepsAway - 1;
    *response = m_sim->isWallRight(halfStepsAhead) ? "true" : "false";
    return true;
  }
  if (fn == "wallLeft") {
    int halfStepsAway = parseHalfSteps(1);
    int halfStepsAhead = halfStepsAway - 1;
    *response = m_sim->isWallLeft(halfStepsAhead) ? "true" : "false";
    return true;
  }
  if (fn == "wallBack") {
    int halfStepsAway = parseHalfSteps(1);
    int halfStepsAhead = halfStepsAway - 1;
    *response = m_sim->isWallBack(halfStepsAhead) ? "true" : "false";
    return true;
  }
  if (fn == "wallFrontRight") {
    int halfStepsAway = parseHalfSteps(1);
    int halfStepsAhead = halfStepsAway - 1;
    *response = m_sim->isWallFrontRight(halfStepsAhead) ? "true" : "false";
    return true;
  }
  if (fn == "wallFrontLeft") {
    int halfStepsAway = parseHalfSteps(1);
    int halfStepsAhead = halfStepsAway - 1;
    *response = m_sim->isWallFrontLeft(halfStepsAhead) ? "true" : "false";
    return true;
  }
  if (fn == "wallBackRight") {
    int halfStepsAway = parseHalfSteps(1);
    int halfStepsAhead = halfStepsAway - 1;
    *response = m_sim->isWallBackRight(halfStepsAhead) ? "true" : "false";
    return true;
  }
  if (fn == "wallBackLeft") {
    int halfStepsAway = parseHalfSteps(1);
    int halfStepsAhead = halfStepsAway - 1;
    *response = m_sim->isWallBackLeft(halfStepsAhead) ? "true" : "false";
    return true;
  }

  if (fn == "moveForward") {
    int distance = parseHalfSteps(1);
    int numHalfSteps = distance * 2;
    bool ok = m_sim->requestMove(numHalfSteps);
    if (!ok) {
      *response = "crash";
      return true;
    }
    *defer = true;
    return true;
  }
  if (fn == "moveForwardHalf") {
    int numHalfSteps = parseHalfSteps(1);
    bool ok = m_sim->requestMove(numHalfSteps);
    if (!ok) {
      *response = "crash";
      return true;
    }
    *defer = true;
    return true;
  }

  if (fn == "turnRight" || fn == "turnRight90") {
    m_sim->requestTurn(Movement::TurnRight90);
    *defer = true;
    return true;
  }
  if (fn == "turnLeft" || fn == "turnLeft90") {
    m_sim->requestTurn(Movement::TurnLeft90);
    *defer = true;
    return true;
  }
  if (fn == "turnRight45") {
    m_sim->requestTurn(Movement::TurnRight45);
    *defer = true;
    return true;
  }
  if (fn == "turnLeft45") {
    m_sim->requestTurn(Movement::TurnLeft45);
    *defer = true;
    return true;
  }

  if (fn == "setWall" || fn == "clearWall") {
    if (tokens.size() != 4) {
      return false;
    }
    bool okX = false;
    bool okY = false;
    int x = tokens.at(1).toInt(&okX);
    int y = tokens.at(2).toInt(&okY);
    if (!okX || !okY || tokens.at(3).size() != 1) {
      return false;
    }
    Direction dir;
    if (!directionFromChar(tokens.at(3).at(0), &dir)) {
      return false;
    }
    m_sim->setKnownWall(x, y, dir,
                        fn == "setWall" ? WallState::Wall : WallState::Open);
    int nx = x;
    int ny = y;
    if (dir == Direction::North) {
      ny += 1;
    } else if (dir == Direction::East) {
      nx += 1;
    } else if (dir == Direction::South) {
      ny -= 1;
    } else if (dir == Direction::West) {
      nx -= 1;
    }
    Direction opposite = rotateLeft(rotateLeft(dir));
    if (m_sim->maze() && m_sim->maze()->inBounds(nx, ny)) {
      m_sim->setKnownWall(nx, ny, opposite,
                          fn == "setWall" ? WallState::Wall : WallState::Open);
    }
    return true;
  }

  if (fn == "setColor") {
    if (tokens.size() != 4 || tokens.at(3).size() != 1) {
      return false;
    }
    bool okX = false;
    bool okY = false;
    int x = tokens.at(1).toInt(&okX);
    int y = tokens.at(2).toInt(&okY);
    if (!okX || !okY) {
      return false;
    }
    m_sim->setCellColor(x, y, tokens.at(3).at(0));
    return true;
  }

  if (fn == "clearColor") {
    if (tokens.size() != 3) {
      return false;
    }
    bool okX = false;
    bool okY = false;
    int x = tokens.at(1).toInt(&okX);
    int y = tokens.at(2).toInt(&okY);
    if (!okX || !okY) {
      return false;
    }
    m_sim->clearCellColor(x, y);
    return true;
  }

  if (fn == "clearAllColor") {
    m_sim->clearAllColors();
    return true;
  }

  if (fn == "setText") {
    int first = command.indexOf(' ');
    int second = command.indexOf(' ', first + 1);
    int third = command.indexOf(' ', second + 1);
    if (first == -1 || second == -1 || third == -1) {
      return false;
    }
    QString xStr = command.mid(first + 1, second - first - 1);
    QString yStr = command.mid(second + 1, third - second - 1);
    bool okX = false;
    bool okY = false;
    int x = xStr.toInt(&okX);
    int y = yStr.toInt(&okY);
    if (!okX || !okY) {
      return false;
    }
    QString text = command.mid(third + 1);
    m_sim->setCellText(x, y, text);
    return true;
  }

  if (fn == "clearText") {
    if (tokens.size() != 3) {
      return false;
    }
    bool okX = false;
    bool okY = false;
    int x = tokens.at(1).toInt(&okX);
    int y = tokens.at(2).toInt(&okY);
    if (!okX || !okY) {
      return false;
    }
    m_sim->clearCellText(x, y);
    return true;
  }

  if (fn == "clearAllText") {
    m_sim->clearAllText();
    return true;
  }

  if (fn == "wasReset") {
    *response = m_sim->wasReset() ? "true" : "false";
    return true;
  }

  if (fn == "ackReset") {
    m_sim->ackReset();
    *response = "ack";
    return true;
  }

  if (fn == "getStat") {
    if (tokens.size() != 2) {
      return false;
    }
    QString statName = tokens.at(1);
    StatId stat;
    if (statName == "total-distance") {
      stat = StatId::TotalDistance;
    } else if (statName == "total-turns") {
      stat = StatId::TotalTurns;
    } else if (statName == "best-run-distance") {
      stat = StatId::BestRunDistance;
    } else if (statName == "best-run-turns") {
      stat = StatId::BestRunTurns;
    } else if (statName == "current-run-distance") {
      stat = StatId::CurrentRunDistance;
    } else if (statName == "current-run-turns") {
      stat = StatId::CurrentRunTurns;
    } else if (statName == "total-effective-distance") {
      stat = StatId::TotalEffectiveDistance;
    } else if (statName == "best-run-effective-distance") {
      stat = StatId::BestRunEffectiveDistance;
    } else if (statName == "current-run-effective-distance") {
      stat = StatId::CurrentRunEffectiveDistance;
    } else if (statName == "score") {
      stat = StatId::Score;
    } else {
      return false;
    }
    QString value = m_sim->stats().statString(stat);
    if (value.isEmpty()) {
      value = "-1";
    }
    *response = value;
    return true;
  }

  return false;
}

}  // namespace hadak
