#pragma once

#include <QMainWindow>
#include <QPlainTextEdit>
#include <QTimer>

#include "controller/BotProcess.h"
#include "controller/SimController.h"
#include "engine/Simulation.h"
#include "ui/MazeWidget.h"

class QLabel;
class QLineEdit;
class QPushButton;
class QSlider;
class QSpinBox;
class QCheckBox;

namespace hadak {

class AppWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit AppWindow(QWidget *parent = nullptr);

 private slots:
  void onPlay();
  void onTogglePlayback();
  void onPause();
  void onStep();
  void onReset();
  void onLoadMaze();
  void onSaveMaze();
  void onGenerateMaze();
  void onStartBot();
  void onStopBot();
  void onSpeedChanged(int value);
  void onSimulationUpdated();
  void onLogMessage(const QString &message);
  void onBotLog(const QString &message);

 private:
  Simulation m_sim;
  BotProcess m_bot;
  SimController m_controller;
  QTimer m_timer;

  MazeWidget *m_mazeWidget = nullptr;
  QPlainTextEdit *m_logView = nullptr;

  QPushButton *m_playButton = nullptr;
  QPushButton *m_pauseButton = nullptr;
  QPushButton *m_stepButton = nullptr;
  QPushButton *m_resetButton = nullptr;
  QSlider *m_speedSlider = nullptr;

  QLineEdit *m_botCommand = nullptr;
  QLineEdit *m_botDir = nullptr;
  QPushButton *m_botStart = nullptr;
  QPushButton *m_botStop = nullptr;

  QSpinBox *m_mazeWidth = nullptr;
  QSpinBox *m_mazeHeight = nullptr;
  QLineEdit *m_seedInput = nullptr;

  QCheckBox *m_showVisited = nullptr;
  QCheckBox *m_showTrueWalls = nullptr;
  QCheckBox *m_showKnownWalls = nullptr;
  QCheckBox *m_showDistances = nullptr;
  QCheckBox *m_showSensors = nullptr;
  QCheckBox *m_editMode = nullptr;

  QLabel *m_posLabel = nullptr;
  QLabel *m_headingLabel = nullptr;
  QLabel *m_stepsLabel = nullptr;
  QLabel *m_collisionsLabel = nullptr;
  QLabel *m_goalLabel = nullptr;

  void buildUi();
  void connectSignals();
  void setTimerFromSlider(int value);
  void updateDebugPanel();
  QString headingToString(SemiDirection dir) const;

  void loadInitialMaze();
  void writeLog(const QString &message);
};

}  // namespace hadak
