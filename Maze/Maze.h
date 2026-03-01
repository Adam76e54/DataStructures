#pragma once

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

template <std::size_t X_SIZE = 64, std::size_t Y_SIZE = 64, std::size_t Z_SIZE = 3>
class Maze {
  using MultiArray = std::array<std::array<std::array<Cell, Z_SIZE>, Y_SIZE>, X_SIZE>;

  MultiArray maze_;
  Point start_, end_;
  unsigned int teleporterWeight_ = 1; //weighted probability for how many cells we want to be teleporters
  unsigned int openingWeight_ = 100; //same but for a regular opening
  std::unordered_multimap<char, Point> teleporterCoordinates_;


  Point randomPoint(std::mt19937& seed){

    //random coordinate within the maze
    std::uniform_int_distribution<> x(0, X_SIZE - 1);//random x position
    std::uniform_int_distribution<> y(0, Y_SIZE - 1);//...
    std::uniform_int_distribution<> z(0, Z_SIZE - 1);//...
    return Point(x(seed), y(seed), z(seed));
  }

  //it's a bit slow to generate these vectors on every call, you could fix this by having an std::unique_ptr to a boolean that checks 
  //if we've already made the containers but that's unnecessary for this 
  char randomValue(std::mt19937& seed){
    //generate possible values
    std::vector<char> values;
    for(const auto it: Cell::teleporters){
      values.push_back(it);
    }
    values.push_back(' ');

    //generate weights 
    std::vector<size_t> weights;
    for(auto it: values){
      weights.push_back(teleporterWeight_);
    }
    weights.back() = openingWeight_;

    std::discrete_distribution<unsigned int> index(weights.begin(), weights.end());

    
    return values[index(seed)];//returns a weighted-probability char (' ' will come out, most likely)
  }
  
