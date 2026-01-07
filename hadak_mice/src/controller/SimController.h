#pragma once

#include <QObject>
#include <QQueue>
#include <QString>

#include "controller/BotProcess.h"
#include "engine/Simulation.h"

namespace hadak {

class SimController : public QObject {
  Q_OBJECT

 public:
  explicit SimController(Simulation *sim, QObject *parent = nullptr);

  void attachBot(BotProcess *bot);
  void setPaused(bool paused);
  bool isPaused() const;

  void enqueueCommand(const QString &command);

 signals:
  void logMessage(const QString &message);

 private slots:
  void onMovementFinished(bool crashed);

 private:
  Simulation *m_sim = nullptr;
  BotProcess *m_bot = nullptr;
  QQueue<QString> m_queue;
  bool m_waitingResponse = false;
  bool m_paused = false;

  void processQueue();
  void sendResponse(const QString &response);
  void handleInvalid(const QString &command);

  bool processCommand(const QString &command, QString *response,
                      bool *defer);
};

}  // namespace hadak
