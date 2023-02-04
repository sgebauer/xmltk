/************************************************************************
// -*- mode: c++ -*-
//  copyright (C) 2001-2002 Makoto Onizuka, University of Washington
//  $Id: List.h,v 1.1.1.1 2002/05/04 12:53:54 tjgreen Exp $
*************************************************************************/
#if ! defined(__XMLTK_LIST_H__)
#define __XMLTK_LIST_H__

#include <stdio.h>
#include <stdlib.h>
#include "Base.h"
#define DEF_CHILD_COUNT 16
#define DEF_EXTEND_COUNT 16

extern "C" {
  extern int addressComp(const void * a, const void * b);
}

template<class T> class List : public Base {
protected:
  unsigned long size;			// �v�f���i�[�ł���̈�̐�
  unsigned long count;			// �i�[���Ă���v�f��
  unsigned long next;			// ���̋󂫔Ԓn
  T **		list;				// �v�f�ւ̃|�C���^�̃��X�g

  enum errcodes {
    OK = 0,
    E_ALLOC,					// �������m�ۃG���[
    E_PARAM,					// �p�����[�^�G���[
    E_FULL,						// �v�f�������ς�
    E_SUPPORT,					// �T�|�[�g����Ă��Ȃ��@�\�ł���
    E_OTHERCLASS,				// ���N���X�̃��\�b�h���G���[�ł���
    E_INTERNAL					// ���������G���[
  };
  static char * errmes[];
  const unsigned long  newItem(); // �󂫗̈�̔Ԓn������ė���B

public:
  List();						// �Ǘ��I�u�W�F�N�g�̍쐬
  List(unsigned int c);			// �Ǘ��I�u�W�F�N�g�̍쐬
  List(const List<T> * const target);	// �R�s�[�R���X�g���N�^(���e�̃|�C���^���R�s�[)
  virtual ~List();
  unsigned long  insertItem(const T * t){
	if (t == 0) return 0;
	const unsigned long pos = newItem();
	count++;
	list[pos] = (T *)t;
	return pos;
  }
  void sort(void){				// sort by address
	if (count > 0){
	  qsort(list, count, sizeof(T *), addressComp);
	}
  }
  unsigned int sortDistinct(void){ // sort by address
	if (count > 1){
	  qsort(list, count, sizeof(T *), addressComp);
	  unsigned int j = 0;
	  unsigned int sig = (unsigned int)list[j];
	  for (unsigned int i = 1; i<count; i++){
		while (list[j]==list[i]){
		  if (i != count-1) i++;
		  else{
			count = j+1;
			return sig;
		  }
		}
		j++;
		if (j != i){
		  list[j] = list[i];
		}
		sig ^= (unsigned int)list[j];
	  }
	  count = j+1;
	  return sig;
	}
	return 0;
  }
  unsigned int isortDistinct(void){ // sort by address (inserting sort)
	if (count > 0){
	  unsigned int j = 0;
	  unsigned int sig = (unsigned int)list[j];
	  for (unsigned int i = 1; i<count; i++){
		while (list[j]==list[i]){
		  if (i != count-1) i++;
		  else{
			count = j+1;
			return sig;
		  }
		}
		j++;
		if (j != i){
		  list[j] = list[i];
		}
		sig ^= (unsigned int)list[j];
	  }
	  count = j+1;
	  return sig;
	}
	return 0;
  }
  unsigned long  insertDistinctItem(T * t){
	for (unsigned int i = 0; i<count; i++){
	  if (list[i] == t) return i;
	}
	const unsigned long pos = newItem();
	setItem(pos, t);
	return pos;
  }
  T 	* setItem(const unsigned long pos, T *); // operator= �̒�`�̕����G���K���g
                                           // (�Â����͍̂폜���Ȃ�)
  long  member(const T * const t){
      unsigned long i;
      for (i = 0; i < count; i++){
	  if (list[i] == t){
	      return 0;
	  }
      }
      return -1;
  }
  T     * deleteItem(const unsigned long pos); // pos �Ԗڂ̍폜(���͍̂폜���Ȃ�)
  long  deleteItem(const T * const t){
    unsigned long i;
    for (i = 0; i < count; i++){
      if (list[i] == t){
	setItem(i, NULL);
	for (unsigned int j = i; j < count + 1; j++){
	  list[j] = list[j+1];
	}
	return i;
      }
    }
    return -1;
  }
  void    clearItems(void){
    for (unsigned long i = 0; i < size; i++){
      if (list[i] != 0) list[i] = 0;
    }
    count = next = 0;
  }
  T 	* getItem(const unsigned long pos); // operator[]�̒�`�̕����G���K���g
  T	* pop(){
    if (count == 0) return NULL;
	T * old = list[count-1];
	list[count-1] = NULL;
	count--;
	next--;
    return old;
  }
  T	* top(){
      if (count==0) return NULL;
      else return list[0];
  }
  T	* last(){
      if (count==0) return NULL;
      else return list[count-1];
  }
  void  delItems(void);
  unsigned long getSize() const {return size;}
  unsigned long getCount() const {return count;}
  void    swap(unsigned int a, unsigned int b){
    if (a == b) return;
    T * tmp = list[a];
    list[a] = list[b];
    list[b] = tmp;
  }
};

// static template<class T> char * List<T>::errmes[] = {
//   "",
//   "Some Parameters are incorrect",
//   "Memory allocation error"
// };
#endif /*  __XMLTK_LIST_H__*/
