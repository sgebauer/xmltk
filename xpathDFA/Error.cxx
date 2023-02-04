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
  if (list[pos] == NULL) count++; // �v�f���Ȃ��Ƃ���ɓ����ꍇ
  if (t == NULL){		// �폜����(NULL ��ݒ肷��)�ꍇ
    count--;
				// list[pos  - 1] �� NULL �łȂ����
				// �폜�����ꏊ�� next �ɂ��Ă���
    if ((pos > 0)&&(list[pos - 1] != NULL)) next = pos;
  }
  list[pos] = t;
}

