// -*- mode: c++ -*-
//  copyright (C) 2001 Makoto Onizuka, NTT CyberSpace Labolatories
//  $Id: XPath.cxx,v 1.3 2002/08/13 12:59:46 monizuka Exp $

#define XPATH_EMBODY
#ifndef WIN32
#include <iostream.h>
#else
#include <iostream>
#endif

#include "Error.h"
#include "Query.h"
#include "Variable.h"
#include "XPath.h"

#include "List.cxx"
template class List<LocationStep>;
template class List<XPath>;
template class List<Predicate>;

static PString	memoryPool[PSTRING_STATIC_MEMORY];
static unsigned int	memoryCount = 0;

void *
PString::operator new(size_t s){
  return &memoryPool[memoryCount++];
}

void
PString::operator delete(void * p){
  memoryCount--;
}

const char * 
PString::getString(void) {
  switch(type){
  case T_XMLSTRING:
#if defined(XERCES_PARSER)
    if (str != 0) return str;
    str = XMLString::transcode(xstr);
    return str;
#endif
  case T_STRING:{
    char * s = (char *)malloc(length+1);
    strncpy(s, str, length);
    s[length] = '\0';
    return (const char *)s;
  }
#if defined(TOKEN_PARSER)
  case T_TOKEN:
    return g_ptt->StrFromXTOKEN(token);
#endif
  default:
    cerr << "inernal error@PString::getString()" << endl;
    return 0;
  }
}


XPath::XPath():Base(),var(0),automata(0),lsteps(new List<LocationStep>),parent(0) {}

void
XPath::deleteLast(void){
  try {
    lsteps->deleteItem(lsteps->getCount()-1);
    LocationStep * sx = lsteps->last();
    sx->next = 0;
  }
  catch (_Error & err){
    err.addItem(new ErrItem(this, "XPath::deleteLast()", __LINE__));
    throw err;
  }
}


LocationStep *
XPath::pop(void){
  try {
    return lsteps->pop();
  }
  catch (_Error & err){
    err.addItem(new ErrItem(this, "XPath::pop()", __LINE__));
    throw err;
  }
}

void
XPath::print(ofstream * ofs){
  try {
    for (unsigned int i = 0; i < lsteps->getCount(); i++){
      LocationStep * sp = lsteps->getItem(i);
      switch(sp->getType()){
      case LocationStep::DoubleSlash:
	* ofs << "/";
	break;
      case LocationStep::Period:
	* ofs << ".";
	break;
      case LocationStep::DoublePeriod:
	* ofs << "..";
	break;
      default:			// Element or Attribute
	if (sp->path) * ofs << sp->path;
	else {
	  * ofs << "(";
	  for (unsigned int j = 0; j < sp->altPath->getCount(); j++){
	    XPath * p = sp->altPath->getItem(j);
	    p->print(ofs);
	    if (j + 1 < sp->altPath->getCount()) * ofs << "|";
	  }
	  * ofs << ")";
	}
      }
      if (i + 1 < lsteps->getCount()) * ofs << '/';
    }
  }
  catch (_Error & err){
    err.addItem(new ErrItem(this, "XPath::print()", __LINE__));
    throw err;
  }
}

LocationStep::LocationStep(XPath * xp, nodeType n, char * p) : Base(), parent(xp), next(0), nType(n), pType(P_NoPredicate), altPath(0), preds(new List<Predicate>()),index(0) {
  try {
    if (n == Element){
      path = strdup(p);
#if defined(TOKEN_PARSER)
      token = g_ptt->XTOKENFromStr(p, XST_ELEMENT);
#endif
    }
    else if (n == Attribute){
      path = strdup(p);			// p contains '@'
#if defined(TOKEN_PARSER)
      token = g_ptt->XTOKENFromStr(p+1, XST_ATTRIBUTE);
#endif
    }
    else altPath = new List<XPath>();
    LocationStep * last = xp->getLastLStep();
    if (last != 0) last->next = this; // if this is not top
    xp->insertLStep(this);
  }
  catch (_Error & err){
    err.addItem(new ErrItem(this, "LocationStep::LocationStep()", __LINE__));
    throw err;
  }
}

LocationStep::~LocationStep(){
  try {
    if (path != 0) free(path);
    else {
      altPath->delItems();
      delete altPath;
    }
    for (unsigned int i = 0;i<preds->getCount();i++){
      delete preds->getItem(i);
    }
    delete preds;
  }
  catch (_Error & err){
    err.addItem(new ErrItem(this, "LocationStep::~LocationStep()", __LINE__));
    throw err;
  }
}
