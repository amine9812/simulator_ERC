#include "app/AppWindow.h"

#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QShortcut>
#include <QSlider>
#include <QSpinBox>
#include <QSplitter>
#include <QTextStream>
#include <QVBoxLayout>

#include "engine/MazeGenerator.h"

namespace hadak {

AppWindow::AppWindow(QWidget *parent)
    : QMainWindow(parent), m_controller(&m_sim, this) {
  buildUi();
  connectSignals();
  setTimerFromSlider(m_speedSlider->value());
  loadInitialMaze();
  setDefaultBot();
  QTimer::singleShot(0, this, [this]() { maybeAutoStartBot(); });
}

void AppWindow::buildUi() {
  QWidget *central = new QWidget(this);
  setCentralWidget(central);

  QSplitter *mainSplitter = new QSplitter(Qt::Horizontal, central);
  QHBoxLayout *rootLayout = new QHBoxLayout(central);
  rootLayout->addWidget(mainSplitter);

  QWidget *leftPanel = new QWidget();
  QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);

  QGroupBox *controlsBox = new QGroupBox("Simulation Controls");
  QVBoxLayout *controlsLayout = new QVBoxLayout(controlsBox);
  QHBoxLayout *buttonRow = new QHBoxLayout();
  m_playButton = new QPushButton("Play");
  m_pauseButton = new QPushButton("Pause");
  m_stepButton = new QPushButton("Step");
  m_resetButton = new QPushButton("Reset");
  buttonRow->addWidget(m_playButton);
  buttonRow->addWidget(m_pauseButton);
  buttonRow->addWidget(m_stepButton);
  buttonRow->addWidget(m_resetButton);
  controlsLayout->addLayout(buttonRow);

  QLabel *speedLabel = new QLabel("Speed");
  m_speedSlider = new QSlider(Qt::Horizontal);
  m_speedSlider->setRange(1, 100);
  m_speedSlider->setValue(40);
  controlsLayout->addWidget(speedLabel);
  controlsLayout->addWidget(m_speedSlider);

  QGroupBox *mazeBox = new QGroupBox("Maze");
  QVBoxLayout *mazeLayout = new QVBoxLayout(mazeBox);
  QPushButton *loadButton = new QPushButton("Load Maze");
  QPushButton *saveButton = new QPushButton("Save Maze");
  QHBoxLayout *mazeButtonRow = new QHBoxLayout();
  mazeButtonRow->addWidget(loadButton);
  mazeButtonRow->addWidget(saveButton);
  mazeLayout->addLayout(mazeButtonRow);

  QHBoxLayout *sizeRow = new QHBoxLayout();
  m_mazeWidth = new QSpinBox();
  m_mazeWidth->setRange(2, 64);
  m_mazeWidth->setValue(16);
  m_mazeHeight = new QSpinBox();
  m_mazeHeight->setRange(2, 64);
  m_mazeHeight->setValue(16);
  sizeRow->addWidget(new QLabel("W"));
  sizeRow->addWidget(m_mazeWidth);
  sizeRow->addWidget(new QLabel("H"));
  sizeRow->addWidget(m_mazeHeight);
  mazeLayout->addLayout(sizeRow);

  QHBoxLayout *seedRow = new QHBoxLayout();
  m_seedInput = new QLineEdit();
  m_seedInput->setPlaceholderText("Seed (optional)");
  QPushButton *generateButton = new QPushButton("Generate");
  seedRow->addWidget(m_seedInput);
  seedRow->addWidget(generateButton);
  mazeLayout->addLayout(seedRow);

  QGroupBox *overlayBox = new QGroupBox("Overlays & Edit");
  QVBoxLayout *overlayLayout = new QVBoxLayout(overlayBox);
  m_showVisited = new QCheckBox("Visited cells");
  m_showTrueWalls = new QCheckBox("True walls");
  m_showKnownWalls = new QCheckBox("Known walls");
  m_showDistances = new QCheckBox("Goal distances");
  m_showSensors = new QCheckBox("Sensor rays");
  m_showVisited->setChecked(true);
  m_showTrueWalls->setChecked(true);
  m_showKnownWalls->setChecked(true);
  overlayLayout->addWidget(m_showVisited);
  overlayLayout->addWidget(m_showTrueWalls);
  overlayLayout->addWidget(m_showKnownWalls);
  overlayLayout->addWidget(m_showDistances);
  overlayLayout->addWidget(m_showSensors);

