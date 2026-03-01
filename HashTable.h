#include <memory>
#include <random>
#include <bit>
#include <bitset>
#include "DynamicBitset.h"
#include <utility>
#include <algorithm>
#include <iostream>
#include <cassert>

bool isPowerOfTwo(size_t a){
  return (a & (a-1) == 0);//a power of two has only one bit flipped (eg. 10, 100, 1000) and -1 from them results in 
}

size_t makePowerOfTwo(size_t n){
  if(n == 0){
    return 1;//1 is a power of 2 since 2^0 = 1
  }
  //we repetitively flip all the bits below the most significant bit to get something like 0001111
  n |= n >> 1;
  n |= n >> 2;
  n |= n >> 4;
  n |= n >> 8;
  n |= n >> 16;
  n |= n >> 32;

  //then add 1 to flip to get a a power of two that's the closest to n (RE: 0001111.... + 1 = 0010000...)
  ++n;

  return n;
}

[[nodiscard]] size_t ensurePowerOfTwo(size_t n){
  if(!isPowerOfTwo(n)){
    return makePowerOfTwo(n);
  }
  return n;
}

template<typename Key, typename Value>
class HashTable{
private:
  std::hash<Key> hashObject_;

  std::vector<Value> values_;
  std::vector<Key> keys_;
  DynamicBitset bitset_;
  size_t numEntries_;
  float loadFactor_;
  static constexpr float LOAD_FACTOR = 0.7;
  static constexpr size_t MIN_SIZE_POWER = 4;

  void updateLoadFactor(){
    if(values_.size() != 0){
      loadFactor_ = numEntries_/values_.size();
    }
    else{
      loadFactor_ = 1;
    }
  }

  //standard double hash probe to resolve collisions
  [[nodiscard]] auto probe(size_t idx, const Key& key, size_t size){
    assert(isPowerOfTwo(size));//shouldn't happen because every call of probe() has a check beforehand but just for debugging
    size_t newIdx = (hashObject_(key) + idx)&(size - 1);//Linear probe (hash + x)&(n-1), works because n will be a power of two.
    //It's a bitwise version of (hash + i)%2

    return newIdx;
  }

  inline bool taken(size_t idx){
    return bitset_[idx];
  }

  template<typename Function>
  [[nodiscard]] size_t hash(const Key& key, size_t size, Function&& foo){
    size = ensurePowerOfTwo(size);

    size_t idx = hashObject_(key) & (size - 1);//extract the k least significant bit if size = 2^k (same thing as n % 2K)
    while(foo(idx) && (this->keys_[idx] != key)){
      idx = probe(idx, key, size);
    }
    return idx;
  }

  
  void rehash(std::vector<Value>& values, std::vector<Key>& keys, DynamicBitset& bits){
    for(size_t i = 0; i < bitset_.size(); ++i){
      if(bitset_[i] == 1){
        //implement rehash
        size_t newSize = values.size() + (values.size()&2);//ensure newSize 
        auto takenInNew = [&](size_t newIdx){
          return bits[newIdx];
        };
        size_t newIdx = hash(this->keys_[i], newSize, takenInNew(newIdx));

        values.at(idx) = this->values_.at(i);
        keys.at(idx) = this->keys_.at(i);
        bits[i] = 1;
      }
    }
  }



  void insert(const Key& key, const Value& value){
    resize();

    size_t idx = hash(key, values_.size(), this->taken);

    values_[idx] = value; 
    keys_[idx] = key;
    bitset_[idx] = 1;

    ++numEntries_;
    updateLoadFactor();
  }

  /**
   * @brief: checks load factor, resizes if LF > 0.7 || LF < 0.01
   */
  void resize(){
    updateLoadFactor();
    if(loadFactor_ > LOAD_FACTOR){
      size_t newSize = 2 * values_.size();
      std::vector<Value> newValues(newSize);
      std::vector<Key> newKeys(newSize);
      DynamicBitset newBits(newSize);
      rehash(newValues, newKeys, newBits);

      this->values_ = std::move(newValues);
      this->keys_ = std::move(newKeys);
      this->bitset_ = std::move(newBits);
    }
    else if((loadFactor_ < 0.01) && numEntries_ > 0){
      size_t newSize = values_.size()/2;
      std::vector<Value> newValues(newSize);
      std::vector<Key> newKeys(newSize);
      DynamicBitset newBits(newSize);
      rehash(newValues, newKeys, newBits);

      this->values_ = std::move(newValues);
      this->keys_ = std::move(newKeys);
      this->bitset_ = std::move(newBits);
    }
  }



public:
  Value& operator[](const Key& key){
    size_t idx = hash(key, values_.size(), taken);

    if(taken(idx)){
      return values_[idx];
    }
    else{
      insert(key, Value{});
    }

    return values_[idx];
  }

  HashTable() : numEntries_(0), loadFactor_(0), values_(1 << MIN_SIZE_POWER), bitset_(1 << MIN_SIZE_POWER), keys_(1 << MIN_SIZE_POWER){

  }

  void print() const{
    for(auto it : values_){
      std::cout << it << ' ';
    }
  }
};