#pragma once
#include <vector>
#include <random>
#include <optional>
#include <exception>
template<typename T>
class random_queue{
  std::vector<T> data_;
  std::mt19937 seed{std::random_device{}()};
public:
  void push(const T& value){
    data_.push_back(value);
  }

  bool empty(){return data_.empty();}
  size_t size(){return data_.size();}

  //NOTE: this shouldn't generally be marked [[nodiscard]] but for the specific maze problem it's handy. Usually this would be bad design though
  [[nodiscard]] T pop_random(){
    if(data_.empty()){
      throw std::runtime_error("Tried to pop from an empty random_queue");
    }

    std::uniform_int_distribution<> dist(0, data_.size() - 1);
    size_t idx = dist(seed);
    std::swap(data_[idx], data_.back());
    T result = data_.back();
    data_.pop_back();
    return result;
  }

  T operator[](size_t idx){
    return data_.at(idx);
  }
};