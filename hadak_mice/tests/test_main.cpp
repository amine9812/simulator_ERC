#include <iostream>

#include "engine/Maze.h"
#include "engine/MazeGenerator.h"
#include "engine/Simulation.h"

using hadak::Direction;
using hadak::Maze;
using hadak::MazeGenerator;
using hadak::Simulation;

static bool testNumParsing() {
  QStringList lines = {
      "0 0 1 1 1 1",
      "0 1 1 1 0 1",
      "1 0 1 1 1 1",
      "1 1 0 1 1 1",
  };
  QString error;
  std::unique_ptr<Maze> maze(Maze::fromNumLines(lines, &error));
  if (!maze) {
    std::cerr << "Num parsing failed: " << error.toStdString() << "\n";
    return false;
  }
  if (!maze->isWall(0, 0, Direction::North)) {
    std::cerr << "Expected wall north at 0,0\n";
    return false;
  }
  return true;
}

static bool testMapParsing() {
  QStringList lines = {
      "+---+---+",
      "|       |",
      "+   +   +",
      "|   |   |",
      "+---+---+",
  };
  QString error;
  std::unique_ptr<Maze> maze(Maze::fromMapLines(lines, &error));
  if (!maze) {
    std::cerr << "Map parsing failed: " << error.toStdString() << "\n";
    return false;
  }
  if (!maze->isWall(0, 0, Direction::West)) {
    std::cerr << "Expected west boundary wall\n";
    return false;
  }
  return true;
}

static bool testMoveCollision() {
  std::unique_ptr<Maze> maze(new Maze(2, 2));
  for (int x = 0; x < maze->width(); ++x) {
    for (int y = 0; y < maze->height(); ++y) {
      maze->setWall(x, y, Direction::North, true);
      maze->setWall(x, y, Direction::East, true);
      maze->setWall(x, y, Direction::South, true);
      maze->setWall(x, y, Direction::West, true);
    }
  }

  Simulation sim;
  sim.setMaze(std::move(maze));
  bool ok = sim.requestMove(2);
  if (ok) {
    std::cerr << "Move should have crashed at wall\n";
    return false;
  }
  return true;
}

static bool testGeneratedMazeValid() {
  std::unique_ptr<Maze> maze(MazeGenerator::generate(10, 10, 123));
  if (!maze) {
    std::cerr << "Generate failed\n";
    return false;
  }
  QString error;
  if (!maze->isValid(&error)) {
    std::cerr << "Generated maze invalid: " << error.toStdString() << "\n";
    return false;
  }
  return true;
}

int main() {
  int failures = 0;
  if (!testNumParsing()) {
    failures++;
  }
  if (!testMapParsing()) {
    failures++;
  }
  if (!testMoveCollision()) {
    failures++;
  }
  if (!testGeneratedMazeValid()) {
    failures++;
  }

  if (failures == 0) {
    std::cout << "All tests passed\n";
    return 0;
  }
  std::cerr << failures << " tests failed\n";
  return 1;
}
