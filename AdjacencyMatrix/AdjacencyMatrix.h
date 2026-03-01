#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <utility>
#include <iostream>
#include <limits>
#include <variant>
#include <queue>
#include <algorithm>
#include <iomanip>
class AdjacencyMatrix {
  using weight = std::variant<int, char>;//variant is quite an interesting tool, it's like a type-safe union.
  //note that comparison operators will consider (in this case) all chars to be greater than all ints because we've listed <int, char>
  //if we listed <char, int> then chars would be considered smaller.

  std::unordered_map<std::string, std::unordered_map<std::string, weight>> matrix_;
  static constexpr char INFTY = '-';


public:
  void node(const std::string& node) {
    //if already exists, don't add
    if (matrix_.count(node)) {
      return;
    }

    //take a snapshot of the current matrix
    std::vector<std::string> keys;
    keys.reserve(matrix_.size());
    for(const auto& key : matrix_){
      keys.push_back(key.first);//collect the keys currently in the matrix
    }
    //NOTE: before this we tried to modify the matrix and iterate over it at the same time, resulting in subtle bugs (rehashing upon resizing issues I think)
    //it's safer to mutate the matrix by string directly, rather than through an iterator to some specific position in the matrix

    //initialise adjacencies
    for (const auto& oldNode : keys) {
      matrix_[node][oldNode] = INFTY;
      matrix_[oldNode][node] = INFTY;
    }

    matrix_[node][node] = 0;
  }

  void edge(const std::string& from, const std::string& to, int weight) {
    if (matrix_.count(from) && matrix_.count(to)) {
      if (from != to) {
        matrix_[from][to] = weight;
      }
    }
  }

  void removeEdge(const std::string& from, const std::string& to) {
    if (matrix_.count(from) && matrix_.count(to)) {
      if (from != to) {
        matrix_[from][to] = '*';
      }
    }
  }

  void removeNode(const std::string& node){
    for(const auto &key : this->vertices()){
      matrix_[key].erase(node);
    }

    matrix_.erase(node);
  }

  //returns number of vertices
  size_t size() const{
    return matrix_.size();
  }

  //returns sorted vector of nodes
  std::vector<std::string> vertices() const{
    std::vector<std::string> v;
    for(const auto& it : matrix_){
      v.push_back(it.first);
    }

    std::sort(v.begin(), v.end());
    return v;
  }

  void print() const {
    //there's some formatting issues here but it's not the end of the world
    std::cout << " | *";
    for (const auto& node : this->vertices()) {
      std::cout << "|" << std::setw(4) << node;
    }
    std::cout << '\n';

    for (const auto& node : this->vertices()) {
      std::cout << " | " << node;
      for (const auto& neighbour : this->vertices()) {
        try {
          std::visit([](const auto& arg) {
            std::cout << "|" << std::setw(4) << arg;
            }, matrix_.at(node).at(neighbour));
        }
        catch (const std::out_of_range& e) {
          std::cout << "tried to access [" << node << "][" << neighbour << "] " << e.what();
        }
      }
      std::cout << '\n';
    }
  }
};