  overlayLayout->addWidget(new QLabel("Edit action"));
  m_editAction = new QComboBox();
  m_editAction->addItem("View");
  m_editAction->addItem("Walls");
  m_editAction->addItem("Set Start");
  m_editAction->addItem("Set Goal");
  overlayLayout->addWidget(m_editAction);

  QGroupBox *botBox = new QGroupBox("Bot Runner");
  QVBoxLayout *botLayout = new QVBoxLayout(botBox);
  QHBoxLayout *botSelectRow = new QHBoxLayout();
  m_botSelector = new QComboBox();
  m_refreshBotsButton = new QPushButton("Refresh");
  botSelectRow->addWidget(m_botSelector);
  botSelectRow->addWidget(m_refreshBotsButton);
  botLayout->addLayout(botSelectRow);

  m_botCommand = new QLineEdit();
  m_botCommand->setPlaceholderText("Command (e.g. python3 controller/bots/flood_fill.py)");
  m_botDir = new QLineEdit();
  m_botDir->setPlaceholderText("Working directory");
  m_botStart = new QPushButton("Start Bot");
  m_botStop = new QPushButton("Stop Bot");
  QHBoxLayout *botButtons = new QHBoxLayout();
  botButtons->addWidget(m_botStart);
  botButtons->addWidget(m_botStop);
  botLayout->addWidget(m_botCommand);
  botLayout->addWidget(m_botDir);
  botLayout->addLayout(botButtons);

  leftLayout->addWidget(controlsBox);
  leftLayout->addWidget(mazeBox);
  leftLayout->addWidget(overlayBox);
  leftLayout->addWidget(botBox);
  leftLayout->addStretch();

  QWidget *rightPanel = new QWidget();
  QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
  QGroupBox *debugBox = new QGroupBox("Live State");
  QGridLayout *debugLayout = new QGridLayout(debugBox);
  m_posLabel = new QLabel("(0,0)");
  m_headingLabel = new QLabel("North");
  m_stepsLabel = new QLabel("0");
  m_collisionsLabel = new QLabel("0");
  m_goalLabel = new QLabel("No");
  debugLayout->addWidget(new QLabel("Position"), 0, 0);
  debugLayout->addWidget(m_posLabel, 0, 1);
  debugLayout->addWidget(new QLabel("Heading"), 1, 0);
  debugLayout->addWidget(m_headingLabel, 1, 1);
  debugLayout->addWidget(new QLabel("Steps"), 2, 0);
  debugLayout->addWidget(m_stepsLabel, 2, 1);
  debugLayout->addWidget(new QLabel("Collisions"), 3, 0);
  debugLayout->addWidget(m_collisionsLabel, 3, 1);
  debugLayout->addWidget(new QLabel("Goal"), 4, 0);
  debugLayout->addWidget(m_goalLabel, 4, 1);

  m_logView = new QPlainTextEdit();
  m_logView->setReadOnly(true);
  m_logView->setMinimumHeight(200);

  rightLayout->addWidget(debugBox);
  rightLayout->addWidget(new QLabel("Event Log"));
  rightLayout->addWidget(m_logView);

  m_mazeWidget = new MazeWidget();
  m_mazeWidget->setSimulation(&m_sim);

  mainSplitter->addWidget(leftPanel);
  mainSplitter->addWidget(m_mazeWidget);
  mainSplitter->addWidget(rightPanel);
  mainSplitter->setStretchFactor(1, 1);

  setWindowTitle("Hadak Micromouse Studio");
  resize(1400, 900);

  QString style =
      "QMainWindow { background: #f3efe6; }"
      "QGroupBox { font-weight: 600; border: 1px solid #d6cfc4; border-radius: 6px; margin-top: 8px; }"
      "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 4px; }"
      "QPushButton { background: #e07a5f; color: white; border-radius: 6px; padding: 6px 10px; }"
      "QPushButton:hover { background: #d36a50; }"
      "QPushButton:pressed { background: #c45d46; }"
      "QLineEdit, QSpinBox, QComboBox { background: #fffaf2; border: 1px solid #d6cfc4; border-radius: 4px; padding: 4px; }"
      "QPlainTextEdit { background: #fffaf2; border: 1px solid #d6cfc4; }"
      "QCheckBox { padding: 2px; }";
  setStyleSheet(style);

