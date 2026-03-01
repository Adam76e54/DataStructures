#ifndef DYNAMICBITSET_H
#define DYNAMICBITSET_H

#include "BitReference.h"
#include <cstdint>
#include <vector>
#include <memory>
#include <iostream>

class DynamicBitset{
public:
  DynamicBitset() : data_(1, 0){
    //empty default constructor
  } 

  DynamicBitset(size_t size) : data_(size/WORD_SIZE){

  }

  //NOTE: this will resize upwards with no regards to bounds checking. It's a bit sketchy but works for this
  BitReference<std::uint32_t>& operator[](size_t idx){
    if(idx >= (data_.size() * WORD_SIZE)){
      data_.resize(idx/WORD_SIZE + 1);
    }
    BitReference<std::uint32_t> ref(data_[idx/WORD_SIZE], idx % WORD_SIZE);
    return ref;
  }

  inline size_t size(){
    return data_.size() * WORD_SIZE;
  }

  ~DynamicBitset() = default;

  DynamicBitset(const DynamicBitset& other) : data_(other.data_){
    //empty copy constructor
  }

  DynamicBitset(DynamicBitset&& other) : data_(std::move(other.data_)){
    //empty move constructor
  }

  DynamicBitset& operator=(DynamicBitset&& other){
    if(this != &other){
      data_ = std::move(other.data_);
    }

    return *this;
  }

  DynamicBitset& operator=(const DynamicBitset& other){
    if(this != &other){
      data_ = other.data_;
    }

    return *this;
  }

  void print() const{
    for(auto it: data_){
      std::cout << it << " ";
    }
  }
private:
  static constexpr size_t WORD_SIZE =32;
  std::vector<std::uint32_t> data_;//vector allows dynamic resizing, words can be accessed by data[idx/WORD_SIZE]
  //within a word, idx % WORD_SIZE returns the location of the bit 
  //EXAMPLE: data = {0101, 0001}. bit 6 is in word 6/4 = 1 (RE: 0-based counting here) and the specific bit within the word is 6%4 = 2 
  // so bit 6 in data is {0101, 00->0<-1}
};

#endif