  //Lists all of point's unvisited neighbouring cells
  template <size_t N>
  auto listNeighbours(const Point& point, const std::bitset<N>& bits) const {
    auto flatten = [](const Point& p){
      return (p.x*Y_SIZE + p.y)*(Z_SIZE) + p.z;
    };

    std::vector<Point> q;
    const Cell& current = (*this)[point];
    /*
    if(current.value() == '#'){
      std::cerr << "Tried to enqueue the neighbours of a wall\n";
      return q;
    }
      */

    //This lambda is a bit verbose and unreadable but all it does is push all unvisited neighbours onto the vector (with bounds checking)
    auto addNSEW = [&](Point p){
      if(this->within(north(p)) && !bits[flatten(north(p))]) q.push_back(north(p));
      if(this->within(south(p)) && !bits[flatten(south(p))]) q.push_back(south(p));
      if(this->within(east(p)) && !bits[flatten(east(p))]) q.push_back(east(p));
      if(this->within(west(p)) && !bits[flatten(west(p))]) q.push_back(west(p));
    };

    //CASE 1: regular cell
    addNSEW(point);

    //CASE 2: current is a teleporter
    if(Cell::teleporters.count(current.value())){
      //std::cout << "Enter if(teleporter) loop for listNeighbours\n";
      std::queue<Point> aliases;//queue to hold all the cells that the teleporter gives access to
      auto range = teleporterCoordinates_.equal_range(current.value());
      for(auto it = range.first; it != range.second; ++it){
        aliases.push(it->second);
        //std::cout << "Pushed (" << it->second.x << "," << it->second.y << ", " << it->second.z << ")\n";
      }
      
      while(!aliases.empty()){
        addNSEW(aliases.front());
        aliases.pop();
      }
    } 

    return q;
  }

public:
  void generate() {
    /*
    Maze generation is basically a less-constrained Minimal Spanning Tree problem where we get to decide at random which
    path to add to the MST. We'll use a variation of Prim's algorithm. It's fairly intuitive: two data structures 1. queue of neighbours 2. list of visited nodes
    start at a random node, queue its neighbours, add a neighbour to the list of visited nodes and queue its own neighbours to be visited.
    Note that the original algorithm only adds the closest neighbour but our graph (maze) has no edge-weights 
    */

    std::mt19937 seed{std::random_device{}()};
    //set up random start
    start_ = randomPoint(seed);
    (*this)[start_] = 's';
    size_t currentLevel = start_.z;

    std::bitset<X_SIZE*Y_SIZE*Z_SIZE> visited;//bitset to track visited nodes
    //std::cout << "Bitset size: " << bits.size() << '\n';
    //lambda to map a Point to a specific bit in the set. The maze is 3D but in reality gets stored as a linear sequence.
    auto flatten = [](const Point& p){
      return (p.x * Y_SIZE + p.y)*Z_SIZE + p.z;
    };

    visited[flatten(start_)] = 1;//mark the start visited

    random_queue<Point> frontier;//tracks the neighbouring cells to the current sub-maze that have only have 1 open neighbour themself
    //NOTE: we're going to go with the condition that a wall cell is considered visited if it has two opened neighbours. 
    //This is slightly arbitrary because this char-based representation of a maze is a touch weird for a graph.
    //NOTE: random_queue is a custom class I've built, not an STL container

    std::unordered_map<char, bool> teleporterCounter; //checks if a specific teleporter has already been put down.
    for(const auto teleporter : Cell::teleporters){
      teleporterCounter[teleporter] = false;//initialise everything to false
    }
    //In order to guarantee solvability, we're just going to ensure that if a teleporter exists on one level, it exists on every level.
    //There are more intricate ways of doing this using another MST algorithm but there's not need.
    

    //start at the start, push all its neighbours to the stack.
    for(auto it : listNeighbours(start_, visited)){
      frontier.push(it);
    }

    size_t loopCount = 0;//debugging purposes
    size_t levelCount = 0;//counts which level we're currently tunnelling

    //Here's the bulk of the function. It revolves around two structures: the current tree and the frontier
    /*
    1. Start = next = randomNode (established above)
    2. We add the current node to our 'tree'; the graph of open paths. 
    3. We assert that all walls who touch more than 1 opening are considered visited (ensure no 2x2 openings, everything is a corrider junction)
    4. We push all unvisited neighbours of current to our 'frontier'; the random_queue of possible next neighbours 
    REPEAT: resassign next = frontier.pop() and go through the process again until every cell is visited
    5. Our algorithm will also randomly choose an ' ' to be the 'e' (end) after the rest of the maze is generated
    */
      Point next;
      while(!frontier.empty()){
        next = frontier.pop_random();
        //std::cout << "Loop: " << loopCount++ << '\n';
        //pick random neighbour
        //std::cout << "Popped (" << next.x << ","  << next.y << "," << next.z <<") from queue\n";
        if(visited[flatten(next)]){
          //std::cout << "Continued past a visited node\n";
          continue;
        }//if already visited, just ignore 
        //ensure that neighbour is unvisited
        

        Cell& changedCell = (*this)[next];
        char value;
        //only modify if not a special cell
        if(!(Cell::teleporters.count(changedCell.value()) || changedCell.value() == 's' || changedCell.value() == 'e')){
          value = randomValue(seed);
          changedCell.change(value);
        }
        //std::cout << "Modified cell to " << changedCell.value() << '\n';
        //std::cout << "A randomvalue is " << randomValue(seed) << '\n';
        visited[flatten(next)] = 1;//add next to the list of visited cells

        //we ensure each level has one of each teleporter
        if(Cell::teleporters.count(value)){//if we've added a teleporter, check it's not already been added
          //std::cout << "Entered teleporter handling if for " << changedCell.value() << '\n';
          if(teleporterCounter[value]){
            changedCell.change(' ');
            //std::cout << "Assigned an '" << changedCell.value() << "'\n";
          } else{
            teleporterCounter[value] = true;
            for(size_t levelCounter = 1; levelCounter < Z_SIZE; ++levelCounter){//cycle through TOTAL_LEVELS - 1 (we remove the current level from the total levels to visit)
              size_t currentLevel = (levelCounter + next.z)%Z_SIZE;// x%size is a classic way to cycle from 0 up to (size-1), it wraps around.
              Point pointOnNewLevel = randomPoint(seed);
              pointOnNewLevel.z = currentLevel;
              //ensure we don't overwrite a pre-existing teleporters, end, or start
              while (Cell::teleporters.count((*this)[pointOnNewLevel].value()) || (*this)[pointOnNewLevel].value() == 's' || (*this)[pointOnNewLevel].value() == 'e')
              {
                pointOnNewLevel = randomPoint(seed);
                pointOnNewLevel.z = currentLevel;
              }
              (*this)[pointOnNewLevel].change(changedCell.value());
              teleporterCoordinates_.emplace(changedCell.value(), pointOnNewLevel);

              std::cout << "Added '" << changedCell.value() << "' to level " << currentLevel << '\n'; 
            }
          }
        }

//std::cout << "From " << changedCell.value() << '\n'; 
        for(auto it : listNeighbours(next, visited)){
          if(!visited[flatten(it)]){
            frontier.push(it);
           // std::cout << "  pushed (" << it.x << "," << it.y << "," << it.z << ")\n";
          }
        }


        //now we run through the list of unvisited neighbours and ensure they they're truly unvisited
        for(size_t i = 0; i < frontier.size(); ++i){
          Point current = frontier[i];
          unsigned int count = 0;
          auto neighboursOfNeighbour = {north(current), south(current), east(current), west(current)};
          for(auto it: neighboursOfNeighbour){
            if(!within(it)) continue;
            if((*this)[it].value() == ' '){//we're ignoring walls that also touch a teleporter but that's fine
            //won't effect the overall structure of the maze unless there's a lot of teleporters
              ++count;
            }
          }

          if(count > 1){
            visited[flatten(frontier[i])] = 1;//if an unvisited neighbour (which must be a wall) is joined to more than one opening, mark it visited
            //this ensure that we don't de-wall areas that will join nodes that are already on the tree
          }
        }

        //now we add new neighbours to the frontier
        for(const auto it: listNeighbours(next, visited)){
          frontier.push(it);
        }
      }
      std::cout << "Broke out of loop\n";
      //place end cell. This way of doing it is slow but I'm just trying to scrap something that works together
      if(!teleporterCoordinates_.empty()){
        next = randomPoint(seed);
        Cell& cell = (*this)[next];
        while(cell.value() != ' '){
          next = randomPoint(seed);
          cell = (*this)[next];
        }
        cell.change('e');
        end_ = next;
        std::cout << "Just assigned " << cell.value() << " to "<< end_ << '\n';
      } else{
        std::cerr << "Maze generating issue\n";
        
      }
  }

