#pragma once

#include <QWidget>

#include "engine/Simulation.h"

namespace hadak {

enum class EditAction { None, Walls, Start, Goal };

class MazeWidget : public QWidget {
  Q_OBJECT

 public:
  explicit MazeWidget(QWidget *parent = nullptr);

  void setSimulation(Simulation *sim);
  void setShowVisited(bool enabled);
  void setShowTrueWalls(bool enabled);
  void setShowKnownWalls(bool enabled);
  void setShowDistances(bool enabled);
  void setShowSensors(bool enabled);
  void setEditAction(EditAction action);

 signals:
  void logMessage(const QString &message);

 protected:
  void paintEvent(QPaintEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;

 private:
  Simulation *m_sim = nullptr;
  bool m_showVisited = true;
  bool m_showTrueWalls = true;
  bool m_showKnownWalls = true;
  bool m_showDistances = false;
  bool m_showSensors = false;
  EditAction m_editAction = EditAction::None;

  QColor wallColor() const;
  QColor knownWallColor(WallState state) const;
  QColor cellColorFromChar(QChar color) const;

  void drawMaze(QPainter *painter, const QRectF &bounds);
  void drawMouse(QPainter *painter, const QRectF &bounds);
  void drawOverlays(QPainter *painter, const QRectF &bounds);

  bool pickCellAndWall(const QPointF &pos, int *cellX, int *cellY,
                       Direction *dir) const;
  bool pickCell(const QPointF &pos, int *cellX, int *cellY) const;
  void toggleWall(int cellX, int cellY, Direction dir);
};

}  // namespace hadak
