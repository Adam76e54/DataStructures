#include <iostream>
#include <memory>
#include <optional>
#include <utility>
#include <functional>
#include <random>


template <typename T>
class BinaryTree{
private:
  struct Node{
    T payload_;
    std::unique_ptr<Node> left_, right_;


    explicit Node(T data) : payload_(std::move(data)), left_(nullptr), right_(nullptr){/*empty*/}
  };


  std::unique_ptr<Node> root_;


  //recursive helper
  void insert(std::unique_ptr<Node>& node, T&& data){
    //CASE 1: empty tree
    if(!node){
      node = std::make_unique<Node>(std::forward<T>(data));
      return;
    }
    //CASE 2: data < root
    if(data < node->payload_){
      insert(node->left_, std::forward<T>(data));
    }
    //CASE 3: data > root
    else{
      insert(node->right_, std::forward<T>(data));
    }
  }


  template <typename Function>
  //NOTE: we sometimes want the value to be preserved if it's an std::function, so we're using perfect forwarding
  //Otherwise, functions can break, degrade, or not work fully as intended
  //This is by far the best option in most cases (unless we're dealing with multithreaded code where we need to handle several operations and events)
  void preorder(const std::unique_ptr<Node>& node, Function&& process){
    using namespace std;
    if(node){
      process(node->payload_);
      preorder(node->left_, forward<Function>(process));
      preorder(node->right_, forward<Function>(process));
    }
  }


  //Example using std::function<>(), I decided ot just feed it the payload, rather than the entire node
  void inorder(const std::unique_ptr<Node>& node, const std::function<void(T&)>& process){
    using namespace std;
    if(node){
      inorder(node->left_, process);
      process(node->payload_);
      inorder(node->right_, process);
    }
  }


  //Example using a function pointer
  void postorder(const std::unique_ptr<Node>& node, void (*process)(T& payload)){
    using namespace std;
    if(node){
      postorder(node->left_, process);
      postorder(node->right_, process);
      process(node->payload_);
    }
  }


public:
  constexpr BinaryTree() noexcept = default;//root_'s default will be called to initialise it to nullptr


  //move and copy insertion (facilitates T being a large object)
  void insert(T&& data){
    //taking advantage of T's possible move constructor
    insert(root_, std::forward<T>(data));
  }


  template<typename Function>
  void inorder(Function&& process){
    inorder(root_, std::forward<Function>(process));
  }




  void preorder(std::function<void(T&)>& process){
    preorder(root_, process);
  }
 
 
  void postorder(void (*process)(T&)){
    postorder(root_, process);
  }


  bool contains(const T& key){
    auto node = root_.get();
    while(node){
      //CASE 1: key is found
      if(key == node->payload_){
        return true;
      }
      //CASE 2: node < target ==> go left
      else if(key < node->payload_){
        node = node->left_.get();
      }
      //CASE 3: node > target ==> go right
      else{
        node = node->right_.get();
      }
    }
    return false;
  }
};
