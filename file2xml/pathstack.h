/* 
  
   Name: pathstack.h
   Description:
   This is a speciallized stack implementation that uses a 
   linked list.The items stored are full paths of files.

   Author: Iliana Avila-Campillo

   Date: July 24, 2001

*/
#include <iostream>
using namespace std;

// directory  manipulation
#include <unistd.h>     
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/param.h>

// file maipulation
#include <sys/stat.h>

#ifndef PATHSTACK_H_
#define PATHSTACK_H_

#ifndef NODE
#define NODE
class Node{
 public:
  char * abs_path;
  Node * next;
};
#endif   

class PathStack{

 public:
  // constructor
  PathStack();
  
  // destructor
  ~PathStack();

  // pops the stack 
  // deletes the poped item
  char * pop();
  
  // tops the stack
  // does not delete the toped item
  char * top();
  
  // push
  void push(char * abs_path);
  
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
