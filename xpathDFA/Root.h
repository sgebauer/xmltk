// -*- mode: c++ -*-
//  This is a Root module for xmatch processor.
//  copyright (C) 2001 Makoto Onizuka, University of Washington
//  $Id: Root.h,v 1.1.1.1 2002/05/04 12:53:57 tjgreen Exp $

/***********************************************************
Root object manages a parsed tree that is generated during the
yacc/lex processing. The root node in the tree is Root object
that is inherited from Node object and it manages 
1) variable declaration each of which binds XPath expression to a variable.
2) transformation declaration that is void for xpathDFA library.
   It is meaningful for xmatch.
 **********************************************************/


#if ! defined(__Root_H__)
#define __Root_H__
#ifndef WIN32
#include <hash_map.h>
#include <fstream.h>
#else
#include <hash_map>
#include <fstream>
#endif
#include "Variable.h"
#include "Node.h"

class Root;

class HashCharPtr
{
public: size_t
operator()(char const *str) const
  { 
	return std::hash<char const *>()(str);
  }
};
class EqualCharPtr
{
public: bool 
operator()(char const *x, char const *y) const 
  { 
	return !strcmp(x, y); 
  }
};

typedef hash_map<unsigned int, Variable *> VarPtrMap;

//
// Root class manages a collection of Node instances,
// that means it is a manager of all transformation rules.
//

class Root : public Node {
 protected:
  enum states {
    S_CONSTRACT = 0,		// 管理オブジェクトのみ存在
    S_OPEN = 1			// ファイルをオプーン成功
  };
  const char * const	filename; // xmatch query filename
  states	state;		// state
  VarPtrList *	variables;	// for variable declarations
  List<Node> *	elements;	// for transformation declarations
  enum errcodes {
    OK = 0,
    E_PARSE,			// yacc parse error
    E_MULTI_ROOT,		// There are more than two root node
    E_STATE,			// state error
    E_OTHERCLASS,		
    E_INTERNAL			// internal error
  };
  static char * errmes[];

private:
  void checkTree(void);

public:
  Root(void);					  // for xpath  (query is char *)
  Root(const char * const fname); // for xmatch (query is file)
  virtual ~Root();
  Variable * parse(void);		// return last Variable
  void open(void);
  void close(void);
  void insertVariable(Variable * vdef){			// for xpath
	variables->push_back(vdef);
  }
  Node * registerElement(char * label, nodeType t);
  VarPtrList * getVarPtrMap(void) { return variables; }
  Variable * getVariable(char * varName){
	return 0;
  }
			// debug
  void printQuery(ofstream*, char *);
};

#if defined (Root_EMBODY)
char * Root::errmes[] = {
  "",
  "Yacc parse error",
  "There are more than two root node", 
  "State error",
  "Other class error",
  "System internal error"
};

#endif
#endif
