//LinkedList.tpp
#include "study.h"
#include <iostream>
#include <utility>
#include <optional>


/*DOUBLY LINKED LIST*/
//default constructor
template<typename T>
DoublyLinkedList<T>::DoublyLinkedList()
: head_(nullptr), tail_(nullptr), count_(0) {}


//copy costructor
template<typename T>
DoublyLinkedList<T>::DoublyLinkedList(const DoublyLinkedList<T>& other)
: head_(nullptr), tail_(nullptr), count_(0){


  if (!other.head_) return;//other is empty. Handled in the member initialisation list.


 
  this->head_ = std::make_unique<Node<T>>(other.head_->payload_);//first node of other is deep copied


  Node<T>* current = this->head_.get();//points to head of *this
  Node<T>* otherCurrent = other.head_->next_.get();//points to the second node of other (first not already copied)


  while (otherCurrent){
    current->next_ = std::make_unique<Node<T>>(otherCurrent->payload_);//deep copies next node in other
    current->next_->back_ = current;//the next Node's back_ is just the current Node.


    current = current->next_.get(); //increments current (.get() required since next_ is unique).
    otherCurrent = otherCurrent->next_.get();//increments otherCurrent
  }/**end while
   * @post: otherCurrent = nullptr, current = the last node of the list
   */
  this->tail_ = current;//sets current based on the postcondition for the while loop.
  this->count_ = other.count_;
}


//move constructor
template<typename T>
DoublyLinkedList<T>::DoublyLinkedList(DoublyLinkedList<T>&& other) noexcept
: head_(std::move(other.head_)), tail_(other.tail_), count_(other.count_) {  
    other.tail_ = nullptr;
    other.count_ = 0;
}


template<typename T>
DoublyLinkedList<T>& DoublyLinkedList<T>::operator=(const DoublyLinkedList<T>& other){
  if (this == &other) return *this;//self-assignment avoided
  if (this->head_ != nullptr) this->clear();//clears *this list if already in existence.


  if (other.head_ == nullptr){
    this->head_ = nullptr;
    this->tail_ = nullptr;
    this->count_ = 0;
    return *this;
  }//end other is empty case
 
  this->head_ = std::make_unique<Node<T>>(other.head_->payload_);
  Node<T>* current = this->head_;
  Node<T>* otherCurrent = other.head_->next_.get();
  while(otherCurrent){
    current->next_ = std::make_unique<Node<T>>(otherCurrent->payload_);
    current->next_->back_ = current;


    otherCurrent = otherCurrent->next_.get();
    current = current->next_.get();
  }
  /**
   * @post otherCurrent = nullptr, current points to the tail.  
   */


  this->tail_ = current;
  this->count_ = other.count_;
  return *this;
}


template<typename T>
DoublyLinkedList<T>& DoublyLinkedList<T>::operator=(DoublyLinkedList<T>&& other) noexcept{
  if(this == &other) return *this;


  if(this->head_) this->clear();


  if(!other.head_){
    this->head_ = nullptr;
    this->tail_ = nullptr;
    this->count_ = 0;
    return *this;
  }


  this->head_ = std::move(other.head_);
  this->tail_ = other.tail_;
  this->count_ = other.count_;


  other.tail_ = nullptr;
  other.count_ = 0;


  return *this;
}


template<typename T>
bool DoublyLinkedList<T>::isEmpty() const{
  return (first == nullptr);
}


template<typename T>
void DoublyLinkedList<T>::clear() noexcept{
  head_.reset(); //destroys all the nodes in a chain since head->next gets destroyed, so head->next->next does too.
  tail_ = nullptr;
  count_ = 0;
}


template<typename T>
void DoublyLinkedList<T>::print() const{
  Node<T>* current = head_.get();
  while(current){
    std::cout << current->payload_ << " ";
    current = current->next_.get();
  }
}


template<typename T>
void DoublyLinkedList<T>::reversePrint() const{
  Node<T>* current = tail_;
  while(current){
    std::cout << current->payload_ << " ";
    current = current->back_;
  }
}


template<typename T>
int DoublyLinkedList<T>::length() const noexcept{
  return this->count_;
}


template<typename T>
std::optional<T> DoublyLinkedList<T>::front() const{
  if(head_) return head_->payload_;
  return std::nullopt;
}


template<typename T>
std::optional<T> DoublyLinkedList<T>::back() const{
  if(tail_) return head->payload_;
  return std::nullopt;
}


template<typename T>
std::optional<const Node<T>*> DoublyLinkedList<T>::search(const T& target) const{
  Node<T>* current = head_.get();
  while(current){
    if(current->payload_ == target) return current;
    if(current->payload_ > target) return std::nullopt;
    current = current->next_.get();
  }
  return std::nullopt;
}


template<typename T>
void DoublyLinkedList<T>::insert(const T& payload){
  //NOTE: we have three cases to deal with. Inserting at the beginning, middle, and the end.
  auto newNode = std::make_unique<Node<T>>(payload);
  if(!head_){
    head_ = std::move(newNode);
    tail_ = head_.get();
    count_++;
    return;
  }


  Node<T>* current = head_.get();
  Node<T> previous = nullptr;
  while(current && current->payload_ < payload){
    previous = current;
    current = current->next_.get();
  }//post: current has moved through the list until the payload is greater than all that was before OR the end of the list is reached


  if(!previous){//if the payload is smaller than head_ we insert at the head
    newNode->next_ = std::move(head_);
    if(newNode){
      newNode->next_->back_ = newNode.get();
    }
    head_ = std::move(newNode);
    head_->back_ = nullptr;
    if(!head_->next_) tail_ = head_.get(); //only one node ==> it's the tail_
  }
  else{
    //insert after previous
    newNode->next_ = std::move(previous->next_);//this is the same as current, except it's moveable because next_ is unique
    if(newNode->next_){
      newNode->next_->back_ = newNode.get();
    }
    else{
      //insert at end, update tail_
      tail_ =  newNode.get();
      //newNode->next_ is nullptr in this scenario
    }
    previous->next_ = std::move(newNode);//links the list back together
    newNode->back_ = previous;
  }


  count_++;
}


template<typename T>
void DoublyLinkedList<T>::deleteEntry(const T& target){
  /**
   * FIVE CASES:
   * 1. the list is empty
   * 2. the item is the head_ of the list
   * 3. the item is the tail_ of the list
   * 4. the item is somewhere in the list
   * 5. the item is not in the list
   */


  if(!head_){
    std::cout << "no such item\n";
    return;
  }//end case 1


  if(head_->payload_ == target){
    head_ = std::move(head_->next_);
    if(!head_)
      tail_ = nullptr;//if head_ is null, the list is empty.
    else
      head_->back_ = nullptr; //this was pointing to the old head, it would've been dangling.
    count_--;
    return;
  }//end case 2


  if(tail_->payload_ == target){
    Node<T>* newTail = tail_->back_;
    if(newTail){//check if tail->back_ is nullptr
    newTail->next_.reset();
    tail_ = newTail;
    } else{
      head_.reset();//if tail_ has no back, then it's the same as deleting the head
      tail_ = nullptr;
    }


    count_--;
    return;
  }//end case 3
 
  Node<T>* current = head_.get();
  while (current && current->payload_ != target) {
    current = current->next_.get();
  }
  if (current) {
    if (current->back_) {
      current->back_->next_ = std::move(current->next_);
    }
    if (current->next_) {
      current->next_->back_ = current->back_;
    }
    count_--;
    return;
  }//end case 4


  // Case 5: Item not found
  std::cout << "no such item\n";
 
}
