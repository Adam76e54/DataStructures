#include <vector>
#include <unordered_map>
#include <utility>
#include <string>
#include <iostream>
#include <iomanip>
#include <limits>
class AdjacencyMatrix {
  std::unordered_map<std::string, std::size_t> map_;
  std::vector<int> matrix_;
  static constexpr int INFTY = std::numeric_limits<int>::max();
  std::size_t x_, y_;

  std::size_t flatten(std::size_t x, std::size_t y) const{
    return (x*this->y_ + y);    
  }

  std::vector<std::size_t> column(std::size_t x) const{
    std::vector<std::size_t> v;
    for(std::size_t i = 0; i < this->y_; ++i){
      v.push_back(flatten(x, i));
    }
    return v;
  }

  std::vector<std::size_t> row(std::size_t y) const{
    std::vector<std::size_t> v;
    for(std::size_t i = 0; i < this->x_; ++i){
      v.push_back(flatten(i, y));
    }

    return v;
  }

public:
  AdjacencyMatrix() : map_(), matrix_(), x_(0), y_(0)  {

  }

  void node(const std::string& node){
    if(!map_.count(node)){
      std::vector<int> newMatrix;
      std::unordered_map<std::string, std::size_t> newMap;
      size_t newX = x_ + 1, newY = y_ + 1;

      newMap.insert({node, flatten(x_, y_)});
      for(const auto& pair : map_){
        
      }
      newMatrix.assign(newX * newY, INFTY);


    }
  }
  
  void print() const{
    constexpr size_t width = 5;
    for(size_t i = 0; i < this->y_; ++i){
      for(const auto& x : row(i)){
        std::cout << "|" << std::setw(width) << matrix_[flatten(x,i)];
      }
    }
  }

};