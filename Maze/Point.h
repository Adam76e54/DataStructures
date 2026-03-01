#pragma once
#include<cstddef>
#include <memory>
struct Point {
  size_t x, y, z;
  Point() : x(0), y(0), z(0) {}
  Point(size_t x, size_t y, size_t z) : x(x), y(y), z(z) {}
  Point(const Point& other) : x(other.x), y(other.y), z(other.z) {}
  Point& operator=(Point other){
    //NOTE: swapping with the self is safe, no need to check for it
    //Self-assignment is rare in well-made code so checking for every case of self-assignment is 
    //actually less efficient (on average) than just letting the unnecessary swap happen. 
    std::swap(x, other.x);
    std::swap(y, other.y);
    std::swap(z, other.z);
    return  *this;
  }

  bool operator==(const Point& other) const{
    if(x == other.x && y == other.y && z == other.z){
      return true;
    }
    return false;
  }
};


std::ostream& operator<<(std::ostream& os, const Point& point){
  os<<  "(" << point.x << "," << point.y << "," << point.z << ")";
  return os;
}

Point west(Point point) {
  point.x -= 1;
  return point;
}

Point east(Point point) {
  point.x += 1;
  return point;
}

Point north(Point point) {
  point.y += 1;
  return point;
}

Point south(Point point) {
  point.y -= 1;
  return point;
}