  connect(loadButton, &QPushButton::clicked, this, &AppWindow::onLoadMaze);
  connect(saveButton, &QPushButton::clicked, this, &AppWindow::onSaveMaze);
  connect(generateButton, &QPushButton::clicked, this,
          &AppWindow::onGenerateMaze);
}

void AppWindow::connectSignals() {
  connect(m_playButton, &QPushButton::clicked, this, &AppWindow::onPlay);
  connect(m_pauseButton, &QPushButton::clicked, this, &AppWindow::onPause);
  connect(m_stepButton, &QPushButton::clicked, this, &AppWindow::onStep);
  connect(m_resetButton, &QPushButton::clicked, this, &AppWindow::onReset);
  connect(m_speedSlider, &QSlider::valueChanged, this,
          &AppWindow::onSpeedChanged);

  connect(m_botStart, &QPushButton::clicked, this, &AppWindow::onStartBot);
  connect(m_botStop, &QPushButton::clicked, this, &AppWindow::onStopBot);
  connect(m_refreshBotsButton, &QPushButton::clicked, this,
          &AppWindow::onRefreshBots);
  connect(m_botSelector, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &AppWindow::onBotSelectionChanged);

  connect(&m_sim, &Simulation::stateChanged, this,
          &AppWindow::onSimulationUpdated);
  connect(&m_sim, &Simulation::eventLogged, this, &AppWindow::onLogMessage);
  connect(&m_controller, &SimController::logMessage, this,
          &AppWindow::onLogMessage);
  connect(&m_bot, &BotProcess::logReceived, this, &AppWindow::onBotLog);
  connect(&m_bot, &BotProcess::commandReceived, &m_controller,
          &SimController::enqueueCommand);

  connect(m_showVisited, &QCheckBox::toggled, m_mazeWidget,
          &MazeWidget::setShowVisited);
  connect(m_showTrueWalls, &QCheckBox::toggled, m_mazeWidget,
          &MazeWidget::setShowTrueWalls);
  connect(m_showKnownWalls, &QCheckBox::toggled, m_mazeWidget,
          &MazeWidget::setShowKnownWalls);
  connect(m_showDistances, &QCheckBox::toggled, m_mazeWidget,
          &MazeWidget::setShowDistances);
  connect(m_showSensors, &QCheckBox::toggled, m_mazeWidget,
          &MazeWidget::setShowSensors);
  connect(m_editAction, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &AppWindow::onEditActionChanged);

  connect(m_mazeWidget, &MazeWidget::logMessage, this,
          &AppWindow::onLogMessage);

  connect(&m_timer, &QTimer::timeout, this, [this]() {
    m_sim.advanceOneTick();
  });

  new QShortcut(QKeySequence(Qt::Key_Space), this, SLOT(onTogglePlayback()));
  new QShortcut(QKeySequence(Qt::Key_S), this, SLOT(onStep()));
  new QShortcut(QKeySequence(Qt::Key_R), this, SLOT(onReset()));
}

QString AppWindow::repoRoot() const {
  auto isRepoRoot = [](const QDir &dir) {
    return dir.exists("controller/bots") && dir.exists("src") &&
           dir.exists("hadak_mice.pro");
  };

  QDir dir(QCoreApplication::applicationDirPath());
  for (int i = 0; i < 6; ++i) {
    if (isRepoRoot(dir)) {
      return dir.absolutePath();
    }
    if (!dir.cdUp()) {
      break;
    }
  }

  QDir cwd(QDir::currentPath());
  for (int i = 0; i < 6; ++i) {
    if (isRepoRoot(cwd)) {
      return cwd.absolutePath();
    }
    if (!cwd.cdUp()) {
      break;
    }
  }

  return QDir::currentPath();
}

void AppWindow::refreshBotList() {
  m_botSelector->clear();
  QDir botsDir(QDir(repoRoot()).filePath("controller/bots"));
  QStringList files = botsDir.entryList(QStringList() << "*.py", QDir::Files,
                                        QDir::Name);
  for (const QString &file : files) {
    QString name = QFileInfo(file).baseName();
    m_botSelector->addItem(name, file);
  }
  if (m_botSelector->count() == 0) {
    m_botSelector->addItem("none", QString());
  }
}

