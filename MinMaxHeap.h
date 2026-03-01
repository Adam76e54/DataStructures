#pragma once

#include <functional>
#include <iostream>
#include <map>
#include <cmath>
#include <optional> 
#include <cstdint>
#include <vector>
#include <bit>

int number = 0;
double other = static_cast<double>(number);

template <typename T>
void printVector(const std::vector<T>& vector ){
  for(auto it : vector){
    std::cout << it << " ";
  }
}


//original way we looked at
size_t level(size_t idx){
  constexpr size_t bits = 8*sizeof(size_t);
  
  //NOTE: some compilers have an __countl_zero but that was before C++20 and isn't cross-compiler so don't use it
  return (bits - std::countl_zero(idx + 1));
}

//simpler way
size_t level_(size_t idx){
  //bit_width ignores leading zeros, just counts the number of siginificant bits (which is equivalent to returnig log2(x))
  return std::bit_width(idx + 1) - 1; //equivalent to floor(log2(idx + 1))
}

template <typename T>
class MinMaxHeap{
public:
  void push(const T& value){
    if(numElements_ == 0){
      //std::cout << "First push done\n";
      array_[0] = value;
      ++numElements_;
      return;
    }

    if(capacity_ > minCapacity_){
      if(bool worked = refit(); !worked){
        throw std::__throw_bad_alloc;
      }
    }
    
    array_[numElements_] = value;

    //std::cout << "about to siftUp():\n";
    siftUp(numElements_);
    //std::cout << "SiftedUp()\n";
    ++numElements_;
  }

  //note to self: need to implement downsizing memory here
  void popMax(){
    //CASE 0: empty
    if(numElements_ == 0){
      return;
    }
    //CASE 1: only one child or empty, don't compare anything, just lower the number of elements
    if(numElements_ < 3){
      --numElements_;
      return;
    }

    if(capacity_ > minCapacity_){
      if(bool worked = refit(); !worked){
        throw std::__throw_bad_alloc;
      }
    }

    //CASE 2: 2 children requires a comparison.
    size_t larger = (array_[1] >= array_[2]) ? 1 : 2;
    std::swap(array_[larger], array_[numElements_ - 1]);
    --numElements_;
    siftDown(larger);
  }

  void popMin(){
    //CASE 0: empty
    if(numElements_ == 0){
      //std::cout << "Empty case reached\n";
      return;
    }
    //CASE 1: no children
    if(numElements_ == 1){
      //std::cout << "no child case reached\n";
      --numElements_;
      return;
    }
    //CASE 2: one valid child
    if(numElements_ == 2){
      std::swap(array_[0], array_[1]);
      --numElements_;
      return;
    }
    //CASE 2: valid children
    //std::cout << "Valid child case reached\n";
    //std::cout << "Swapping root(" << array_[0] << ") with " << array_[numElements_ - 1] << '\n';
    std::swap(array_[0], array_[numElements_ - 1]);
    --numElements_;
    siftDown(0);
  }

  std::optional<T> max(){
    //CASE 0: empty
    if(numElements_ == 0){
      return std::nullopt;
    }
    //CASE 1: no children
    if(numElements_ == 1){
      return array_[0];
    }
    //CASE 2: one child
    if(numElements_ == 2){
      return array_[1];
    }
  //CASE 2: two valid children
    return (array_[1] >= array_[2]) ? array_[1] : array_[2];
  }

  std::optional<T> min(){
    //CASE 0: empty
    if(numElements_ == 0){
      return std::nullopt;
    }
    //CASE 1: at least 1 element
    return array_[0];
  }

  void sequentialPrint(){
    for(size_t i = 0; i < numElements_; ++i){
      std::cout << array_[i] << " ";
    }
    std::cout << std::endl;
  }
  
  bool checkHeapIntegrity(){
    return checkHeapIntegrity(0);
  }
  //CONSTRUCTION, DESTRUCTION, AND ASSIGNMENT
  MinMaxHeap(size_t size = minCapacity_) : array_(new T[size]), numElements_(0), capacity_(size) {

  }

  MinMaxHeap(const MinMaxHeap& other) : array_(new T[other.capacity_]), numElements_(other.numElements_), capacity_(other.capacity_){
    for(size_t i = 0; i < numElements_; ++i){
      array_[i] = other.array_[i];
    }
  }

