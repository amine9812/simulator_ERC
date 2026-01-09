#include "engine/Mouse.h"

namespace hadak {

QPair<int, int> SemiPosition::toCell() const { return {x / 2, y / 2}; }

Mouse::Mouse() { reset(); }

void Mouse::reset() {
  m_pos = {1, 1};
  m_heading = SemiDirection::North;
}

const SemiPosition &Mouse::position() const { return m_pos; }

SemiDirection Mouse::heading() const { return m_heading; }

void Mouse::setPosition(const SemiPosition &pos) { m_pos = pos; }

void Mouse::setHeading(SemiDirection dir) { m_heading = dir; }

}  // namespace hadak
