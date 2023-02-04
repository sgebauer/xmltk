// -*- mode: c++ -*-
//  copyright (C) 2001 Makoto Onizuka, NTT CyberSpace Labolatories
//  $Id: XPath.h,v 1.3 2002/08/13 12:59:46 monizuka Exp $

#if ! defined(__XPATH_H__)
#define __XPATH_H__
#include <stdlib.h>
#ifndef WIN32
#include <fstream.h>
#else
#include <fstream>
#endif
#if defined(XERCES_PARSER)
#include <util/XMLUniDefs.hpp>
#include <util/XMLString.hpp>
#endif
#if defined(TOKEN_PARSER)
#include <xmltk.h>
#endif

class XPath;
class LocationStep;
class Variable;
class Automata;

#include "Base.h"
#include "Error.h"
#include "List.h"
#include "Predicate.h"


class PString {
public:
  enum types {
    T_XMLSTRING = 0,
    T_TOKEN = 1,
    T_STRING = 2				// no terminator (for xmill XML parser)
  };
private:
#if defined(XERCES_PARSER)
  const XMLCh	* xstr;
#endif
  const char	* str;                          // no terminator
  unsigned int	  length;
  char	* str2;                                 // with terminator
#if defined(TOKEN_PARSER)
  XTOKEN		  token;
#endif
  types		  	  type;
public:
  PString(void): str(0),type(T_STRING){}
#if defined(XERCES_PARSER)
  PString(const XMLCh * s): xstr(s),str(0),str2(0),type(T_XMLSTRING){}
#endif
  PString(const char * s, unsigned int len): str(s),length(len),str2(0),type(T_STRING){}
#if defined(TOKEN_PARSER)
  PString(XTOKEN xt): str(0),token(xt),type(T_TOKEN){}
#endif
  ~PString(void){
    switch(type){
    case T_XMLSTRING:
      if (str2) free((void *)str2);
      break;
    case T_STRING:
#if defined(XERCES_PARSER)
      if (xstr) free((void *)xstr);
#endif
      if (str2) free((void *)str2);
      break;
    default:
      break;
    }
  }
  void * PString::operator new(size_t s);
  void PString::operator delete(void * p);

#if defined(TOKEN_PARSER)
  const XTOKEN getKey(void) { return token; }
#else
  const char * getKey(void) {
    switch(type){
    case T_XMLSTRING:
#if defined(XERCES_PARSER)
      if (str2) return str2;
      str2 = XMLString::transcode(xstr);
      return str2;
#endif
    case T_STRING:{
      str2 = (char *)malloc(length+1);
      strncpy(str2, str, length);
      str2[length] = '\0';
      return (const char *)str2;
    }
    default:
      cerr << "internal error@PString::getKey" << endl;
      return 0;
    }
  }
#endif
  const char * getString(void);
#if defined(TOKEN_PARSER)
  int comp(XTOKEN xt){ return xt == token; }
#else  
  int comp(char * s){
    if (str){
      if (strlen(s)!= length) return 1;
      return strncmp(str, s, length);
    }
    switch(type){
    case T_XMLSTRING:
#if defined(XERCES_PARSER)
      str = XMLString::transcode(xstr);
#endif
      break;
    case T_STRING:
      cerr << "internal error@PString::comp" << endl;
      break;
    default:
      break;
    }
    return strcmp(str, s);
  }
#endif
};

inline ostream& operator<<(ostream& target, PString * toDump){
  target << toDump->getString();
  return target;
}

//
// XPath class expresses an XPath expression like
// //a[cond]/(b|c|d[cond])/(e[cond]|f).
// Each path divided by '/' is mapped to LocationStep.
// Each part divided by '[' ... ']' is mapped to Predicate
//

