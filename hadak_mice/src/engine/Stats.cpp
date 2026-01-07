#include "engine/Stats.h"

#include <limits>

namespace hadak {

Stats::Stats() { resetAll(); }

void Stats::resetAll() {
  m_started = false;
  m_solved = false;
  m_penalty = 0.0f;

  m_values.clear();
  setStat(StatId::TotalDistance, 0.0f);
  setStat(StatId::TotalTurns, 0.0f);
  setStat(StatId::CurrentRunDistance, 0.0f);
  setStat(StatId::CurrentRunTurns, 0.0f);
  setStat(StatId::TotalEffectiveDistance, 0.0f);
  setStat(StatId::CurrentRunEffectiveDistance, 0.0f);

  m_values[StatId::BestRunTurns] = std::numeric_limits<float>::max();
  m_values[StatId::BestRunDistance] = 0.0f;
  m_values[StatId::BestRunEffectiveDistance] = 0.0f;
  updateScore();
}

void Stats::addDistance(int distance) {
  float eff = effectiveDistance(distance);
  increment(StatId::TotalDistance, static_cast<float>(distance));
  increment(StatId::TotalEffectiveDistance, eff);
  if (m_started) {
    increment(StatId::CurrentRunDistance, static_cast<float>(distance));
    increment(StatId::CurrentRunEffectiveDistance, eff);
  }
  updateScore();
}

void Stats::addTurn() {
  increment(StatId::TotalTurns, 1.0f);
  if (m_started) {
    increment(StatId::CurrentRunTurns, 1.0f);
  }
  updateScore();
}

void Stats::startRun() {
  setStat(StatId::CurrentRunDistance, 0.0f);
  setStat(StatId::CurrentRunTurns, 0.0f);
  setStat(StatId::CurrentRunEffectiveDistance, 0.0f);
  if (m_penalty > 0.0f) {
    increment(StatId::CurrentRunEffectiveDistance, m_penalty);
    increment(StatId::TotalEffectiveDistance, m_penalty);
    m_penalty = 0.0f;
  }
  m_started = true;
  updateScore();
}

void Stats::finishRun() {
  m_started = false;
  m_solved = true;

  float currentScore = m_values.value(StatId::CurrentRunTurns) +
                       m_values.value(StatId::CurrentRunEffectiveDistance);
  float bestScore = m_values.value(StatId::BestRunTurns) +
                    m_values.value(StatId::BestRunEffectiveDistance);

  if (currentScore < bestScore) {
    setStat(StatId::BestRunTurns, m_values.value(StatId::CurrentRunTurns));
    setStat(StatId::BestRunDistance,
            m_values.value(StatId::CurrentRunDistance));
    setStat(StatId::BestRunEffectiveDistance,
            m_values.value(StatId::CurrentRunEffectiveDistance));
  }
  updateScore();
}

void Stats::endUnfinishedRun() {
  m_started = false;
  updateScore();
}

void Stats::penalizeForReset() { m_penalty = 15.0f; }

float Stats::effectiveDistance(int distance) const {
  if (distance > 2) {
    return distance / 2.0f + 1.0f;
  }
  return static_cast<float>(distance);
}

QString Stats::statString(StatId stat) const {
  if (stat == StatId::BestRunTurns &&
      m_values.value(StatId::BestRunTurns) ==
          std::numeric_limits<float>::max()) {
    return "";
  }
  if (stat == StatId::BestRunDistance &&
      m_values.value(StatId::BestRunTurns) ==
          std::numeric_limits<float>::max()) {
    return "";
  }
  if (stat == StatId::BestRunEffectiveDistance &&
      m_values.value(StatId::BestRunTurns) ==
          std::numeric_limits<float>::max()) {
    return "";
  }

  if (stat == StatId::Score) {
    return QString::number(m_values.value(StatId::Score));
  }

  float value = m_values.value(stat);
  if (isIntegerStat(stat)) {
    return QString::number(static_cast<int>(value));
  }
  return QString::number(value);
}

float Stats::statValue(StatId stat) const { return m_values.value(stat); }

void Stats::setStat(StatId stat, float value) { m_values[stat] = value; }

void Stats::increment(StatId stat, float amount) {
  setStat(stat, m_values.value(stat) + amount);
}

bool Stats::isIntegerStat(StatId stat) const {
  return stat == StatId::TotalDistance || stat == StatId::TotalTurns ||
         stat == StatId::BestRunDistance || stat == StatId::BestRunTurns ||
         stat == StatId::CurrentRunDistance ||
         stat == StatId::CurrentRunTurns;
}

void Stats::updateScore() {
  float score = 2000.0f;
  if (m_solved) {
    score = m_values.value(StatId::BestRunEffectiveDistance) +
            m_values.value(StatId::BestRunTurns) +
            0.1f * (m_values.value(StatId::TotalEffectiveDistance) +
                    m_values.value(StatId::TotalTurns));
  }
  m_values[StatId::Score] = score;
}

}  // namespace hadak
