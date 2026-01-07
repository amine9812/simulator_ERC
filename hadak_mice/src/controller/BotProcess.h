#pragma once

#include <QObject>
#include <QProcess>
#include <QStringList>

namespace hadak {

class BotProcess : public QObject {
  Q_OBJECT

 public:
  explicit BotProcess(QObject *parent = nullptr);

  bool start(const QString &command, const QString &workingDir);
  void stop();
  bool isRunning() const;

  void sendLine(const QString &line);

 signals:
  void commandReceived(const QString &command);
  void logReceived(const QString &line);
  void finished();

 private:
  QProcess *m_process = nullptr;
  QStringList m_stdoutBuffer;
  QStringList m_stderrBuffer;

  QStringList consumeLines(QString text, QStringList *buffer);
};

}  // namespace hadak
