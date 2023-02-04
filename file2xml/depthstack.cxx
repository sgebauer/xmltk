/* 
  
   Name: depthstack.cxx
   Description:
   This is a speciallized stack implementation that uses a 
   linked list.The items stored depths of directories.

   Author: Iliana Avila-Campillo

   Date: August 20, 2001

*/

#include "depthstack.h"     

// constructor
DepthStack::DepthStack(){
  stack = NULL;
  size = 0;
}

// destructor
DepthStack::~DepthStack(){
  while(!is_empty()){
    pop();
  }
  stack = NULL;
}

// pops the stack 
// deletes the poped item
// returns NULL if the stack is empty
int  DepthStack::pop(){
  if(size == 0)
    return -99;
  int  depth = stack->depth;
  Node * temp = stack;
  stack = stack->next;
  delete temp;
  size--;
  return depth;
}

// tops the stack
// does not delete the toped item
int DepthStack::top(){
  if(size == 0)
    return -99;

  return stack->depth;

}

// push
void DepthStack::push(int depth){
  Node * newNode = new Node();
  if(newNode == NULL){
    cout << " DepthStack::push() -Could not create a new node." << endl;
  }
  newNode->depth = depth;
  newNode->next = stack;
  stack = newNode;
  size++;
}

// get size
int DepthStack::get_size(){
  return size;
}

bool DepthStack::is_empty(){
  return (size == 0);
}

