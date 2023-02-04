// -*- mode: c++ -*-
//  copyright (C) 2001 Makoto Onizuka, University of Washington
//  $Id: Node.cxx,v 1.1.1.1 2002/05/04 12:53:55 tjgreen Exp $

#define NODE_EMBODY
#define XML_INDENT 2
#ifndef WIN32
#include <iostream.h>
#else
#include <iostream>
#endif
#include "Node.h"
#include "Root.h"
#include "Error.h"
#include "List.cxx"

static int nest;

template class List<char>;
template class List<Node>;
template class List<CNodeDef>;

Node::Node(char * symName, nodeType t) : Base(), state(S_INACTIVE), parentNodes(new List<Node>), childNodes(new List<CNodeDef>){
  try{
    if (symName == NULL) throw _Error(this, "Node::Node()", __LINE__);
    name = strdup(symName);
    if (name == NULL) throw _Error(this, "Node::Node()", __LINE__);
    nType = t;
  }
  catch (_Error & err){
    err.addItem(new ErrItem(this, "Node::Node()", __LINE__));
    throw err;
  }
}

unsigned int			// insert child node.
Node::insert(CNodeDef * ndef){
  try {
    Node * newChild = ndef->node;
    if (newChild->nType == Attribute){ // Attribute uniqueness check
      for (unsigned int i = 0; i < childNodes->getCount(); i++){
	CNodeDef * cd = childNodes->getItem(i);
	Node * n = cd->node;
	if (n->nType == Attribute && strcmp(n->name, newChild->name)==0){
				// There is the same name Attribute
	  cerr << n->name << " conflict with existing attribute name." << endl;
	  throw _Error(this, "Node::insert()", __LINE__);
	}
      }
    }
    newChild->parentNodes->insertItem(this);
    return childNodes->insertItem(ndef);
  }
  catch (_Error & err){
    err.addItem(new ErrItem(this, "Node::insert()", __LINE__));
    throw err;
  }
}

void
Node::check(void){
  try {			// inifinit loop check.
    for (unsigned long i = 0; i < childNodes->getCount();i++){
      List<Node> * checked = new List<Node>(); // for recursive detection
      delete checked;
    }
    // Root candidate element must have XPath
    List<Node> * elements = new List<Node>();
    parentElements(elements);
				// If this element is root and fullXPath is null.
    if (elements->getCount()==0){
      cerr << "Node:: name=\"" << name << "\"" << endl;
      cerr << "Root candidate element or a top element under ANY must have XPath." << endl;
      throw _Error(this, "Node::check()", __LINE__);	
    }
    delete elements;
  }
  catch (_Error & err){
    err.addItem(new ErrItem(this, "Node::reorderChildNodes()", __LINE__));
    throw err;
  }
}

void				// move all attribute before any element.
Node::reorderChildNodes(List<Node> * checked){
  try {
    unsigned int top = 0;	// Top position of Attribute
    for (unsigned int i = 0; i < childNodes->getCount();i++){
      CNodeDef * cd = childNodes->getItem(i);
      if (cd->getNodeType() == Attribute){
	childNodes->swap(top++, i);
      }
    }
    checked->insertItem(this); // checked this Node.
				// reorderChildNodes of this childNodes
    for (unsigned int j = 0; j < childNodes->getCount();j++){
      CNodeDef * cd = childNodes->getItem(j);
      if (checked->member(cd->node)<0) // if not checked yet then check
	cd->node->reorderChildNodes(checked);
    }
  }
  catch (_Error & err){
    err.addItem(new ErrItem(this, "Node::reorderChildNodes()", __LINE__));
    throw err;
  }
}

void				// print for debug.
Node::print(ofstream * ofs){
  nest = 4;
  List<Node> * checked = new List<Node>(); // for recursive detection
  printSub(ofs, checked, NULL);
  delete checked;
}

