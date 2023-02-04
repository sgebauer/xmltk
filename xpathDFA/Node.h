// -*- mode: c++ -*-
//  This is a Node module for xmatch processor.
//  copyright (C) 2001 Makoto Onizuka, University of Washington
//  $Id: Node.h,v 1.1.1.1 2002/05/04 12:53:55 tjgreen Exp $

/***********************************************************
Node object manages a tranformation declaration in for xmatch
expression. The expression can be a collection of
    {element} := {child element (XPath expression list)}*
But it is not decided yet.
 **********************************************************/

#if ! defined(__NODE_H__)
#define __NODE_H__
#include <stdlib.h>
#ifndef WIN32
#include <fstream.h>
#else
#include <fstream>
#endif

#include "Base.h"
#include "Error.h"
#include "List.h"
#include "XPath.h"

class Root;
class CNodeDef;
class Node;

//
// Node class exprsses an each transformation rule like
// bib ::= (paper {/bib/paper})
// In this case, "bib" is mapped to Node and 
// "paper" is mapped as one of CNodeDef in childNodes.
// Each CNodeDef has a link to a Node.
//

class Node : public Base {
  friend class Root;
  friend class CNodeDef;
public:
  enum states {
    S_INACTIVE = 0,		// not outputted
    S_COMPLETE			// outputted already
  };
  enum nodeType {
    Element = 1,		// non-leaf element definition
    EmptyElement = 2,		// empty element definition
    AnyElement = 3,		// any 	element definition
    TextNode = 4,		// text element definition
    Attribute = 5,		// attribute definition
    pseudoSeqElement = 6,	// an item expressed (,,) in content model
    pseudoSelElement = 7,	// an item expressed (||) in content model
    pseudoOneElement = 8	// an item expressed (A) in content model
  };

protected:
  states	state;		// The status for transformation processing
  nodeType	nType;
  List<Node>  * parentNodes;	// List of parent nodes
  List<CNodeDef>  * childNodes;	// List of child nodes using CNodeDef
  char 	      *	name;		// nodeName
  enum errcodes {
    OK = 0,
    E_ALLOC,
    E_OTHERCLASS,
    E_INTERNAL			// sysytem internal errror
  };
  static char * errmes[];

public:
  Node() : Base(), state(S_INACTIVE), parentNodes(new List<Node>), childNodes(new List<CNodeDef>), name(0) {} // for Root
  Node(nodeType n) : Base(), state(S_INACTIVE), nType(n), parentNodes(new List<Node>), childNodes(new List<CNodeDef>), name(0) {} //for pseudoElement
  Node(char * name, nodeType t); // for Element and Attribute
  virtual ~Node() {
	delete parentNodes;
	childNodes->delItems(); delete childNodes;
	if (name != NULL) free(name);
  }

public:
  nodeType getNodeType(void) { return nType; }
  void   setNodeType(nodeType t) {
    if ((nType == TextNode && t == Element)||
	(nType == pseudoOneElement && t == pseudoOneElement)||
	(nType == pseudoOneElement && t == pseudoSeqElement)||
	(nType == pseudoOneElement && t == pseudoSelElement)
	)
      nType = t;
    else {
      cerr << "Node:" << name << " conflicts. " << (int)t << " and " << (int)nType << endl;
      throw _Error(this, "Node::setNodeType()", __LINE__);
    }
  }
  char * getNodeName() const { return name; }
  List<Node> * getParentNodes() const { return parentNodes; }
  Node * getLastParent() const { return parentNodes->last(); }
				// for root tempalte
  unsigned int getChildCount() const { return childNodes->getCount(); }
  CNodeDef * getCNodeDef(unsigned long pos) const {
    return childNodes->getItem(pos);
  }
  unsigned int insert(CNodeDef * d);
  void	 parentElements(List<Node> *);

public:
  void startDocument(void){
    state = S_COMPLETE;
    cout << "<" << name << ">" << endl;
  }
  void endDocument(void){
    state = S_INACTIVE;
    cout << "</" << name << ">" << endl;
  }

protected:
  void   check(void);
  void   reorderChildNodes(List<Node>*); 
  void   print(ofstream *);
private:
  void   indent(void);
  void   indent(ofstream *);
  void   printSub(ofstream *, List<Node> *, CNodeDef *);
};


class CNodeDef : public Base {	// Child node 
  friend class Node;
public:
  enum states {
    S_INACTIVE = 0,		// not outputted
    S_WAIT,			// not outputted and wait for others complete
    S_COMPLETE			// outputted already
  };
private:
  List<XPath> * skolemParams; // paramters for skolem function
public:
  Node *  node;			// Reference to Node definition
  XPath * valXPath;		// XPath List for value map
  states  state;		// The status of transformation processing

public:
  CNodeDef(Node * n): Base(), skolemParams(0), node(n), valXPath(0), state(S_INACTIVE) {}
  ~CNodeDef() {
    if (skolemParams != NULL){
      for (unsigned int i; i < skolemParams->getCount(); i++){
	XPath * xp = skolemParams->getItem(i);
	if (xp != 0){
	  xp->delSpaths();
	  delete xp;
	}
      }
      delete skolemParams;
    }
  }
  Node::nodeType getNodeType() const { return node->nType; }
  void   setSkolemParam(List<XPath> * xp) { skolemParams = xp; }
private:
  void	 indent(void);
  void   indent(ofstream *);
  void   printSkolemParams(ofstream * ofs);
};


#if defined (NODE_EMBODY)
char * Node::errmes[] = {
  "",
  "Memory allocation error",
  "Other class error",
  "System internal error"
};

#endif
#endif
