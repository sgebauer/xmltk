/* 
  
   Name: depthstack.h
   Description:
   This is a speciallized stack implementation that uses a 
   linked list.The items stored are integers representing
   depths of directories.

   Author: Iliana Avila-Campillo

   Date: August 20, 2001

*/
#include <iostream>
using namespace std;

#ifndef DEPTHSTACK_H_
#define DEPTHSTACK_H_

#ifndef NODE
#define NODE
class Node{
 public:
  int depth;
  Node * next;
};   
#endif

class DepthStack{

 public:
  // constructor
  DepthStack();
  
  // destructor
  ~DepthStack();

  // pops the stack 
  // deletes the poped item
  int pop();
  
  // tops the stack
  // does not delete the toped item
  int top();
  
  // push
  void push(int d);
  
  // get size
  int get_size();
  
  // returns true if the stack is empty
  bool is_empty();

 private:

  // the top of the stack
  Node * stack;
  // the size of the stack
  int size;
};

#endif
