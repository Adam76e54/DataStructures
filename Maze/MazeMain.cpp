#include <array>
#include <cstddef>
#include <unordered_set>
#include <iostream>
#include <random>
#include <unordered_map>
#include <queue>
#include <functional>
#include <memory>
#include <bitset>
#include <stack>
#include "Point.h"
#include "Cell.h"
#include "RandomQueue.h"  
#include "Maze.h"

int main() {
  Point point = { 1, 1, 1 };
  Point newPoint = north(point);
  constexpr size_t size = 10;
  Maze<size, size, 3> maze;
  maze.generate();

  std::array<Point, size*size*3> path;
  auto recordParent = [&maze, &path](Point parent, Point current){
    size_t idx = (current.x*maze.y() + current.y)*maze.z() + current.z;
    path[idx] = parent;
  };
  

  maze.print();

  breadthFirstSearch(maze, maze.start(), recordParent);
  auto flatten = [&maze](const Point& p){
    return (p.x*maze.y() + p.y)*maze.z() + p.z;
  };
  bool finished = false;
  bool started = false;
  for(auto it : path){
    //std::cout << it << " ";
    if(it == maze.end()){
      finished = true;
    }
    if(it == maze.start()){
      started = true;
    }
  }
  if(!(finished || started)){
    std::cerr << "Didn't actually find the end of the maze\n";
    return EXIT_FAILURE;
  } else{
    std::cout << "Successfully found a path\n";
  }

  std::cout << "Continue:?";
  std::cin >> finished;
  std::cout << "Path:\n";
  std::cout << "'e'";
  Point current = maze.end();
  while(maze[current].value() != 's'){
    std::cout << " <- "<<path[flatten(current)];
    current = path[flatten(current)];
  }
}