void				// print for debug.
Node::printSub(ofstream * ofs, List<Node> * checked, CNodeDef * cd){
  if (checked->member(this)>=0){ // loop detected
    *ofs << "Element "<< getNodeName();
    *ofs << "[" << childNodes->getCount() << "]: loops." << endl;
    return;
  }
  checked->insertItem(this);
  indent(ofs);
  switch (nType){
  case Element:
    *ofs << "<node type=\"Element\" name=\""<< getNodeName() << "\"";
    break;
  case EmptyElement:
    *ofs << "<node type=\"emptyElement\" name=\""<< getNodeName() << "\"";
    break;
  case AnyElement:
    *ofs << "<node type=\"anyElement\" name=\""<< getNodeName() << "\"";
    break;
  case TextNode:
    *ofs << "<node type=\"TextNode\" name=\""<< getNodeName() << "\"";
    break;
  case Attribute:
    *ofs << "<node type=\"Attribute\" name=\""<< getNodeName() << "\"";
    break;
  case pseudoSeqElement:
    *ofs << "<node type=\"nestSequence\" name=\""<< getNodeName() << "\"";
    break;
  case pseudoSelElement:
    *ofs << "<node type=\"nestSelective\" name=\""<< getNodeName() << "\"";
    break;
  case pseudoOneElement:
    *ofs << "<node type=\"nest\" name=\""<< getNodeName() << "\"";
    break;
  }
  try {
    *ofs << " childCount=\"" << childNodes->getCount() << "\">" << endl;
    nest += XML_INDENT;
    if (cd != NULL) cd->printSkolemParams(ofs);
    for (unsigned int i = 0; i < childNodes->getCount();i++){
      CNodeDef * cd = childNodes->getItem(i);
      cd->node->printSub(ofs, checked, cd);
    }
    nest -= XML_INDENT;
    indent(ofs);
    *ofs << "</node>" << endl;
  }
  catch (_Error & err){
    err.addItem(new ErrItem(this, "Node::printSub()", __LINE__));
    throw err;
  }
}

void
Node::indent(ofstream * ofs){
  for (int i = 0; i < nest; i++) *ofs << " ";
}

void
Node::indent(void){
  for (int i = 0; i < nest; i++) cout << " ";
}

void				// This is a recursive function to get 
				// all reachable parent Element.
Node::parentElements(List<Node> * elements){
  try {
    for (unsigned long i = 0; i < parentNodes->getCount();i++){
      Node * n = parentNodes->getItem(i);
      switch (n->nType) {
      case Element:
      case EmptyElement:
      case AnyElement:
	if (elements->member(n)<0){
	  elements->insertItem(n);
	  n->parentElements(elements);
	}
	break;
      default:			// pseudo*, Attribute, Text
	n->parentElements(elements);
	break;
      }
    }
  }
  catch (_Error & err){
    err.addItem(new ErrItem(this, "Node::parentElements()", __LINE__));
    throw err;
  }
}

void
CNodeDef::indent(ofstream * ofs){
  for (int i = 0; i < nest; i++) *ofs << " ";
}

void
CNodeDef::indent(void){
  for (int i = 0; i < nest; i++) cout << " ";
}

void
CNodeDef::printSkolemParams(ofstream * ofs){
  try {
    if (skolemParams == NULL) return; // This is default action.
    indent(ofs);
    * ofs << "<skolem paramCount=\"" << skolemParams->getCount() << "\">" << endl;
    nest += XML_INDENT;
    for (unsigned int i = 0; i < skolemParams->getCount(); i++){
      XPath * v = skolemParams->getItem(i);
      indent(ofs);
      *ofs << "<param name=\"";
      v->print(ofs);
      *ofs << "\"/>" << endl;
    }
    nest -= XML_INDENT;
    indent(ofs);
    * ofs << "</skolem>" << endl;
  }
  catch (_Error & err){
    err.addItem(new ErrItem(this, "CNodeDef::printSkolemParams()", __LINE__));
    throw err;
  }
}
