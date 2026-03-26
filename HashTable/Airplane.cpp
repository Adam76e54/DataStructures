#include "Airplane.h" 

Airplane::Airplane(int fuelCapacity) : fuelCapacity(new int(fuelCapacity)), 
  passengerList(new int[MIN_SIZE]),
  bits(MIN_SIZE, EMPTY), 
  passengerCount(0),
  listSize(MIN_SIZE) {

  for(size_t i = 0; i < MIN_SIZE; ++i){
    passengerList[i] = -1;
    /**
     * assuming negatives are invalid passengerIDs although the sheet specifies any integer would technically be valid
     * if all integers were valid any search() method would have a small chance of containing a passengerID that it wasn't intended to 
     * so I'm forced to make this assumption for safety (I think?) 
     */
  }
}

Airplane::Airplane(const Airplane& other) : fuelCapacity(new int(*(other.fuelCapacity))), 
  passengerList(new int[other.listSize]),
  bits(other.bits), 
  passengerCount(other.passengerCount),
  listSize(other.listSize) {

  assertValid(passengerList);
  for(size_t i = 0; i < listSize; ++i){
    this->passengerList[i] = other.passengerList[i];
  }
}



Airplane::~Airplane(){
  delete[] passengerList;
  delete fuelCapacity;
}

Airplane& Airplane::operator=(Airplane other){
  std::swap(bits, other.bits);
  std::swap(fuelCapacity, other.fuelCapacity);
  std::swap(listSize, other.listSize);
  std::swap(passengerCount, other.passengerCount);
  std::swap(passengerList, other.passengerList);

  return *this;
}


[[nodiscard]] size_t Airplane::probe(int key, size_t probeCount, size_t size) const{
  size = ensurePowerOfTwo(size);
  size_t newIdx = (hashObject(key) + probeCount)&(size - 1);//because size is a power of two, (hash + i)&(2^k - 1) is the same thing as (hash + i)%size
  //it basically works by ANDing (10000 - 1 = 01111) with the new index. 
  //This preserves the bits in the index that are lower than the kth bit, same thing as n % 2^k
  return newIdx;
}

[[nodiscard]] size_t Airplane::hash(int key, size_t size, int* const array ,const std::function<SlotState(size_t)>& slot) const{
  size = ensurePowerOfTwo(size);
  size_t idx = hashObject(key)&(size - 1);
  assertValid(array);
  size_t probeCount = 0;
  while(slot(idx) == OCCUPIED && array[idx] != key){
    idx = probe(key, probeCount, size);

    if(++probeCount >= size){
      throw std::runtime_error("Collision resolution issue in  Airplane::hash()\n");
    }
  }

  return idx;
}

/**
 * @pre: the original state of the airplane should be in place before this is called since it uses old size data
 * @pre: newList and newBits must be empty and of size newSize
 */
void Airplane::rehash(int* const newList, std::vector<SlotState>& newBits, size_t newSize) const{
  assertValid(passengerList);
  for(size_t i = 0; i < this->bits.size(); ++i){
    if(this->bits[i] == OCCUPIED){
      size_t idx = hash(passengerList[i], newSize, newList,[&newBits](size_t idx){
        return newBits[idx];
      });

      newList[idx] = passengerList[i];
      newBits[idx] = OCCUPIED;
    }
  }
}

inline SlotState Airplane::slotState(size_t idx) const{
  return bits[idx];
}

bool Airplane::refitList(size_t size){
  size = ensurePowerOfTwo(size);

  int* newList = new int[size];//local allocation
  for(size_t i = 0; i < size; ++i){
    newList[i] = -1;
  }
  std::vector<SlotState> newBits(size, EMPTY);
  try{
    rehash(newList, newBits, size);
  }catch(std::runtime_error){
    std::cerr << "refitList failed due to passengerList == nullptr";
    delete[] newList;
    return false;
  }

  delete[] passengerList;//clear for new data
  this->passengerList = newList;//pass local allocation to the outside
  this->bits = std::move(newBits);
  listSize = size;
  return true;
}

/**
 * @brief: enforces the load factor and resizes if necessary. Will downsize if the load factor is <0.01 but no below MIN_SIZE
 */
void Airplane::checkAndRefit(){
  //CASE 1: load factor too high
  if((static_cast<double>(passengerCount)/listSize) > LOAD_FACTOR){
    refitList(listSize * 2);
    return;
  }
  //CASE 2: load factor is very small
  if(static_cast<double>(passengerCount)/listSize < 0.01 && listSize > (MIN_SIZE * 2)){//downsize if very small
    refitList(listSize >> 1);
    return;
  }
}