  MinMaxHeap(MinMaxHeap&& other) : array_(other.array_), numElements_(other.numElements_), capacity_(other.capacity_){
    other.array_ = nullptr;
    other.numElements_ = 0;
    other.capacity_ = minCapacity_;
  }


  MinMaxHeap& operator=(MinMaxHeap other){
    if(this != &other){
      std::swap(*this, other);
    }
    return *this;
  }

  ~MinMaxHeap(){
    delete[] array_;
  }
private:
  T* array_;
  size_t capacity_, numElements_;
  static constexpr std::uint8_t minCapacity_ = std::pow(2, 4);

  void siftUp(size_t currentIndex){
    //std::cout << "Current index = " << currentIndex << '\n';
    //while current is not the root, we sift up. 
    while(currentIndex >= 1){
      //set level, parent, and grandparent of current
      
      size_t level = std::floor(std::log2(currentIndex + 1));//standard way to convert log to discrete range. RE: log(1) = 0.
      //std::cout << "current index of "<< currentIndex << " produced" << " level = " << level << '\n';
      size_t parentIndex = (currentIndex - 1)/2;
      size_t grandparentIndex = (parentIndex - 1)/2; //this will be massive if parent index = 0;
      //std::cout << grandparentIndex << " grandparent index\n";

      //if even(min) layer. Make sure the grandparent is smallest and the parent is larger. 
      if(level % 2 == 0){
        //std::cout << "Entered min level\n";
        //swap with grandparent first. This prevents a grandparent from being switched to a different layer when it was already larger/smaller
        //than others in the layers below, it will break the minmax property becuase everything is essentially shunted down one, reversing the layer order
        //we basically want to avoid two consecutive child-parent swaps. 
        if((array_[currentIndex] < array_[grandparentIndex]) && grandparentIndex < numElements_){
          std::swap(array_[currentIndex], array_[grandparentIndex]);
          currentIndex = grandparentIndex;
          //std::cout << "swapped min grandparent" << "with parent index = " << parentIndex << " and grandparent index = " << grandparentIndex << '\n';
        }
        //If the grandparent doesn't need swapping, check if the parent is larger, swap if not. 
        else if(array_[currentIndex] > array_[parentIndex]){
          std::swap(array_[currentIndex], array_[parentIndex]);
          currentIndex = parentIndex;
          //std::cout << "swapped maxlevel parent\n";
        }
        //return if the node need not be moved
        else{
          //std::cout << "positioned on min level\n";
          return;
        }
      }

      //if odd(max) layer. We make sure the grandparent is bigger and make sure the parent is smaller
      if(level % 2 == 1){
        //std::cout << "Entered max level\n";
        //swap grandparent
        if(array_[currentIndex] > array_[grandparentIndex] && parentIndex > 0){
          std::swap(array_[currentIndex], array_[grandparentIndex]);
          currentIndex = grandparentIndex;
          //std::cout << "swapped max grandparent\n";
        }
        //if not swapped grandparent, swap parent
        else if(array_[currentIndex] < array_[parentIndex]){
          std::swap(array_[currentIndex], array_[parentIndex]);
          currentIndex = parentIndex;
          //std::cout << "swapped minlevel parent\n";
        }
        //if no swaps made, we're in position
        else{
          //std::cout << "Positioned on max level\n";
          return;
        }
      }
      //std::cout << "Looped through siftUp()\n";
    }
  }

  //this is the problematic method. 
  
  void siftDown(size_t currentIndex){

    size_t count = 0;
    //while there's at least one valid descendent
    while((2*currentIndex + 1) < numElements_){

      std::vector<size_t> validDescendentsIndices = returnValidDescendentsIndices(currentIndex);
      //std::cout << "Valid descendents of " << array_[currentIndex] << " = {"; printValidDescendentValues(validDescendentsIndices); std::cout << "}\n";
      //printValidDescendentValues(validDescendentsIndices);

      bool minProcessed = false, maxProcessed = false;
      if(isMinLevel(currentIndex)){
      //std::cout << "Valid descendents = {"; printValidDescendentValues(validDescendentsIndices); std::cout << "}\n";
        minProcessed = processMinLevel(currentIndex);
        //continue;
      }

      if(!isMinLevel(currentIndex)){
      //std::cout << "Valid descendents = {"; printValidDescendentValues(validDescendentsIndices); std::cout << "}\n";
        maxProcessed = processMaxLevel(currentIndex);
      }
      ++count;
      //std::cout << "Iteration " << count << '\n';
      if(!(minProcessed || maxProcessed)){
        break;
      }
    }
  }


