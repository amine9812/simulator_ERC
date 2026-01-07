#pragma once

#include <QMap>
#include <QString>

namespace hadak {

enum class StatId {
  TotalDistance,
  TotalTurns,
  BestRunDistance,
  BestRunTurns,
  CurrentRunDistance,
  CurrentRunTurns,
  TotalEffectiveDistance,
  BestRunEffectiveDistance,
  CurrentRunEffectiveDistance,
  Score
};

class Stats {
 public:
  Stats();

  void resetAll();
  void addDistance(int distance);
  void addTurn();

  void startRun();
  void finishRun();
  void endUnfinishedRun();
  void penalizeForReset();

  QString statString(StatId stat) const;
  float statValue(StatId stat) const;

 private:
  QMap<StatId, float> m_values;
  bool m_started = false;
  bool m_solved = false;
  float m_penalty = 0.0f;

  void setStat(StatId stat, float value);
  void increment(StatId stat, float amount);
  float effectiveDistance(int distance) const;
  bool isIntegerStat(StatId stat) const;
  void updateScore();
};

}  // namespace hadak