  //returns only walkable neighbours
  std::vector<Point> neighbours(Point current) const{
    std::vector<Point> v;
    
    //NOTE: I saw this trick in a youtube video to simplify generating the cardinal directions
    static const int dx[] = {0,0,-1,1};
    static const int dy[] = {-1,1,0,0};
    //cool use of parallel arrays 

    for(unsigned int i = 0 ; i < 4; ++i){
      Point neighbour{current.x + dx[i], current.y + dy[i], current.z};
      if(within(neighbour) && ((*this)[neighbour].value() == ' ' || Cell::teleporters.count((*this)[neighbour].value()) || (*this)[neighbour].value() == 'e')){
        v.push_back(neighbour);
      }
    }

    char value = (*this)[current].value();
    if(Cell::teleporters.count(value)){
      auto range = teleporterCoordinates_.equal_range(value);
      for(auto pair = range.first; pair != range.second; ++pair){
        v.push_back(pair->second);
      }
    }
    return v;
  }

  Point start() const {
    return start_;
  }

  Point end() const{
    return end_;
  }
  //for debugging
 /* void unlockStart(){
    (*this)[start_].change(' ');//NOTE: maze_[start.x][start.y][start.z] would also be valid but this is shorter to type
  }*/

  Cell& operator[](const Point& point) {
    return maze_.at(point.x).at(point.y).at(point.z);
  }

  const auto& teleporterCoordinates() const{
    return teleporterCoordinates_;
  }

  const Cell& operator[](const Point& point) const {
    return maze_.at(point.x).at(point.y).at(point.z);
  }

  bool within(Point point) const{
    if(point.x >= X_SIZE || point.y >= Y_SIZE || point.z >= Z_SIZE){
      return false;
    }

    return true;
  }

  void print() const{
    size_t count = 0;
    for(size_t z = 0; z < Z_SIZE; ++z){
      std::cout << "Level " << z << ": ----------------\n";
      for(size_t y = 0; y < Y_SIZE; ++y){
        for(size_t x = 0; x < X_SIZE; ++x){
          std::cout << maze_[x][y][z].value() << " ";
        }
        std::cout << '\n';
      }
    }
  }
 
  size_t x() const{
    size_t x  = X_SIZE;
    return x;
  }

