// -*- mode: c++ -*-
//  This is a Variable module for xmatch processor.
//  copyright (C) 2001 Makoto Onizuka, University of Washington
//  $Id: Variable.h,v 1.2 2002/08/13 12:59:46 monizuka Exp $

#if ! defined(__Variable_H__)
#define __Variable_H__
#include <stdlib.h>
#include <string.h>
#ifndef WIN32
#include <fstream.h>
#include <set.h>
#include <list.h>
#include <hash_set.h>
#else
#include <fstream>
#include <set>
#include <list>
#include <hash_set>
using namespace std;
#endif

class Variable;

#include "Base.h"
#include "XPath.h"

#define VAR_DEFAULT_PRECEDENCE 0.0

class HashPtr
{
public: size_t
operator()(void const * s) const
  { 
	return std::hash<unsigned int>()((unsigned int)s);
  }
};
class EqualPtr
{
public: bool 
operator()(void const *x, void const *y) const 
  { 
	return (x == y);
  }
};

typedef hash_set<Variable *,HashPtr,EqualPtr> VarPtrSet;
typedef list<Variable *> VarPtrList;
typedef List<Variable> VarPtrArray;

//
// Variable class expresses each variable in VAR clause
// like VAR $title in {$paper/title}.
// In this case, "$title" is mapped to Variable, 
// "$paper" is mapped as its parent variable,
// "/title" is mamped as its xpath.
//

class Variable : Base {
protected:
  VarPtrList	* children;	// children Variables
  XPath 	* xpath;
  float		  precedence;	// precdedance for prural variables are
				// matched with some input node (tag)
  size_t          lParam;	// application-defined data field
  Predicate	* pred;		// dynamic: this locks enableFlag
  void		* userSpace;
public:
  Variable 	* parent;	// parent variable
  bool		  outputFlag;
  bool		  enableFlag;	// dynamic: toggle (disable, enable)
  unsigned int	  childCount;	// cache (only for the treeAutomata)
  unsigned int	  depth;	// cache (only for the treeAutomata)

public:				// for xpath
  Variable(void) : Base(),xpath(0),precedence(VAR_DEFAULT_PRECEDENCE),pred(0),userSpace(0),parent(0),outputFlag(false),enableFlag(true),childCount(0),depth(0){
    children = new VarPtrList();
  }
  virtual ~Variable() {
    delete children;
    if (xpath){
      xpath->delSpaths();
      delete xpath;
    }
				// here, we need to free the userSpace
  }
  Variable * getParent(void){ return parent; }
  VarPtrList * getChildren(void){ return children; }
  unsigned int getChildCount(void) {return childCount; }
  void setParent(Variable * p){
    parent = p;
    p->children->push_back(this);
    p->childCount+=1;
    depth = parent->depth+1;
  }
  XPath * getXPath(void){ return xpath; }
  void setXPath(XPath * xp){
    xpath = xp;
    xpath->setVariable(this);
  }
  bool	getOutputFlag(void) { return outputFlag; }
  float	getPrecedence(void) { return precedence; }
  bool	getEnableFlag(void) { return enableFlag; }
  void	setOutputFlag(bool b) { outputFlag = b; }
  void	setOutputFlag(bool b, float prec) {
	outputFlag = b;
	precedence = prec;
  }
  bool  setEnableFlag(Predicate * p, bool b);

  size_t  getlParam(void) { return lParam; }
  void  setlParam(size_t l) { lParam = l; }
  void*	getUserSpace(void) { return userSpace; }
  void	setUserSpace(void * s) { userSpace = s; }
};


#endif