  bool isMinLevel(size_t index){
    
    return (static_cast<int>(std::floor(std::log2(index+1))) % 2) == 0;
  }

  bool processMinLevel(size_t& currentIndex){
    std::vector<size_t> validDescendentsIndices = returnValidDescendentsIndices(currentIndex);
    //std::cout << "Valid descendents = {"; printValidDescendentValues(validDescendentsIndices); std::cout << "}\n";
    if(validDescendentsIndices.empty()){
      return false;
    }
    //std::cout << "Entered min level\n";
    size_t smallestDescendentIndex = returnSmallestDescendentIndex(validDescendentsIndices);
    //std::cout << smallestDescendentIndex << '\n';
    //swap with smallest descendent if needed. This ensure the min property that everything in the subtree is larger
    if(array_[currentIndex] > array_[smallestDescendentIndex]){
      //std::cout << "Swapped " << array_[currentIndex] << " with its smaller grandchild " << array_[smallestDescendentIndex] << '\n';
      std::swap(array_[currentIndex], array_[smallestDescendentIndex]);
      currentIndex = smallestDescendentIndex;
      //check the parent (max) is still larger
      if(currentIndex >= 1){
        size_t newParentIndex = (currentIndex - 1)/2;
        if((array_[currentIndex] > array_[newParentIndex]) && !(currentIndex == validDescendentsIndices[0] || currentIndex == validDescendentsIndices[1])){
          //std::cout << "reswapped " << array_[currentIndex] << " with its smaller parent " << array_[newParentIndex] << '\n';
          std::swap(array_[currentIndex], array_[newParentIndex]);
          //NOTE: WE DON'T CHANGE CURRENT INDEX HERE. If the current swaps up to the parent, that validates the new parent but not the new current.
          //So we keep the new current and sift it too.
          //For example: we swap 10 (min) with a grandchild 0, then 10 with it's new parent 8 so we go from {10 -> 8 -> 0 -> x} to {0 -> 10 -> 8 -> x}. 
          //because 8 was already in its correct position, it's guaranteed to be bigger than all of the subtree x. our new version of the tree thus still
          //need to be sifted down through the subtree x still. 10 is in position, we can ignore it now, it's 8 we need to look at.
          //currentIndex = newParentIndex;//this line of code was breaking everything originally because it was wrong
        }
      }

      return true;
    }
    //if nothing is moved then current is in position, return true
    else{
      return false;
    }
  }

  bool processMaxLevel(size_t& currentIndex){
    std::vector<size_t> validDescendentsIndices = returnValidDescendentsIndices(currentIndex);
    //std::cout << "Valid descendents = {"; printValidDescendentValues(validDescendentsIndices); std::cout << "}\n";
    if(validDescendentsIndices.empty()){
      return false;
    }
    //std::cout << "Entered max level\n";
    size_t largestDescendentIndex = returnLargestDescendentIndex(validDescendentsIndices);
    //std::cout << "Largest descendent is " << array_[largestDescendentIndex] << '\n';
    //swap with largest descendent if needed. This ensure the max property that everything in the subtree is smaller
    if(array_[currentIndex] < array_[largestDescendentIndex]){
      //std::cout << "Swapped " << array_[currentIndex] << " with its larger grandchild " << array_[largestDescendentIndex] << '\n';
      std::swap(array_[currentIndex], array_[largestDescendentIndex]);
      currentIndex = largestDescendentIndex;
      //check the parent (min) is still small
      size_t newParentIndex = (currentIndex - 1)/2;
      if(array_[currentIndex] < array_[newParentIndex] && !(currentIndex == validDescendentsIndices[0] || currentIndex == validDescendentsIndices[1])){
        //std::cout << "reswapped " << array_[currentIndex] << " with its larger parent " << array_[newParentIndex] << '\n';
        std::swap(array_[currentIndex], array_[newParentIndex]);
        //NOTE: WE DON'T CHANGE CURRENT INDEX HERE. If the current swaps up to the parent, that validates the new parent but not the new current.
        //So we keep the new current and sift it too.
        //For example: we swap 10 (min) with a grandchild 0, then 10 with it's new parent 8 so we go from {10 -> 8 -> 0 -> x} to {0 -> 10 -> 8 -> x}. 
        //because 8 was already in its correct position, it's guaranteed to be bigger than all of the subtree x. our new version of the tree thus still
        //need to be sifted down through the subtree x still. 10 is in position, we can ignore it now, it's 8 we need to look at.
        //currentIndex = newParentIndex;//this line of code was breaking everything originally because it was wrong
      }
      return true;
    }
    else{
      return false;
    }
  }
  size_t returnSmallestDescendentIndex(const std::vector<size_t>& validDescendentsIndices){
    size_t smallest = validDescendentsIndices[0];//assume this is true, then compare.
    for(auto idx : validDescendentsIndices){
      if(array_[idx] < array_[smallest]){
        smallest = idx;
      }
    }//POST: smallest is the array_ index of the smallest grandchild
    //std::cout << "Smallest = " << array_[smallest] << '\n';
    return smallest;
  }

