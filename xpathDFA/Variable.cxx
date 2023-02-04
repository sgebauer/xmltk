// -*- mode: c++ -*-
//  copyright (C) 2001 Makoto Onizuka, University of Washington
//  $Id: Variable.cxx,v 1.2 2002/08/13 12:59:46 monizuka Exp $

#define VARIABLE_EMBODY
#ifndef WIN32
#include <iostream.h>
#else
#include <iostream>
#endif
#include "Variable.h"

bool
Variable::setEnableFlag(Predicate * p, bool b){
  if (b){
    if (pred && pred == p){	// release the lock
      pred = 0;
      enableFlag = b;
    }
  }
  else {			// b == false
    if (!pred){
      pred = p;			// lock the false
      enableFlag = b;
    }
  }
  return enableFlag;
}
