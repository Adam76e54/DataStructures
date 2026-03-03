#pragma once

#include <memory>
#include <optional>


template<typename T>
struct Node{
  T payload_;
  std::unique_ptr<T> next_;
  Node<T>* back_ = nullptr; //non-owning since unique pointers can't be shared.


  Node(const T& data) : payload_(data), next_(nullptr), back_(nullptr){}
};


template<typename T>//this T is the same as the one above.
class DoublyLinkedList{
  public:
    //copy assignment
    DoublyLinkedList<T>& operator=(const DoublyLinkedList<T>&);


    //move assignment
    DoublyLinkedList<T>& operator=(DoublyLinkedList<T>&&) noexcept;


      //postcondition: returns true iff the list has no Nodes, false otherwise
    bool isEmpty() const;


    /**
     * @brief: leaves the list empty
     * @post: first_ = nullptr, last_ = nullptr, count_ = 0. All nodes deleted.
     */
    void clear() noexcept;


    /**
     * @brief prints the payload of each node from front to back
     */
    void print() const;


    /**
     * @brief prints the payload of each node from back to front
     */
    void reversePrint() const;


    /**
     * @brief returns the number of entries in the list
     * @post returns the value of count_
     */
    int length() const noexcept;


    /**
     * @brief returns the first entry in the list
     * @pre the list must not be empty
     * @post returns this->last_->payload_
     */
    std::optional<T> front() const;


    /**
     * @brief returns the last entry in the list
     * @pre the list must not be empty
     * @post return this->last_->payload_
     */
    std::optional<T> back() const;


    /**
     * @brief searches for a value in the list and returns a pointer to it (if it exists).
     * @post returns a pointer to a node containing the targetData, if it's in the list.  
     */
    std::optional<const Node<T>*> search(const T& targetData) const;


    /**
     * @brief inserts anew entry into the list
     * @pre if the list is nonempty, it must be in ascending order
     * @post a new node is created with the given argument as a payload in a position s.t. the list is in ascending order.
     */
    void insert(const T& data);


    /**
     * @brief deletes the given entry
     * @if the entry doesn't exist, prints error message and program continues
     * @post
     */
    void deleteEntry(const T& targetData);


      //default constructor
    DoublyLinkedList() : first_(nullptr), last_(nullptr), count_(0){}


      //copy constructor
    DoublyLinkedList(const DoublyLinkedList<T>&);


      //move constructor
    DoublyLinkedList(DoublyLinkedList<T>&&) noexcept;


      //destructor
    ~DoublyLinkedList() = default;//tells the compiler to just use the unique pointer destructor (also tells devs)
  protected:
    int count_;
    std::unique_ptr<Node<T>> head_;
    Node<T>* tail_;
  private:
    //postcondition: *this is a deep copy of the argument
    void copy(const DoublyLinkedList<T>&);
};
