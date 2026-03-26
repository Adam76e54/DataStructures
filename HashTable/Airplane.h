#ifndef AIRPLANE_H
#define AIRPLANE_H

#include <iostream>
#include <functional>
#include <functional>
#include <vector>
#include <cassert>
#include <cstdint>
#include <limits>

//functions to enfore that the list size is a power of two, there was originally a different reasoning for this (trying to avoid collision resolution cycles)
//but then I switched to a linear probe where cycling wasn't an issue so size = 2^k mainly serves to provide super fast modulo calculations. 
//In hindsight I don't think it's worth the memory overhead.
bool isPowerOfTwo(size_t a);
[[nodiscard]] size_t makePowerOfTwo(size_t n);

//this function is actually useless. 
[[nodiscard]] size_t ensurePowerOfTwo(size_t n);
//Used to serve a purpose but then I changed something but now it's embedded in the code so I don't want to change it

//quicky checks if a ptr is valid, throws if not. 
template<typename T>
inline void assertValid(T* ptr){
  if(!ptr){
    throw std::runtime_error("Nullptr!");
  }
}
//NOTE: I'm aware all these non-member functions could go into a differeng header and .cpp but it seemed unnecessary for this specific assignment

enum SlotState {EMPTY, OCCUPIED, DELETED};

//I decided to make it a little harder and operate on the specified int* as though is were a hashtable
//this should be more efficient if testing for large enough passengerLists (true of a regular commercial flight in Europe)
//Otherwise we'd have to sequential search through the list for nearly every operation, quite slow.
class Airplane {
private:
  static constexpr size_t MIN_SIZE = 16;
  static constexpr float LOAD_FACTOR = 0.65;//open addressing gets very slow above a certain load factor, I've chosen 0.65 somewhat arbitrarily

  int* fuelCapacity;
  int* passengerList;
  std::vector<SlotState> bits;
  int passengerCount;
  size_t listSize;
  //vector to track which indices are in what state. This was originally a bitset but it wasn't working, that's why it's called bits



  //hash for O(1) look-up on passengerList
  std::hash<int> hashObject;
  //linear probe to resolve collisions
  [[nodiscard]] size_t probe(int key, size_t idx, size_t size) const;
  //function to encapsulate the hashobject and the probe together
  [[nodiscard]] size_t hash(int key, size_t size, int* const array,const std::function<SlotState(size_t)>&) const;
  //rehash all keys into a new list and bitset to facilitate resizing
  void rehash(int* const newList, std::vector<SlotState>& newBits, size_t newSize) const;
  //check if an index is taken
  inline SlotState slotState(size_t) const;

  bool refitList(size_t size);
  void checkAndRefit();

  void insert(int id);
  std::int64_t find(int id) const;//this would ideally return std::optional<size_t> but submitty doesn't have C++17
  void remove(int id);
  void sequentialPrint() const;
public:
  Airplane(int fuelCapacity = 0);
  Airplane(const Airplane& other);
  virtual ~Airplane();
  Airplane& operator=(Airplane other);//this will by copy-and-swapped so not using const T& parameter

  Airplane& operator+(int passengerID);
  Airplane& operator++();
  Airplane& operator-(int passengerID);
  Airplane& operator--();
  void SetFuelCapacity(int capacity);
  int GetFuelCapacity() const;
  int GetPassengerCount() const;
  virtual void PrintDetails() const;


};
#endif//AIRPLANE_H