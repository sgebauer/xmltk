// -*- mode: c++ -*-
//  This is a Predicate module for xmatch processor.
//  copyright (C) 2001 Makoto Onizuka, University of Washington
//  $Id: Predicate.h,v 1.3 2002/08/13 12:59:46 monizuka Exp $
#if ! defined(__PREDICATE_H__)
#define __PREDICATE_H__

#include <string>

#include "operator.h"			// basic operator definition
#include "Base.h"
#include "Error.h"

typedef unsigned int	AccessCount;

class Predicate;
class XPath;
class Variable;
class Automata;
class PString;

class Expression : public Base {
public:
  enum types {
    T_XPATH = 0,
    T_VAL_STRING,
    T_VAL_INTEGER,
    T_VAL_FLOAT
  };
  XPath	  * xpath;		// relative location path expression
  class value {
  public:
    char  * string;
    int	    integer;
    float   real;
  } value;
  types	  valType;

public:
  Expression(XPath * xp): Base(), xpath(xp) {
    value.string = 0;	
  }
  Expression(int val): Base(),xpath(0),valType(T_VAL_INTEGER){
    value.string = 0;
    value.integer = val;
  }
  Expression(float val): Base(),xpath(0),valType(T_VAL_FLOAT){
    value.string = 0;
    value.real = val;
  }
  Expression(char * val): Base(),xpath(0),valType(T_VAL_STRING){
    value.string = val;
  }
  ~Expression() {
    if (value.string) free(value.string);
  }
  XPath * getXPath(void){ return xpath; }
};

class AtomPredicate : public Base {
public:
  enum types {
    T_INDEX = 0,
    T_UNARY,
    T_BINARY,
    T_FUNCTION
  };
  Expression	* left;	 // index, unary, or binary left
  unsigned int 	opr;	 // operator (=,!=,>,...) of binary expression
			 // defined at "operator.h"
  Expression	* right; // right side of binary expression
  types		oprType;
  static char * oprs[];
  static char * funcs[];

public:
  AtomPredicate(void): Base(),left(0),opr(MATH_OPR_NOT_DEFINED),right(0){}
  ~AtomPredicate(){
    if (left) delete left;
    if (right) delete right;
  }
  char * getOperator(void){ return oprs[opr]; }
  char * getFunction(void){ return funcs[opr]; }
  char * getRightKey(void){
    if (right) return right->value.string;
    else return "";
  }
};

//
// Predicate expresses a binary tree strucutre of selection condition (or a qualifier) 
// in XPath expression that is divided by AND, OR, (...).
// For example, in case [paper/author='Dan' and (paper/title='XML' or paper/title='DBMS')]
// Predicate are 
//   - root: [A and B]
//   - A: paper/author='Dan'
//   - B: (C or D)]
//   - C: paper/title='XML'
//   - D: paper/title='DBMS'
// in case [a and b and c] Predicate are
//   - root: [C and D]
//   - A: [a and b]
//   - B: c
//   - C: a
//   - D: b
//

class Predicate : public Base {
public:
  enum types {
    T_INDEX = 0,		// contains index predicate (exactly one)
    T_DOUBLESLASH_INDEX,	// contains //e[index] predicate (exactly one)
    T_SIMPLE_PREDICATE,		// "[@atr opr val][text() opr val]"
    T_PREDICATE			// all conditions are binary or unary predicate
  };
  enum boolOprTypes {
    T_NOT_DEFINED = 0,
    T_AND,
    T_OR
  };
  enum compTypes {		// only for T_INDEX type
    T_CONTINUE = 0,		// before such n as element[n]
    T_COMPLETE			// after  such n as element[n]
  };

  List<Predicate>* children;	// non-leaf node (for AND OR expressions)
  unsigned int	  opr;		// operator type (AND,OR) that connect left and right
  AtomPredicate * atom;		// leaf node
  Variable	* var;		// This controls this Variable.
  char		* path;		// for raw predicate expression
  types		type;		// static: Predicate type
  AccessCount   accessCount;	// dynamic: The number of accessed time 
				//          for e[n], //e[n]
  unsigned int	automataStackCount; // dynamic: for //e[n] reset
  compTypes	ctype;		// dynamic: only for T_INDEX type
protected:
  enum errcodes {
    OK = 0,
    E_ALLOC,
    E_OTHERCLASS,
    E_NOT_SUPPORT,
    E_TYPE_MISMATCH,
    E_PERIOD,
    E_INTERNAL
  };
  static char * errmes[];

public:
  Predicate(Variable * v, char * fp): Base(),children(new List<Predicate>()),opr(T_NOT_DEFINED),atom(0),var(v),path(fp)
			   ,type(T_PREDICATE),accessCount(0),automataStackCount(0),ctype(T_CONTINUE)
  {}
  virtual ~Predicate() {
    for (unsigned int i = 0; i < children->getCount(); i++){
      Predicate * p = children->getItem(i);
      delete p;
    }
    delete children;
    if (atom) delete atom;
    if (path) free(path);
  }
  bool apply(Automata * a, PString * ps);
				// for StackItem stack()
  AccessCount getAccessCount(void){ return accessCount; } 
				// for StackItem pop()
  void setAccessCount(AccessCount c){ accessCount = c; }  
  void reset(Automata * a);
  void resetAccessCount(Automata * a);
  XPath * getLeftXPath(void){ return atom->left->xpath; }
  char * getOperator(void){ return atom->getOperator(); }
  char * getFunction(void){ return atom->getFunction(); }
  char * getRightKey(void){ return atom->getRightKey(); }
  void   setType(void);
  // only for LocationStep::setPredTypeDoubleSlash()
  void  setType(types t){ type = t; }
  types getType(void){ return type; }
  AtomPredicate::types getOprType(void) {return atom->oprType; }
  unsigned int getIndex(void){
    if (type==T_PREDICATE){
      cerr << errmes[E_INTERNAL] << endl;
      throw _Error(this, "Predicate::getIndex()", __LINE__);
    }
    return atom->left->value.integer;
  }
  AtomPredicate * getTopAtom(void){
    if (atom) return atom;
    else {
      cerr << errmes[E_INTERNAL] << endl;
      throw _Error(this, "Predicate::getTopAtom()", __LINE__);
    }
  }
  bool 	isSatisfiable(void){
    if (ctype == T_COMPLETE) return true;
    else return false;
  }
  void	pred2string(string & s){s.append(path);}
};

#if defined (PREDICATE_EMBODY)
char * Predicate::errmes[] = {
  "",
  "Memory allocation error",
  "Other class error",
  "Not supported",
  "Parameters type mismatch",
  "Period in a predicate (for text() concatination) is not supported",
  "System internal error"
};
char * AtomPredicate::oprs[] = {
  "",
  "=",
  "!=",
  "<",
  ">",
  "<=",
  ">="
};
char * AtomPredicate::funcs[] = {
  "",
  "contains",
  "starts-with"
};
#endif

#endif