  size_t y(){
    return Y_SIZE;
  }

  size_t z(){
    return Z_SIZE;
  }
  template<typename Function>
  friend void bfs(const Maze& maze, Function&& foo);
};


//NOTE: this is going to be our path-finder. 
/*
The big shortest path algorithms are: 
1. Dijkstra's algorithm (for positively weighted graphs)
2. A* (and extension of dijkstra's, often more efficient)
3. Bellman-ford's algorithm (for graphs with negative weights)

All of these are heavily reliant on what's called a breadth-first search (BFS)- you visit the closest neighbours first, then branch out.
Consider dijkstra's algorithm (the most foundational of the big 3), it uses a priority-queue based BFS where you first visit the nearest neighbours
before anything else. But in our maze (an unweighted graph), all the neighbouring cells are the same distance apart. Dijkstra's priority-queue
thus devolves into a regular queue which is literally just a bread-first search.

A key feature of the BFS is that a node through the closest possible path from the start (IF IT'S UNWEIGHTED), any time we visit a node we can thus 
record the path we took and we have the shortest path to it (the least amount of edges to it, disregarding edge weights). 
*/
template<std::size_t X_SIZE, std::size_t Y_SIZE, std::size_t Z_SIZE, typename Function>
void breadthFirstSearch(const Maze<X_SIZE, Y_SIZE, Z_SIZE>& maze, Point start, Function& processor) {
  std::queue<Point> q;//set up the queue
  std::bitset<X_SIZE*Y_SIZE*Z_SIZE> visited;
  auto flatten = [](const Point& p){
    return (p.x*Y_SIZE + p.y)*Z_SIZE + p.z;
  };
  visited[flatten(start)] = 1;
  q.push(start);//push start to the queue

  //loop until the queue is empty (the queue will be empty once every single neighbour is visited)
  //NOTE: if we reach 'e' early then we can return early
  while (!q.empty()) {
    Point current = q.front();

    q.pop();


    for(const Point& neighbour : maze.neighbours(current)){
      if(!visited[flatten(neighbour)]){
        visited[flatten(neighbour)] = 1;
        q.push(neighbour);


        //NOTE: std::forward is a bit unnecessary here but if we want to be able to std::move something into the function list it could be handy
        processor(current, neighbour);
      }
    if(neighbour == maze.end()){
      return;
    }
    }

  }
}
//NOTE ON FUNCTORS
/*
C++ has a 3 main ways of passing functions as arguments:
1. As a raw pointer like (*int)(int a, int b) vibes
2. As an std::functional<> object which is basically a type-safe wrapper for a raw function pointer (we call std::functionals functors because
they're technically a whole object with constructors, members, etc, but they have operator() overloaded so you can call them like a function)
3. Templated function types

The most efficient is generally the 3rd, to my knowledge anyway. 
*/
//NOTE ON VARIADIC TEMPLATES
/*
This is a new one to me, C++11 introduced it. You can basically tell the compiler "this template could take as many arguments as it wants"
and it will let you do it. I'm just playing around with it.
*/

//To track what path has been taken


/**
 * @pre:point should be an open walk cell
 */
template<std::size_t X_SIZE, std::size_t Y_SIZE, std::size_t Z_SIZE>
void enqueueNeighbours(const Point& point, const Maze<X_SIZE, Y_SIZE, Z_SIZE>& maze, std::queue<Point>& q) {
  const Cell& current = maze[point];
  if(current.value() == '#'){
    std::cerr << "Tried to enqueue the neighbours of a wall\n";
    return;
  }

  auto enqueueNSEW = [&q](Point p){
    q.push(north(p));
    q.push(south(p));
    q.push(east(p));
    q.push(west(p));
  };

  //CASE 1: regular cell
  enqueueNSEW(point);

  //CASE 2: current is a teleporter
  if(Cell::teleporters.count(current.value())){
   std::queue<Point> aliases;//queue to push all the points of the teleporter gives access to
    for(const auto it: maze.teleporterCoordinates()){
      if(it.first == current.value()){//if('Teleporter name' == 'name of cell')
        aliases.push(it.second);//enqueue the Point held by the iterator (which is iterating over the map Maze::teleporterCoordinates)
      }
    }

    while(!aliases.empty()){
      enqueueNSEW(aliases.front());
      aliases.pop();
    }
  } 
}





