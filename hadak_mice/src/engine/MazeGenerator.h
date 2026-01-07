#pragma once

#include "engine/Maze.h"

namespace hadak {

class MazeGenerator {
 public:
  static Maze *generate(int width, int height, quint32 seed);
};

}  // namespace hadak