  size_t returnLargestDescendentIndex(const std::vector<size_t>& validDescendentsIndices){
    size_t largest = validDescendentsIndices[0];
    for(auto idx : validDescendentsIndices){
      if(array_[idx] > array_[largest]){
        largest = idx;
      }
    } //POST: smallest is the array_ index of the smallest 
      //std::cout << "Largest = " << array_[largest] << '\n';
    return largest;
  }

  std::vector<size_t> returnValidDescendentsIndices(size_t currentIndex){
    //container to handle all possible descendents within the next two generations
    size_t descendentIndices[6] = {
      (2*currentIndex) + 1,
      (2*currentIndex) + 2,
      4*currentIndex + 3,
      4*currentIndex + 4,
      4*currentIndex + 5,
      4*currentIndex + 6
    };

    //get the valid descendents
    std::vector<size_t> validDescendentsIndices;
    for(size_t i = 0; i < 6; ++i){
      if(descendentIndices[i] < numElements_){
        validDescendentsIndices.push_back(descendentIndices[i]);
      }
    }
    return validDescendentsIndices;
  }


  void doubleSize(){
    T* newArray = new T[2*capacity_];//local allocation
    //make a deep copy
    for(size_t i = 0; i < numElements_; ++i){
      newArray[i] = array_[i];
    }
    delete[] array_;
    array_ = newArray;//ownership transfer to the outside
    capacity_ *= 2;
  }

  [[nodiscard]] bool refit(){
    if(numElements_ > capacity_){
      return false;
    }

    if(numElements_ == capacity_ || numElements_ < 4*capacity_){
      size_t newCapacity = 2*numElements_;
      T* newArray = new T[newCapacity];//local allocation
      for(size_t i = 0; i < numElements_; ++i){
        newArray[i] = array_[i];
      }
      if(array_) delete[] array_;
      array_ = newArray;//ownership handoff
      capacity_ = newCapacity;
    }

    return true;
  }


  //for debugging
  void printValidDescendentValues(const std::vector<size_t>& descendents){
    for(auto idx : descendents){
      std::cout << array_[idx] << " ";
    }
  }
  bool checkHeapIntegrity(size_t currentIndex){
    size_t leftIndex = 2*currentIndex + 1;
    size_t rightIndex = 2*currentIndex + 2;

    if(!((leftIndex < numElements_))){
      return true;
    }
    
    bool left = false, right = false;
    if(isMinLevel(currentIndex)){
      if(leftIndex < numElements_){
        left = (array_[currentIndex] < array_[leftIndex]) && checkHeapIntegrity(leftIndex);
      }
      else{
        left = true;
      }

      if(rightIndex < numElements_){
        right = (array_[currentIndex] < array_[rightIndex]) && checkHeapIntegrity(rightIndex);
      }
      else{
        right = true;
      }
      if(!left){
        printError(left);
      }
    }

    if(!isMinLevel(currentIndex)){
      if(leftIndex < numElements_){
        left = (array_[currentIndex] > array_[leftIndex]) && checkHeapIntegrity(leftIndex);
      }
      else{
        left = true;
      }

      if(rightIndex < numElements_){
        right = (array_[currentIndex] > array_[rightIndex]) && checkHeapIntegrity(rightIndex);
      }
      else{
        right = true;
      }
      if(!right){
        printError(right);
      }
    }
    
    return left && right;
  }

  void printError(size_t index){
    //std::cout << "Error at index " << index << " = " << array_[index] << '\n';
  }
};

