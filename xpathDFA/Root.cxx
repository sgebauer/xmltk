// -*- mode: c++ -*-
//  copyright (C) 2001 Makoto Onizuka, University of Washington
//  $Id: Root.cxx,v 1.2 2002/08/13 12:59:46 monizuka Exp $

#define Root_EMBODY
#include "Error.h"
#include "Root.h"
#include "List.cxx"
#include "Query.h"

extern int yyparse(void);	// to link yyparse
extern FILE * yyin;

template class List<Variable>;

Root::Root(const char * const fname) : Node(), filename(fname), state(S_CONSTRACT) {
  try {
    variables   = new VarPtrList();
    elements    = new List<Node>();
  }
  catch (_Error & err){
    err.addItem(new ErrItem(this, "Root::Root()", __LINE__));
    throw err;
  }
}

Root::Root(void) : Node(),filename(0),state(S_CONSTRACT){
  try {
    variables   = new VarPtrList();
    elements    = new List<Node>();
  }
  catch (_Error & err){
    err.addItem(new ErrItem(this, "Root::Root()", __LINE__));
    throw err;
  }
}

Root::~Root(){
  if (state != S_CONSTRACT)
    throw _Error(this, "Root::~Root", __LINE__);
  try {
	for (VarPtrList::iterator itr = variables->begin(); itr != variables->end(); ++itr){
	  delete *itr;
	}
    delete variables;
	for (unsigned int i = 0; i <elements->getCount();i++){
	  delete elements->getItem(i);
	}
    delete elements;
  }
  catch (_Error & err){
    err.addItem(new ErrItem(this, "Root::~Root()", __LINE__));
    throw err;
  }
}

// register Element
// If there is a registerd Element with the same,
Node *			// this function returns the element.
Root::registerElement(char * label, nodeType t){
  try{
    Node * n;
    unsigned long count = elements->getCount();
    for (unsigned long i = 0; i < count; i++){
      n = elements->getItem(i);
      if (strcmp(label, n->getNodeName()) == 0){
	if (n->getNodeType() == Node::TextNode)
	  n->setNodeType(t);
	return n;
      }
    }
    n = new Node(label, t);
    elements->insertItem(n);
    return n;
  }
  catch (_Error & err){
    err.addItem(new ErrItem(this, "Root::registerElement()", __LINE__));
    throw err;
  }
}

Variable *						// return last Variable
Root::parse(void){
  try{
    if (yyparse()){
	  throw _Error(this, "Root::parse()", __LINE__);
	}
	VarPtrList::reverse_iterator itr = variables->rbegin();
	return *itr;
  }
  catch (_Error & err){
    err.addItem(new ErrItem(this, "Root::parse()", __LINE__));
    throw err;
  }
}

void
Root::open(void){
  try {
	if (state != S_CONSTRACT)
	  throw _Error(this, "Root::open", __LINE__);
	if ((yyin = fopen(filename , "r")) == NULL)
	  throw _Error(this, "Root::open", __LINE__);
	state = S_OPEN;
  }
  catch (_Error & err){
    err.addItem(new ErrItem(this, "Root::open()", __LINE__));
    throw err;
  }
}

void
Root::close(void){
  if (state != S_OPEN)
    throw _Error(this, "Root::close", __LINE__);
  if (fclose(yyin) != 0)
    throw _Error(this, "Root::close", __LINE__);
  state = S_CONSTRACT;
}

void				// print for debugging
Root::printQuery(ofstream * ofs, char * fname){
  try{
    *ofs << "<?xml version=\"1.0\" encoding=\"euc-jp\"?>" << endl << endl;
    *ofs << "<query fileName=\"" << fname << "\">" << endl;
    *ofs << "  <variables count=\"" << variables->size() << "\">" << endl;
    *ofs << "  </variables>" << endl;
    *ofs << "  <nodes count=\"" << elements->getCount() << "\">" << endl;
    for (unsigned long i = 0; i < elements->getCount(); i++){
      Node * n = elements->getItem(i);
      *ofs << "    <node id=\"" << i << "\" name=\"" << n->getNodeName() << "\"/>" << endl;
    }
    *ofs << "  </nodes>" << endl;
    *ofs << "  <structure>" << endl;
    Node * n = elements->top();
    if (n != 0) n->print(ofs);
    *ofs << "  </structure>" << endl;
    *ofs << "</query>" << endl;
  }
  catch (_Error & err){
    err.addItem(new ErrItem(this, "Root::print()", __LINE__));
    throw err;
  }
}

				// 1. Attribute uniqueness is checked by 
				//    Node::insert(CNodeDef *).
void				// 2. permit (A AS {a0}, A AS {a1}, B).
Root::checkTree(void){
  try{
    for (unsigned long i = 0; i < elements->getCount(); i++){
      Node * n = elements->getItem(i);
      n->check();
    }
  }
  catch (_Error & err){
    err.addItem(new ErrItem(this, "Root::checkTree()", __LINE__));
    throw err;
  }
}