void AppWindow::setDefaultBot() {
  refreshBotList();
  int pledgeIndex = -1;
  int floodIndex = -1;
  for (int i = 0; i < m_botSelector->count(); ++i) {
    QString file = m_botSelector->itemData(i).toString();
    if (file == "pledge.py") {
      pledgeIndex = i;
    }
    if (file == "flood_fill.py") {
      floodIndex = i;
    }
  }
  if (pledgeIndex >= 0) {
    m_botSelector->setCurrentIndex(pledgeIndex);
  } else if (floodIndex >= 0) {
    m_botSelector->setCurrentIndex(floodIndex);
  } else {
    m_botSelector->setCurrentIndex(0);
  }
  onBotSelectionChanged(m_botSelector->currentIndex());
}

bool AppWindow::startBot(bool quiet) {
  if (m_bot.isRunning()) {
    return true;
  }
  m_controller.resetState();
  QString cmd = m_botCommand->text().trimmed();
  QString dir = m_botDir->text().trimmed();
  if (cmd.isEmpty() || dir.isEmpty()) {
    if (!quiet) {
      QMessageBox::warning(this, "Bot", "Command and directory are required");
    }
    return false;
  }
  if (!QDir(dir).exists()) {
    if (!quiet) {
      QMessageBox::warning(this, "Bot", "Working directory not found");
    }
    writeLog(QString("Bot start failed: missing directory %1").arg(dir));
    return false;
  }
  if (!m_bot.start(cmd, dir)) {
    if (!quiet) {
      QMessageBox::warning(this, "Bot", "Failed to start bot process");
    }
    return false;
  }
  m_controller.attachBot(&m_bot);
  writeLog("Bot started");
  return true;
}

void AppWindow::maybeAutoStartBot() {
  if (startBot(true)) {
    writeLog("Auto-started bot");
    onPlay();
  } else {
    writeLog("Auto-start bot failed");
  }
}

void AppWindow::loadInitialMaze() {
  std::unique_ptr<Maze> maze(MazeGenerator::generate(16, 16, 1));
  m_sim.setMaze(std::move(maze));
  onSimulationUpdated();
}

void AppWindow::onPlay() {
  m_controller.setPaused(false);
  m_timer.start();
  writeLog("Simulation running");
}

void AppWindow::onTogglePlayback() {
  if (m_timer.isActive()) {
    onPause();
  } else {
    onPlay();
  }
}

void AppWindow::onPause() {
  m_controller.setPaused(true);
  m_timer.stop();
  writeLog("Simulation paused");
}

void AppWindow::onStep() {
  m_controller.setPaused(false);
  m_sim.advanceOneTick();
  m_controller.setPaused(true);
}

void AppWindow::onReset() {
  bool wasPlaying = m_timer.isActive();
  bool wasBotRunning = m_bot.isRunning();
  m_timer.stop();
  m_controller.setPaused(true);
  m_controller.resetState();
  m_bot.stop();
  m_sim.reset();
  writeLog("Reset simulation");
  if (wasBotRunning) {
    startBot(true);
  }
  if (wasPlaying) {
    onPlay();
  }
}

void AppWindow::onLoadMaze() {
  QString path = QFileDialog::getOpenFileName(
      this, "Open Maze", QString(), "Maze files (*.map *.num)");
  if (path.isEmpty()) {
    return;
  }
  QString error;
  std::unique_ptr<Maze> maze(Maze::fromFile(path, &error));
  if (!maze) {
    QMessageBox::warning(this, "Maze Load Failed", error);
    return;
  }
  bool wasPlaying = m_timer.isActive();
  bool wasBotRunning = m_bot.isRunning();
  m_timer.stop();
  m_controller.setPaused(true);
  m_controller.resetState();
  m_bot.stop();
  m_sim.setMaze(std::move(maze));
  writeLog(QString("Loaded maze: %1").arg(path));
  if (wasBotRunning) {
    startBot(true);
  }
  if (wasPlaying) {
    onPlay();
  }
}

void AppWindow::onSaveMaze() {
  if (!m_sim.maze()) {
    return;
  }
  QString path = QFileDialog::getSaveFileName(
      this, "Save Maze", QString(), "Num maze (*.num)");
  if (path.isEmpty()) {
    return;
  }
  QFile file(path);
  if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
    QMessageBox::warning(this, "Save Failed", "Unable to write file");
    return;
  }
  QTextStream stream(&file);
  for (const QString &line : Maze::toNumLines(*m_sim.maze())) {
    stream << line << "\n";
  }
  writeLog(QString("Saved maze: %1").arg(path));
}

