#include "ui/MazeWidget.h"

#include <QMouseEvent>
#include <QPainter>
#include <QtMath>

namespace hadak {

MazeWidget::MazeWidget(QWidget *parent) : QWidget(parent) {
  setMinimumSize(400, 400);
  setMouseTracking(true);
}

void MazeWidget::setSimulation(Simulation *sim) { m_sim = sim; }

void MazeWidget::setShowVisited(bool enabled) {
  m_showVisited = enabled;
  update();
}

void MazeWidget::setShowTrueWalls(bool enabled) {
  m_showTrueWalls = enabled;
  update();
}

void MazeWidget::setShowKnownWalls(bool enabled) {
  m_showKnownWalls = enabled;
  update();
}

void MazeWidget::setShowDistances(bool enabled) {
  m_showDistances = enabled;
  update();
}

void MazeWidget::setShowSensors(bool enabled) {
  m_showSensors = enabled;
  update();
}

void MazeWidget::setEditAction(EditAction action) {
  m_editAction = action;
  update();
}

void MazeWidget::paintEvent(QPaintEvent *event) {
  Q_UNUSED(event);
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.fillRect(rect(), QColor(243, 239, 230));

  if (!m_sim || !m_sim->maze()) {
    return;
  }

  int width = m_sim->maze()->width();
  int height = m_sim->maze()->height();
  if (width <= 0 || height <= 0) {
    return;
  }

  int cellSize = qMin(rect().width() / width, rect().height() / height);
  int mazeWidthPx = cellSize * width;
  int mazeHeightPx = cellSize * height;
  QRectF bounds((rect().width() - mazeWidthPx) / 2.0,
                (rect().height() - mazeHeightPx) / 2.0, mazeWidthPx,
                mazeHeightPx);

  drawOverlays(&painter, bounds);
  drawMaze(&painter, bounds);
  drawMouse(&painter, bounds);
}

void MazeWidget::drawOverlays(QPainter *painter, const QRectF &bounds) {
  if (!m_sim || !m_sim->maze()) {
    return;
  }

  int width = m_sim->maze()->width();
  int height = m_sim->maze()->height();
  qreal cellSize = bounds.width() / width;

  if (m_showVisited) {
    QColor visited(224, 239, 230, 200);
    for (const auto &pos : m_sim->visitedCells()) {
      QRectF cell(bounds.left() + pos.first * cellSize,
                  bounds.bottom() - (pos.second + 1) * cellSize, cellSize,
                  cellSize);
      painter->fillRect(cell, visited);
    }
  }

  QColor goalFill(129, 178, 154, 160);
  QColor startFill(69, 123, 157, 170);
  for (const auto &goal : m_sim->goalCells()) {
    QRectF cell(bounds.left() + goal.first * cellSize,
                bounds.bottom() - (goal.second + 1) * cellSize, cellSize,
                cellSize);
    painter->fillRect(cell.adjusted(3, 3, -3, -3), goalFill);
  }
  QPair<int, int> start = m_sim->startCell();
  QRectF startCell(bounds.left() + start.first * cellSize,
                   bounds.bottom() - (start.second + 1) * cellSize, cellSize,
                   cellSize);
  painter->fillRect(startCell.adjusted(6, 6, -6, -6), startFill);

  for (int x = 0; x < width; ++x) {
    for (int y = 0; y < height; ++y) {
      QRectF cell(bounds.left() + x * cellSize,
                  bounds.bottom() - (y + 1) * cellSize, cellSize, cellSize);
      QChar color = m_sim->cellColor(x, y);
      if (!color.isNull()) {
        QColor fill = cellColorFromChar(color);
        if (fill.isValid()) {
          painter->fillRect(cell.adjusted(2, 2, -2, -2), fill);
        }
      }

      QString text = m_sim->cellText(x, y);
      if (!text.isEmpty()) {
        painter->setPen(QColor(60, 60, 60));
        painter->drawText(cell, Qt::AlignCenter, text);
      }
    }
  }

  if (m_showDistances) {
    QVector<QVector<int>> distances = m_sim->maze()->distancesToCenter();
    painter->setPen(QColor(120, 120, 120));
    QFont font = painter->font();
    font.setPointSize(8);
    painter->setFont(font);
    for (int x = 0; x < width; ++x) {
      for (int y = 0; y < height; ++y) {
        QRectF cell(bounds.left() + x * cellSize,
                    bounds.bottom() - (y + 1) * cellSize, cellSize, cellSize);
        int value = distances[x][y];
        if (value >= 0) {
          painter->drawText(cell.adjusted(2, 2, -2, -2),
                            Qt::AlignTop | Qt::AlignLeft,
                            QString::number(value));
        }
      }
    }
  }
}

void MazeWidget::drawMaze(QPainter *painter, const QRectF &bounds) {
  if (!m_sim || !m_sim->maze()) {
    return;
  }

  int width = m_sim->maze()->width();
  int height = m_sim->maze()->height();
  qreal cellSize = bounds.width() / width;

  painter->setPen(QPen(QColor(210, 200, 190), 1));
  for (int x = 0; x <= width; ++x) {
    qreal px = bounds.left() + x * cellSize;
    painter->drawLine(QPointF(px, bounds.top()), QPointF(px, bounds.bottom()));
  }
  for (int y = 0; y <= height; ++y) {
    qreal py = bounds.top() + y * cellSize;
    painter->drawLine(QPointF(bounds.left(), py), QPointF(bounds.right(), py));
  }

  if (m_showTrueWalls) {
    painter->setPen(QPen(wallColor(), 3));
    for (int x = 0; x < width; ++x) {
      for (int y = 0; y < height; ++y) {
        const Cell &c = m_sim->maze()->cell(x, y);
        qreal left = bounds.left() + x * cellSize;
        qreal right = left + cellSize;
        qreal bottom = bounds.bottom() - y * cellSize;
        qreal top = bottom - cellSize;

        if (c.north) {
          painter->drawLine(QPointF(left, top), QPointF(right, top));
        }
        if (c.east) {
          painter->drawLine(QPointF(right, top), QPointF(right, bottom));
        }
        if (c.south) {
          painter->drawLine(QPointF(left, bottom), QPointF(right, bottom));
        }
        if (c.west) {
          painter->drawLine(QPointF(left, top), QPointF(left, bottom));
        }
      }
    }
  }

  if (m_showKnownWalls) {
    QPen pen;
    pen.setWidth(2);
    pen.setStyle(Qt::DashLine);
    for (int x = 0; x < width; ++x) {
      for (int y = 0; y < height; ++y) {
        qreal left = bounds.left() + x * cellSize;
        qreal right = left + cellSize;
        qreal bottom = bounds.bottom() - y * cellSize;
        qreal top = bottom - cellSize;

        for (int d = 0; d < 4; ++d) {
          WallState state = m_sim->knownWall(x, y, static_cast<Direction>(d));
          if (state == WallState::Unknown) {
            continue;
          }
          pen.setColor(knownWallColor(state));
          painter->setPen(pen);
          if (d == static_cast<int>(Direction::North)) {
            painter->drawLine(QPointF(left + 4, top + 4),
                              QPointF(right - 4, top + 4));
          } else if (d == static_cast<int>(Direction::East)) {
            painter->drawLine(QPointF(right - 4, top + 4),
                              QPointF(right - 4, bottom - 4));
          } else if (d == static_cast<int>(Direction::South)) {
            painter->drawLine(QPointF(left + 4, bottom - 4),
                              QPointF(right - 4, bottom - 4));
          } else if (d == static_cast<int>(Direction::West)) {
            painter->drawLine(QPointF(left + 4, top + 4),
                              QPointF(left + 4, bottom - 4));
          }
        }
      }
    }
  }
}

void MazeWidget::drawMouse(QPainter *painter, const QRectF &bounds) {
  if (!m_sim || !m_sim->maze()) {
    return;
  }
  int width = m_sim->maze()->width();
  qreal cellSize = bounds.width() / width;

  SemiPosition pos = m_sim->mouse().position();
  qreal cx = bounds.left() + (pos.x / 2.0) * cellSize;
  qreal cy = bounds.bottom() - (pos.y / 2.0) * cellSize;

  qreal size = cellSize * 0.35;
  QVector<QPointF> triangle;
  triangle << QPointF(0, -size) << QPointF(size * 0.8, size)
           << QPointF(-size * 0.8, size);

  QTransform transform;
  transform.translate(cx, cy);
  qreal angle = 0.0;
  switch (m_sim->mouse().heading()) {
    case SemiDirection::North:
      angle = 0;
      break;
    case SemiDirection::NorthEast:
      angle = 45;
      break;
    case SemiDirection::East:
      angle = 90;
      break;
    case SemiDirection::SouthEast:
      angle = 135;
      break;
    case SemiDirection::South:
      angle = 180;
      break;
    case SemiDirection::SouthWest:
      angle = 225;
      break;
    case SemiDirection::West:
      angle = 270;
      break;
    case SemiDirection::NorthWest:
      angle = 315;
      break;
  }
  transform.rotate(angle);

  QVector<QPointF> rotated;
  for (const auto &p : triangle) {
    rotated << transform.map(p);
  }

  painter->setPen(Qt::NoPen);
  painter->setBrush(QColor(224, 122, 95));
  painter->drawPolygon(rotated);

  if (m_showSensors) {
    painter->setPen(QPen(QColor(224, 122, 95, 180), 2));
    QPair<int, int> delta = deltaFor(m_sim->mouse().heading());
    QPointF front(cx + delta.first * cellSize, cy - delta.second * cellSize);
    painter->drawLine(QPointF(cx, cy), front);

    QPair<int, int> leftDelta =
        deltaFor(rotateLeft90(m_sim->mouse().heading()));
    QPointF left(cx + leftDelta.first * cellSize,
                 cy - leftDelta.second * cellSize);
    painter->drawLine(QPointF(cx, cy), left);

    QPair<int, int> rightDelta =
        deltaFor(rotateRight90(m_sim->mouse().heading()));
    QPointF right(cx + rightDelta.first * cellSize,
                  cy - rightDelta.second * cellSize);
    painter->drawLine(QPointF(cx, cy), right);
  }
}

QColor MazeWidget::wallColor() const { return QColor(46, 49, 54); }

QColor MazeWidget::knownWallColor(WallState state) const {
  if (state == WallState::Wall) {
    return QColor(244, 162, 97);
  }
  return QColor(42, 157, 143);
}

QColor MazeWidget::cellColorFromChar(QChar color) const {
  switch (color.toLatin1()) {
    case 'k':
      return QColor(0, 0, 0);
    case 'b':
      return QColor(60, 120, 200);
    case 'a':
      return QColor(160, 160, 160);
    case 'c':
      return QColor(80, 190, 200);
    case 'g':
      return QColor(80, 180, 120);
    case 'o':
      return QColor(224, 122, 95);
    case 'r':
      return QColor(220, 80, 80);
    case 'w':
      return QColor(245, 245, 245);
    case 'y':
      return QColor(235, 190, 70);
    case 'B':
      return QColor(30, 60, 120);
    case 'C':
      return QColor(40, 120, 130);
    case 'A':
      return QColor(90, 90, 90);
    case 'G':
      return QColor(40, 120, 80);
    case 'R':
      return QColor(130, 40, 40);
    case 'Y':
      return QColor(160, 120, 40);
    default:
      return QColor();
  }
}

void MazeWidget::mousePressEvent(QMouseEvent *event) {
  if (!m_sim || !m_sim->maze()) {
    return QWidget::mousePressEvent(event);
  }

  if (m_editAction == EditAction::Walls) {
    int cellX = -1;
    int cellY = -1;
    Direction dir;
    if (!pickCellAndWall(event->pos(), &cellX, &cellY, &dir)) {
      return;
    }
    toggleWall(cellX, cellY, dir);
    emit logMessage(QString("Toggled wall at %1,%2 %3")
                        .arg(cellX)
                        .arg(cellY)
                        .arg(directionToChar(dir)));
    update();
    return;
  }

  if (m_editAction == EditAction::Start || m_editAction == EditAction::Goal) {
    int cellX = -1;
    int cellY = -1;
    if (!pickCell(event->pos(), &cellX, &cellY)) {
      return;
    }
    if (m_editAction == EditAction::Start) {
      m_sim->setStartCell(cellX, cellY);
      emit logMessage(QString("Set start at %1,%2").arg(cellX).arg(cellY));
    } else {
      m_sim->setGoalCell(cellX, cellY);
      emit logMessage(QString("Set goal at %1,%2").arg(cellX).arg(cellY));
    }
    update();
    return;
  }

  QWidget::mousePressEvent(event);
}

bool MazeWidget::pickCellAndWall(const QPointF &pos, int *cellX, int *cellY,
                                Direction *dir) const {
  int width = m_sim->maze()->width();
  int height = m_sim->maze()->height();
  int cellSize = qMin(rect().width() / width, rect().height() / height);
  int mazeWidthPx = cellSize * width;
  int mazeHeightPx = cellSize * height;
  QRectF bounds((rect().width() - mazeWidthPx) / 2.0,
                (rect().height() - mazeHeightPx) / 2.0, mazeWidthPx,
                mazeHeightPx);

  if (!bounds.contains(pos)) {
    return false;
  }

  qreal relX = (pos.x() - bounds.left()) / cellSize;
  qreal relY = (bounds.bottom() - pos.y()) / cellSize;

  int x = static_cast<int>(qFloor(relX));
  int y = static_cast<int>(qFloor(relY));
  if (!m_sim->maze()->inBounds(x, y)) {
    return false;
  }

  qreal localX = relX - x;
  qreal localY = relY - y;

  qreal distLeft = localX;
  qreal distRight = 1.0 - localX;
  qreal distBottom = localY;
  qreal distTop = 1.0 - localY;

  qreal minDist = distLeft;
  *dir = Direction::West;
  if (distRight < minDist) {
    minDist = distRight;
    *dir = Direction::East;
  }
  if (distBottom < minDist) {
    minDist = distBottom;
    *dir = Direction::South;
  }
  if (distTop < minDist) {
    minDist = distTop;
    *dir = Direction::North;
  }

  *cellX = x;
  *cellY = y;
  return true;
}

bool MazeWidget::pickCell(const QPointF &pos, int *cellX, int *cellY) const {
  int width = m_sim->maze()->width();
  int height = m_sim->maze()->height();
  int cellSize = qMin(rect().width() / width, rect().height() / height);
  int mazeWidthPx = cellSize * width;
  int mazeHeightPx = cellSize * height;
  QRectF bounds((rect().width() - mazeWidthPx) / 2.0,
                (rect().height() - mazeHeightPx) / 2.0, mazeWidthPx,
                mazeHeightPx);

  if (!bounds.contains(pos)) {
    return false;
  }

  qreal relX = (pos.x() - bounds.left()) / cellSize;
  qreal relY = (bounds.bottom() - pos.y()) / cellSize;

  int x = static_cast<int>(qFloor(relX));
  int y = static_cast<int>(qFloor(relY));
  if (!m_sim->maze()->inBounds(x, y)) {
    return false;
  }

  *cellX = x;
  *cellY = y;
  return true;
}

void MazeWidget::toggleWall(int cellX, int cellY, Direction dir) {
  if (!m_sim || !m_sim->maze()) {
    return;
  }
  Maze *maze = m_sim->maze();
  bool current = maze->isWall(cellX, cellY, dir);
  maze->setWall(cellX, cellY, dir, !current);

  int nx = cellX;
  int ny = cellY;
  Direction opposite = rotateLeft(rotateLeft(dir));
  if (dir == Direction::North) {
    ny += 1;
  } else if (dir == Direction::East) {
    nx += 1;
  } else if (dir == Direction::South) {
    ny -= 1;
  } else if (dir == Direction::West) {
    nx -= 1;
  }
  if (maze->inBounds(nx, ny)) {
    maze->setWall(nx, ny, opposite, !current);
  }
}

}  // namespace hadak
