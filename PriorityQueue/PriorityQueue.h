#pragma once

#include <cmath>
#include <iostream>
#include <utility>
#include <memory>
#include <functional>
#include <optional>
template<typename T, typename Compare = std::less<T>>
class PriorityQueue{
public:
  std::optional<T> top(){
    if(numElements_ != 0){
      return array_[0];
    }

    return std::nullopt;
  }

  void pop(){
    if(numElements_ == 0){
      return;
    }

    array_[0] = array_[numElements_ - 1];
    --numElements_;
    siftDown(0);

    if(numElements_ < capacity_/8){//if there's more than three layers-worth of space
      halfCapacity();
    }
  }

  void push(const T& value){
    if(numElements_ == capacity_){
      doubleCapacity();
    }
    array_[numElements_] = value;
    siftUp(numElements_);
    ++numElements_; 
  }

  void remove(size_t removand){
    std::swap(array_[removand], array_[numElements_ - 1]);
    siftDown(removand);
  }

  PriorityQueue& operator=(const PriorityQueue& other){
    if(this != &other){
      delete[] this->array_;
      this->capacity_ = other.capacity_;
      this->numElements_ = other.numElements_;

      this->array_ = new T[capacity_];
      for(size_t i = 0; i < numElements_; ++i){
        array_[i] = other.array_[i];
      }
    }

    return *this;
  }

  PriorityQueue& operator=(PriorityQueue&& other){
    if(this != &other){
      delete[] this->array_;

      this->array_ = other.array_;
      this->capacity_ = other.capacity_;
      this->numElements_ = other.numElements_;

      other.array_ = nullptr;
      other.capacity_ = 0;
      other.numElements_ = 0;
    }

    return *this;
  }

  explicit PriorityQueue(size_t size = 1) : array_(new T[size]), numElements_(0), capacity_(size * 2){
    //empty constructor
  }

  PriorityQueue(const PriorityQueue& other) : array_(new T[other.capacity_]), numElements_(other.numElements_), capacity_(other.capacity_) {
    for(size_t i = 0; i < numElements_; ++i){
      array_[i] = other.array_[i];
    }
  }

  PriorityQueue(PriorityQueue&& other) : array_(other.array_), numElements_(other.numElements_), capacity_(other.capacity_){
    other.array_ = nullptr;
    other.numElements_ = 0;
    other.capacity_ = 0;
  }
  
  ~PriorityQueue(){
    delete[] array_;
  }

private:
  T* array_;
  size_t numElements_, capacity_;
  Compare compare;

  void siftUp(size_t currentIndex){
    if(currentIndex >= capacity_){
      throw std::invalid_argument("Index out of bounds");
    }

    if(currentIndex == 0){
      return;
    }
    do{
      size_t parentIndex = (currentIndex - 1)/2;
      if(compare(array_[currentIndex], array_[parentIndex])){
        std::swap(array_[currentIndex], array_[parentIndex]);
        currentIndex = parentIndex;
      } else{
        return;
      }
    }while(currentIndex != 0);
  }

  void siftDown(size_t currentIndex){
    while(2*currentIndex + 1 < numElements_){
      size_t leftChildIndex = 2*currentIndex + 1;
      size_t rightChildIndex = leftChildIndex + 1;

      size_t extremeChildIndex = leftChildIndex;
      if(rightChildIndex < numElements_ && compare(array_[rightChildIndex], array_[leftChildIndex])){
        extremeChildIndex = rightChildIndex;
      }
      if(!compare(array_[currentIndex], array_[extremeChildIndex])){
        std::swap(array_[currentIndex], array_[extremeChildIndex]);
        currentIndex = extremeChildIndex;
      } else{
        return;
      }
    }
  }

  void doubleCapacity(){
    T* newArray_ = new T[2*capacity_];//local allocation 1
    for(size_t i = 0; i < numElements_; ++i){
      newArray_[i] = array_[i];
    }
    if(capacity_ != 0){
      capacity_ *= 2;
    } else{
      capacity_ = 2;
    }

    delete[] array_;
    array_ = newArray_;//ownership transfer
  }

  void halfCapacity(){
    T* newArray_ = new T[capacity_/2];//local allocation 1;
    for(size_t i = 0; i < numElements_; ++i){
      newArray_[i] = array_[i];
    }

    if(capacity_ > 1){
      capacity_ /= 2;
    } else{
      capacity_ = 1;
    }

    delete[] array_;
    array_ = newArray_;//ownership transfer
  }
};