class XPath : public Base {
protected:
  Variable	     * var;	// the variable bind the XPath
  Automata	     * automata;// automata for the XPath
  List<LocationStep> * lsteps;	// b and c are mapped to LocationStep
  XPath		     * parent;	// for XPath with predicates and altPath
  enum errcodes {
    OK = 0,
    E_ALLOC,
    E_OTHERCLASS,
    E_INTERNAL			// system internal error
  };
  static char * errmes[];

public:				// creation and deletion
  XPath();
  XPath(XPath * xp){
    * this = * xp;
    lsteps = new List<LocationStep>(xp->lsteps);
  }
  virtual ~XPath() { delete lsteps; }
  void delSpaths(void){ lsteps->delItems(); }
  Variable * getVariable(void) { return var; }
  void setVariable(Variable * v){ var = v; }
  Automata * getAutomata(void) { return automata; }
  void	  setAutomata(Automata * a){ automata = a; }
  XPath * getParent(void){
    if (parent == 0) return 0;
    else return parent;
  }
  void	  setParent(XPath *xp) {parent = xp;}

public:				// lsteps operation
  void insertLStep(LocationStep * sp) {
    try {
      lsteps->insertItem(sp);
    }
    catch (_Error & err){
      err.addItem(new ErrItem(this, "XPath::insertLStep()", __LINE__));
      throw err;
    }
  }
  unsigned int 	getLStepCount(void) {return lsteps->getCount();}
  LocationStep * getLStep(unsigned int pos) {return lsteps->getItem(pos);}
  void deleteLast(void);
  LocationStep * getTopLStep(void) {return lsteps->top();}
  LocationStep * getLastLStep(void) {return lsteps->last();}
  LocationStep * pop(void);
  void print(ofstream * ofs);
};

class LocationStep : public Base {
  friend class XPath;
public:
  enum nodeType {
    Element = 0,
    Attribute,
    Text,			// text()
    Period,			// '.'
    DoublePeriod,		// '..'
    AltElement,			// choice (A|B|...)
    AnyElement,			// *
    AnyAttribute,		// @*
    DoubleSlash,		// '//'
    DocumentRoot		// '/'
  };
  enum predType {
    P_NoPredicate = 0,
    P_Index,
    P_SimplePredicate,		// all predicates are T_SIMPLE_PREDICATE
    P_Predicate			// general predicate
  };

protected:
  XPath		* parent;	// parent XPath
  LocationStep	* next;		// link to next LocationStep
  nodeType	  nType;
  predType	  pType;	// Predicate type (non, index, predicate)
  char 		* path;		// This stores simple path like A, '//', '.', and '..'.
#if defined(TOKEN_PARSER)
  XTOKEN	  token;	// token expression of the path
#endif
  List<XPath>   * altPath;	// This stores alternative expression like (A|B/C|//D|...).
  List<Predicate> * preds;	// predicate expression
  Predicate	  * index;	// this is a cache of Predicates' index 
  enum errcodes {
    OK = 0,
    E_ALLOC,
    E_OTHERCLASS,
    E_MULTIPLE_INDEXPREDICATE,	// there is a multiple index predicate
    E_DOUBLESLASH_INDEX,	// there is //e[n]
    E_NOT_SUPPORT,
    E_INTERNAL			// system internal errror
  };
  static char * errmes[];

public:				// for yacc
  LocationStep(XPath * xp, nodeType n, char * p);
  virtual ~LocationStep();