void AppWindow::onGenerateMaze() {
  int width = m_mazeWidth->value();
  int height = m_mazeHeight->value();
  bool ok = false;
  quint32 seed = m_seedInput->text().toUInt(&ok);
  if (!ok) {
    seed = static_cast<quint32>(QDateTime::currentMSecsSinceEpoch() & 0xffffffff);
  }
  std::unique_ptr<Maze> maze(MazeGenerator::generate(width, height, seed));
  if (!maze) {
    QMessageBox::warning(this, "Generate Failed", "Unable to generate maze");
    return;
  }
  bool wasPlaying = m_timer.isActive();
  bool wasBotRunning = m_bot.isRunning();
  m_timer.stop();
  m_controller.setPaused(true);
  m_controller.resetState();
  m_bot.stop();
  m_sim.setMaze(std::move(maze));
  writeLog(QString("Generated maze %1x%2 (seed %3)")
               .arg(width)
               .arg(height)
               .arg(seed));
  if (wasBotRunning) {
    startBot(true);
  }
  if (wasPlaying) {
    onPlay();
  }
}

void AppWindow::onStartBot() { startBot(false); }

void AppWindow::onStopBot() {
  m_bot.stop();
  m_controller.resetState();
  writeLog("Bot stopped");
}

void AppWindow::onSpeedChanged(int value) { setTimerFromSlider(value); }

void AppWindow::setTimerFromSlider(int value) {
  int minInterval = 5;
  int maxInterval = 200;
  int interval = maxInterval - (value - 1) * (maxInterval - minInterval) / 99;
  m_timer.setInterval(interval);
}

void AppWindow::onSimulationUpdated() {
  bool goalReached = m_sim.goalReached();
  if (goalReached && !m_goalReachedLast) {
    QPair<int, int> cell = m_sim.mouse().position().toCell();
    writeLog(QString("Success: goal reached at %1,%2").arg(cell.first).arg(cell.second));
    m_timer.stop();
    m_controller.setPaused(true);
    if (m_bot.isRunning()) {
      m_bot.stop();
      m_controller.resetState();
    }
  }
  m_goalReachedLast = goalReached;
  updateDebugPanel();
}

void AppWindow::updateDebugPanel() {
  SemiPosition pos = m_sim.mouse().position();
  m_posLabel->setText(QString("(%1,%2)").arg(pos.toCell().first).arg(
      pos.toCell().second));
  m_headingLabel->setText(headingToString(m_sim.mouse().heading()));
  m_stepsLabel->setText(QString::number(m_sim.stepCount()));
  m_collisionsLabel->setText(QString::number(m_sim.collisionCount()));
  m_goalLabel->setText(m_sim.goalReached() ? "Yes" : "No");
  m_mazeWidget->update();
}

QString AppWindow::headingToString(SemiDirection dir) const {
  switch (dir) {
    case SemiDirection::North:
      return "North";
    case SemiDirection::South:
      return "South";
    case SemiDirection::East:
      return "East";
    case SemiDirection::West:
      return "West";
    case SemiDirection::NorthEast:
      return "Northeast";
    case SemiDirection::NorthWest:
      return "Northwest";
    case SemiDirection::SouthEast:
      return "Southeast";
    case SemiDirection::SouthWest:
      return "Southwest";
  }
  return "Unknown";
}

void AppWindow::onLogMessage(const QString &message) { writeLog(message); }

void AppWindow::onBotLog(const QString &message) {
  writeLog(QString("[bot] %1").arg(message));
}

void AppWindow::writeLog(const QString &message) {
  m_logView->appendPlainText(message);
}

void AppWindow::onEditActionChanged(int index) {
  EditAction action = EditAction::None;
  if (index == 1) {
    action = EditAction::Walls;
  } else if (index == 2) {
    action = EditAction::Start;
  } else if (index == 3) {
    action = EditAction::Goal;
  }
  m_mazeWidget->setEditAction(action);
}

void AppWindow::onRefreshBots() { refreshBotList(); }

void AppWindow::onBotSelectionChanged(int index) {
  if (index < 0) {
    return;
  }
  QString file = m_botSelector->itemData(index).toString();
  if (file.isEmpty()) {
    return;
  }
  m_botCommand->setText(QString("python3 -u controller/bots/%1").arg(file));
  m_botDir->setText(repoRoot());
}

}  // namespace hadak