//returns -1 as a sentinel if id not found
//The idea is that this should have an amortized O(1) time complexity which is better than sequential search, especially for large lists
std::int64_t Airplane::find(int id) const{
  size_t idx = hashObject(id)&(listSize - 1);
  assertValid(passengerList);

  for(size_t probes = 0; probes < listSize; ++probes){
    if(bits[idx] == EMPTY){
      return -1;//if the slot is empty the id doesn't exist yet
    }
    if(bits[idx] == OCCUPIED && passengerList[idx] == id){
      return idx;
    }
    idx = probe(id, probes, listSize);
  }

  return -1;
}

void Airplane::insert(int id){
  if(find(id) == -1){
    size_t idx = hash(id, listSize, passengerList,[this](size_t i){ return this->slotState(i);});//you have to use a lambda so slotState() is bound to an instantiation
    passengerList[idx] = id;
    bits[idx] = OCCUPIED;
    ++passengerCount;
    checkAndRefit();
  }
}



void Airplane::remove(int id){
  std::int64_t idx = find(id);
  if(idx >= 0){
    //CASE 1: exists
    assertValid(passengerList);
    passengerList[idx] = -1;
    bits[idx] = DELETED;
    --passengerCount;
    checkAndRefit();
  }
  //CASE 2: Doesn't exist, do nothing
}

Airplane& Airplane::operator++(){
  bool overflow = *fuelCapacity + 1000 > std::numeric_limits<int>::max();
  if(!overflow){
    *fuelCapacity += 1000;
  }else{
    *fuelCapacity = std::numeric_limits<int>::max();
  }

  return *this;
}

Airplane& Airplane::operator--(){
  *fuelCapacity -= 1000;
  if(*fuelCapacity < 0){
    *fuelCapacity = 0;
  }

  return *this;
}

void Airplane::SetFuelCapacity(int fuelCapacity){
  if(fuelCapacity > 0){
    *(this->fuelCapacity) = fuelCapacity;
  }else{
    *(this->fuelCapacity) = 0;
  }
}

int Airplane::GetFuelCapacity() const{
  return *fuelCapacity;
}

int Airplane::GetPassengerCount() const{
  return passengerCount;
}

//This method might actually be problematic since the hashtable won't track what IDs are placed in first, ordering might be different.
void Airplane::PrintDetails() const{
  std::cout << "The airplane's fuel capacity is: " << GetFuelCapacity() <<
  " litres and the passenger list includes the following list of IDs:" <<'\n';
  sequentialPrint();
}

void Airplane::sequentialPrint() const{
  size_t count = 0;
  for(size_t i = 0; i < listSize; ++i){
    if(slotState(i) == OCCUPIED){
      ++count;
      std::cout << count << ". " << passengerList[i] << '\n';
    }
  }

}

//***********NON-CLASS FUNCTIONS */
bool isPowerOfTwo(size_t a){
  return ((a & (a-1)) == 0) && a > 0;//a power of two has only one bit flipped (eg. 10, 100, 1000) and -1 from them results in 
}

[[nodiscard]] size_t makePowerOfTwo(size_t n){
  if(n == 0){
    return 1;//1 is a power of 2 since 2^0 = 1
  }
  --n;//ensures if n = 2^k that the result will be n = 2^k, code was broken without it.

  //we repetitively flip all the bits below the most significant bit to get something like 0001111
  n |= n >> 1;
  n |= n >> 2;
  n |= n >> 4;
  n |= n >> 8;
  n |= n >> 16;
  //using a preprocessing directive so it will only get checked once when compiled on whatever system it is
  #if SIZE_MAX > 0xFFFFFFFF//8 hex digits, 4 bits per per hex, 32 bits
    n |= n >> 32;//only defined for >32-bit integers
  #endif

  //then add 1 to flip to get a a power of two that's the closest to n (RE: 0001111.... + 1 = 0010000...)
  ++n;

  return n;
}

[[nodiscard]] size_t ensurePowerOfTwo(size_t n){
    return makePowerOfTwo(n);
}

Airplane& Airplane::operator+(int passengerID){
  insert(passengerID);
  return *this;
}

Airplane& Airplane::operator-(int passengerID){
  remove(passengerID);
  return *this;
}

