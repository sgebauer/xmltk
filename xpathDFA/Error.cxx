/************************************************************************
// -*- mode: c++ -*-
//  copyright (C) 2001 Makoto Onizuka, University of Washington
//  $Id: Error.cxx,v 1.1.1.1 2002/05/04 12:53:54 tjgreen Exp $
*************************************************************************/

#define ELIST_EMBODY
#include "Error.h"

void EList::alloc(unsigned int newsize){
    size = newsize;
    list = (ErrItem **)malloc(sizeof(ErrItem *) * size);
    for (unsigned int i = 0; i < size; i++)
	list[i] = NULL;
}

unsigned long EList::newItem(){
  if (list[next] == NULL) return next++;
  for (unsigned long i = 0; i < size; i++){
      if (list[i] != NULL){
	  next = i + 1;
	  return i;
      }
  }
  return size;
}

ErrItem * EList::getItem(unsigned long pos){
  if(pos >= size) return NULL;
  return list[pos];
}

void EList::setItem(unsigned long pos, ErrItem * t){
  if(pos >= size) return;
  if (list[pos] == NULL) count++; // 要素がないところに入れる場合
  if (t == NULL){		// 削除する(NULL を設定する)場合
    count--;
				// list[pos  - 1] が NULL でなければ
				// 削除した場所を next にしておく
    if ((pos > 0)&&(list[pos - 1] != NULL)) next = pos;
  }
  list[pos] = t;
}