  void insertAltPath(XPath * p){
    p->setParent(this->parent);
    altPath->insertItem(p);
  }
  void addPredicate(Predicate * p){ preds->insertItem(p); }
  void setType(void){		// update the pType
    for (unsigned int i=0;i<preds->getCount();i++){
      Predicate * p = preds->getItem(i);
      switch(p->getType()){
      case Predicate::T_INDEX:
	if (index){
	  cerr << errmes[E_MULTIPLE_INDEXPREDICATE] << endl;
	  throw _Error(this, "LocationStep::addPredicate()", __LINE__);
	}
	pType = P_Index;
	index = p;		// cache the predicate as "index"
	break;
      case Predicate::T_SIMPLE_PREDICATE:
	if (pType==P_NoPredicate) pType = P_SimplePredicate;
	break;
      case Predicate::T_PREDICATE:
	if (pType==P_NoPredicate||pType==P_SimplePredicate) pType = P_Predicate;
	break;
      default:		// T_DOUBLESLASH_INDEX
	break;		// do nothing
      } // end switch
    }
    return;
  }
  Predicate * getLastPred(void) {return preds->last();}
  void setPredTypeDoubleSlash(void){
    if (index) {
      index->setType(Predicate::T_DOUBLESLASH_INDEX);
      cerr << errmes[E_DOUBLESLASH_INDEX] << endl;
      throw _Error(this, "LocationStep::addPredicate()", __LINE__);
    }
  }
  nodeType getType(void) { return nType; }
  predType getPredType(void) { return pType; }
  Predicate * getIndexPredicate(void) { return index; }
  List<Predicate> * getPredicates(void) { return preds; }
  void	   swapPredicates(LocationStep * to){
    List<Predicate> * tlp = to->preds;
    to->preds = preds;
    preds = tlp;
  }
#if defined(TOKEN_PARSER)
  XTOKEN getPath(void) { return token; }
#else
  char * getPath(void) { return path; }
#endif
  char * getLabel(void){
    switch(nType){
    case Element:
    case Attribute:
      return path;
    case Text:
      return "text()";
    case Period:
      return ".";
    case DoublePeriod:
      return "..";
    case AnyElement:
      return "*";
    case AnyAttribute:
      return "@*";
    case DoubleSlash:
      return "//";
    case DocumentRoot:
      return "/";
    default:					// AltElement
      cerr << errmes[E_INTERNAL] << endl;
      throw _Error(this, "LocationStep::getLabel()", __LINE__);
    }
  }
  void	simplePred2string(string & s){
    for (unsigned int i=0;i<preds->getCount();i++){
      Predicate * p = preds->getItem(i);
      switch (p->getType()){
      case Predicate::T_INDEX:
      case Predicate::T_DOUBLESLASH_INDEX:
      case Predicate::T_PREDICATE:
				// do nothing
	break;
      case Predicate::T_SIMPLE_PREDICATE:
	s.append("[");
	p->pred2string(s);
	s.append("]");
	break;
      }
    }
  }
  void	indexPred2string(string & s){
    for (unsigned int i=0;i<preds->getCount();i++){
      Predicate * p = preds->getItem(i);
      switch (p->getType()){
      case Predicate::T_INDEX:
      case Predicate::T_DOUBLESLASH_INDEX:
	if (i==0) {
	  s.append("[");
	  p->pred2string(s);
	  s.append("]");
	}
	else {
	  cerr << errmes[E_NOT_SUPPORT] << endl;
	  throw _Error((Base *)this, "LocationStep::indexPred2string()", __LINE__);
	}
	break;
      case Predicate::T_PREDICATE:
      case Predicate::T_SIMPLE_PREDICATE:
				// do nothing
	break;
      }
    }
  }
  
  
public:				// for Query class
  bool match(PString * ps){
    if (nType == AnyElement) return true;
    if (ps == 0 || path == 0) return false;
#if defined(TOKEN_PARSER)
    if (ps->comp(token)==0) return true;
#else	
    if (ps->comp(path)==0) return true;
#endif
    else return false;
  }
  unsigned int getAltPathCount(void){ return altPath->getCount(); }
  XPath * getAltPath(unsigned int i){ return altPath->getItem(i); }
  inline bool
  operator <(const LocationStep & sp) const{
    if (nType != sp.nType) return nType < sp.nType;
    else {
#if defined(TOKEN_PARSER)
      return token < sp.token;
#else
      if (path) return strcmp(path, sp.path) < 0;
      else {					// case altPath
	cerr << errmes[E_INTERNAL] << endl;
	throw _Error((Base *)this, "LocationStep::equal()", __LINE__);
      }
#endif
    }
  }
  inline bool
  operator ==(const LocationStep & sp) const{ 
    if (nType != sp.nType) return false;
    else {
#if defined(TOKEN_PARSER)
      return token == sp.token;
#else
      if (path) return strcmp(path, sp.path) == 0;
      else {					// case altPath
	cerr << errmes[E_INTERNAL] << endl;
	throw _Error((Base *)this, "LocationStep::equal()", __LINE__);
      }
#endif
    }
  }
};

#if defined (XPATH_EMBODY)
char * XPath::errmes[] = {
  "",
  "Memory allocation error",
  "Other class error",
  "System internal error"
};

char * LocationStep::errmes[] = {
  "",
  "Memory allocation error",
  "Other class error",
  "Multiple index predicate \"element[pred]...[n]...[m]...[pred]\" is not proceeded.\n Please use single index predicate.",
  "...//element[index] is not supported yet",
  "Not supported",
  "System internal error"
};

#endif
#endif
