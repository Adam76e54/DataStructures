#pragma once
#include <cstdint>

/**
 * @brief: contains a mutable reference to a bitmask but has operations that only allow it to change a specific bit
 */
template<typename T>
class BitReference{
public:
  BitReference(T& word, size_t bit) : word_(word), bit_(bit){
    //empty constructor
  }

  /**
   * @brief: return true/false based on whether the bit_ in question is 1/0. 
   * @example: word = 1100, bit = 2, bool(bitRef) does (1100 >> 2) = 0011 then (0011) & 0001 = 1.
   */
  operator bool() const{
    return (word_ >> bit_) & 1;
  }

  BitReference& operator=(bool value){
    if(value){
      word_ |= (1 << bit_);// OR masks the bit in question with 1 (always changes the bit to 1)
    }
    else{
      word_ &= ~(1 << bit_);// & masks the bit in quesiton with ~1 = 0 (always changes the bit to 0)
    }

    return *this;
  }

  ~BitReference() = default;

private:
  T word_;
  size_t bit_;
};