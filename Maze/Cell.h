#pragma once

#include <unordered_set>
#include <iostream>
class Cell {
private:
  char value_;

public:
//NOTE: in hindsight we shouldn't have grouped regulars = {o, w, s, e}, it made the code clunky. Should be seperate pairs {o,w} and {s, e} or something like that
  static const std::unordered_set<char> teleporters;
  static const std::unordered_set<char> regulars;
  void change(char value) {
    if (!teleporters.count(value) && !regulars.count(value)) {
      std::cerr << "Improper argument given to a Cell. No change made" << '\n';
      return;
    }
    this->value_ = value;
  }
  const char value() const {
    return value_;
  }

  char value() {
    return value_;
  }
  Cell() : value_('#') {}
  Cell(char value) : value_(value) {
    if (!(teleporters.count(value) || regulars.count(value))) {
      this->value_ = '#';
    }
  }
};

const std::unordered_set<char> Cell::teleporters = { 'a', 'b', 'c', 'd' };
const std::unordered_set<char> Cell::regulars = { ' ', '#', 's', 'e'};