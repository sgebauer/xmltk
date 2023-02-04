/* 
  
   Name: pathstack.cxx
   Description:
   This is a speciallized stack implementation that uses a 
   linked list.The items stored are full paths of files.

   Author: Iliana Avila-Campillo

   Date: July 10, 2001

*/

#include "pathstack.h"     

// constructor
PathStack::PathStack(){
  stack = NULL;
  size = 0;
}

// destructor
PathStack::~PathStack(){
  while(!is_empty()){
    pop();
  }
  stack = NULL;
}

// pops the stack 
// deletes the poped item
// returns NULL if the stack is empty
char * PathStack::pop(){
  if(size == 0)
    return NULL;
  char * path = stack->abs_path;
  Node * temp = stack;
  stack = stack->next;
  delete temp;
  size--;
  return path;
}

// tops the stack
// does not delete the toped item
char * PathStack::top(){
  if(size == 0)
    return NULL;

  return stack->abs_path;

}

// push
void PathStack::push(char * abs_path){
  Node * newNode = new Node();
  if(newNode == NULL){
    cout << " PathStack::push() -Could not create a new node." << endl;
  }
  newNode->abs_path = abs_path;
  newNode->next = stack;
  stack = newNode;
  size++;
}

// get size
int PathStack::get_size(){
  return size;
}

bool PathStack::is_empty(){
  return (size == 0);
}

