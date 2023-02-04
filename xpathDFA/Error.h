/************************************************************************
// -*- mode: c++ -*-
//  copyright (C) 2001-2002 Makoto Onizuka, University of Washington
//  $Id: Error.h,v 1.1.1.1 2002/05/04 12:53:54 tjgreen Exp $
*************************************************************************/

#if ! defined(__XMLTK_ERR_H__)
#define __XMLTK_ERR_H__
#define ERROR_STACK_SIZE  30

#ifndef WIN32
#include <iostream.h>
#else
#include <iostream>
using namespace std;
#endif
#include <stdlib.h>
#include "Base.h"

class ErrItem {
 public:
  Base	      * lastobj;
  char        * lastmethod;
  int		errcode;

  ErrItem(Base * obj, char * mtd, int code):
    lastobj(obj), lastmethod(mtd), errcode(code) {};
  ~ErrItem() {};
  char * getMessage() {return lastmethod;}
  int    getErrcode() {return errcode;}
};

class EList {
protected:
  unsigned long size;
  unsigned long count;
  unsigned long next;
  ErrItem **	list;

public:
  EList(): size(0), count(0), next(0), list(0) {};
  virtual ~EList() {};
  void    alloc(unsigned int);
  void    freeSpace() {free(list);}
  unsigned long newItem();
  ErrItem* getItem(unsigned long pos);
  void 	  setItem(unsigned long pos, ErrItem *);
  unsigned long getSize() {return size;}
  unsigned long getCount() {return count;}
};

class _Error {
protected:
  EList * errlist;

public:
  _Error(Base * obj, char * mtd, int code) : errlist(new EList) {
    errlist->alloc(ERROR_STACK_SIZE);
    errlist->setItem(errlist->newItem(), new ErrItem(obj, mtd, code));
  }
  ~_Error(){};
  void addItem(ErrItem * ei) {errlist->setItem(errlist->newItem(), ei);}
  void perror(){
    ErrItem * ei;
    unsigned long count = errlist->getCount();
    for (unsigned long i = 0; i < count; i++){
	ei = errlist->getItem(i);
	cerr << i << "\t";
	cerr << ei->getMessage() << ", line = " << ei->getErrcode()<< endl;
    }
  }
};

#endif /* __XMLTK_ERR_H__ */

