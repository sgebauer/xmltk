/************************************************************************
// -*- mode: c++ -*-
//  copyright (C) 2001 Makoto Onizuka, University of Washington
//  $Id: List.cxx,v 1.1.1.1 2002/05/04 12:53:54 tjgreen Exp $
*************************************************************************/
#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LIST_EMBODY
#include "List.h"

template<class T> 
List<T>::List(): Base(), count(0), next(0) {
    size = DEF_CHILD_COUNT;
    if ((list = (T **)malloc(sizeof(T*) * size)) == NULL){
	throw _Error(this, "List::alloc()", __LINE__);
    }
    for (unsigned int i = 0; i < size; i++) list[i] = NULL;
}

template<class T> 
List<T>::List(unsigned int c): Base(), count(0), next(0) {
    size = c;
    if ((list = (T **)malloc(sizeof(T*) * size)) == NULL){
	throw _Error(this, "List::alloc()", __LINE__);
    }
    for (unsigned int i = 0; i < size; i++) list[i] = NULL;
}

template<class T> 
List<T>::List(const List<T> * const target): Base(){
  size = target->size;
  count = next = target->count;
  if ((list = (T **)malloc(sizeof(T *) * size)) == NULL){
      throw _Error(this, "List(List<T> *)", __LINE__);
  }
  memcpy(list, target->list, sizeof(T *) * size);
}

template<class T> 
List<T>::~List(){
//  pthread_mutex_destroy(&lock);
  free(list);
}

template<class T> 
const unsigned long List<T>::newItem(){
  if (size == count){		// In case the list is full,
      T ** oldList = list;	// then extend the child list size.
      if ((list = (T **)malloc(sizeof(T*) * (size+DEF_EXTEND_COUNT)))==NULL){
	  throw _Error(this, "List::alloc()", __LINE__);
      }
      memcpy(list, oldList, sizeof(T *) * count);
      size += DEF_EXTEND_COUNT;
      for (unsigned int i = count; i < size; i++) list[i] = NULL;
      free (oldList);
  }
  if (list[next] == NULL) return next++;
  for (unsigned long i = 0; i < size; i++){
      if (list[i] == NULL){
	  next = i + 1;
	  return i;
      }
  }
  throw _Error(this, "List::newItem()", __LINE__);
}

template<class T> 
T * List<T>::deleteItem(const unsigned long pos){
  T * ret = setItem(pos, NULL);
  for (unsigned int i = pos; i < count + 1; i++){
    list[i] = list[i+1];
  }
  return ret;
}

template<class T> 
T * List<T>::getItem(const unsigned long pos){
  if(pos >= size) return NULL;
  return list[pos];
}

template<class T> 
T * List<T>::setItem(const unsigned long pos, T * t){
  if(pos >= size){
      throw _Error(this, "List::getItem()", __LINE__);
  }
  T * old = list[pos];
  if (list[pos] == NULL) count++; // —v‘f‚ª‚È‚¢‚Æ‚±‚ë‚É“ü‚ê‚éê‡
  if (t == NULL){		// íœ‚·‚é(NULL ‚ðÝ’è‚·‚é)ê‡
    count--;
    if (pos==0) next = 0;
				// list[pos  - 1] ‚ª NULL ‚Å‚È‚¯‚ê‚Î
				// íœ‚µ‚½êŠ‚ð next ‚É‚µ‚Ä‚¨‚­
    else if ((pos > 0)&&(list[pos - 1] != NULL)) next = pos;
  }
  list[pos] = t;
  return old;
}

template<class T> 
void List<T>::delItems(void){
  for (unsigned long i = 0; i < size; i++){
      if (list[i] != NULL) {
	delete list[i];
	list[i] = 0;
      }
  }
  count = next = 0;
}
