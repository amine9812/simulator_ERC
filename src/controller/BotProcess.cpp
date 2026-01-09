#include "controller/BotProcess.h"

#include <QProcess>

namespace hadak {

BotProcess::BotProcess(QObject *parent) : QObject(parent) {}

bool BotProcess::start(const QString &command, const QString &workingDir) {
  stop();

  m_process = new QProcess(this);
  m_process->setWorkingDirectory(workingDir);
  QStringList args = QProcess::splitCommand(command);
  if (args.isEmpty()) {
    delete m_process;
    m_process = nullptr;
    return false;
  }
  QString program = args.takeFirst();

  connect(m_process, &QProcess::readyReadStandardOutput, this, [this]() {
    QString output = m_process->readAllStandardOutput();
    QStringList lines = consumeLines(output, &m_stdoutBuffer);
    for (const QString &line : lines) {
      emit commandReceived(line);
    }
  });

  connect(m_process, &QProcess::readyReadStandardError, this, [this]() {
    QString output = m_process->readAllStandardError();
    QStringList lines = consumeLines(output, &m_stderrBuffer);
    for (const QString &line : lines) {
      emit logReceived(line);
    }
  });

  connect(m_process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
          this, [this](int, QProcess::ExitStatus) { emit finished(); });

  m_process->start(program, args);
  return m_process->waitForStarted();
}

void BotProcess::stop() {
  if (!m_process) {
    return;
  }
  m_process->kill();
  m_process->waitForFinished();
  m_process->deleteLater();
  m_process = nullptr;
  m_stdoutBuffer.clear();
  m_stderrBuffer.clear();
}

bool BotProcess::isRunning() const {
  return m_process && m_process->state() != QProcess::NotRunning;
}

void BotProcess::sendLine(const QString &line) {
  if (!m_process) {
    return;
  }
  QString msg = line + "\n";
  m_process->write(msg.toUtf8());
}

QStringList BotProcess::consumeLines(QString text, QStringList *buffer) {
  text.replace("\r", "");
  QStringList parts = text.split("\n");
  QStringList lines;
  if (parts.size() > 1) {
    lines.append(buffer->join("") + parts.at(0));
    buffer->clear();
  }
  for (int i = 1; i < parts.size() - 1; ++i) {
    lines.append(parts.at(i));
  }
  buffer->append(parts.last());
  return lines;
}

}  // namespace hadak
