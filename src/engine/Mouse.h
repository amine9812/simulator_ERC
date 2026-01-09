#pragma once

#include <QPair>

#include "engine/Direction.h"

namespace hadak {

struct SemiPosition {
  int x = 1;
  int y = 1;

  QPair<int, int> toCell() const;
};

class Mouse {
 public:
  Mouse();

  void reset();

  const SemiPosition &position() const;
  SemiDirection heading() const;

  void setPosition(const SemiPosition &pos);
  void setHeading(SemiDirection dir);

 private:
  SemiPosition m_pos;
  SemiDirection m_heading;
};

}  // namespace hadak
