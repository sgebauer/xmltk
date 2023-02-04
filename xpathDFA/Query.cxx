// -*- mode: c++ -*-
//  copyright (C) 2001 Makoto Onizuka, University of Washington
//  $Id: Query.cxx,v 1.3 2002/08/13 12:59:46 monizuka Exp $

#define QUERY_EMBODY
#ifndef WIN32
#include <iostream.h>
#else
#include <iostream>
#endif

#include "Query.h"
#include "Root.h"

Query::Query(void) : Base(),state(S_CONSTRUCT),type(T_STRING),query(0),top(0){
  try {
    parseTree = new Root();
	combinedAutomaton = 0;
	automata = 0;
  }
  catch (_Error & err){
    err.addItem(new ErrItem(this, "Query::Query()", __LINE__));
    throw err;
  }
}

Variable *
Query::parse(void) {
  if (state == S_START_DOC || state == S_END_DOC){
	cerr << errmes[E_STATE] << endl;
	throw _Error(this, "Query::parse()", __LINE__);
  }
  try {
	__gQuery = this;
	state = S_PARSED;
	return parseTree->parse();
  }
  catch (_Error & err){
	err.addItem(new ErrItem(this, "Query::parse()", __LINE__));
	throw err;
  }
}

void 
Query::xpath2NFA(void){
  try {
	VarPtrList * vs = parseTree->getVarPtrMap();
	automata  = new AutomataPtrList();
	for (VarPtrList::iterator itr = vs->begin(); itr != vs->end(); ++itr){
	  Automata * a = new Automata();
	  a->create((*itr)->getXPath(), automata);
	  automata->push_back(a);
	}
  }
  catch (_Error & err){
	err.addItem(new ErrItem(this, "Query::xpath2NFA()", __LINE__));
	throw err;
  }
}

void 
Query::NFAs2DFAs(void){
  try {
	for (AutomataPtrList::iterator itr = automata->begin(); itr != automata->end(); ++itr){
	  (*itr)->NFA2DFA();
	}
  }
  catch (_Error & err){
	err.addItem(new ErrItem(this, "Query::NFAs2DFAs()", __LINE__));
	throw err;
  }
}

//
// for SAX interfaces
//
Query::FAStates
Query::startDocument(void){
  if (state != S_PARSED){
	cerr << errmes[E_STATE] << endl;
	throw _Error(this, "Query::startDocument()", __LINE__);
  }
  state = S_START_DOC;
  try {
	switch (combinedAutomaton->startDocument()){
	case Automata::S_INACTIVE:
	  return S_INACTIVE;
	case Automata::S_COMPLETE:
	  return S_COMPLETE;
	default:					// case Automata::S_SKIP:
	  return S_SKIP;
	}
  }
  catch (_Error & err){
    err.addItem(new ErrItem(this, "Query::startDocument()", __LINE__));
    throw err;
  }
}

Query::FAStates
Query::endDocument(void){
  if (state != S_START_DOC){
	cerr << errmes[E_STATE] << endl;
	throw _Error(this, "Query::endDocument()", __LINE__);
  }
  state = S_END_DOC;
  try {
	switch (combinedAutomaton->endDocument()){
	case Automata::S_INACTIVE:
	  return S_INACTIVE;
	case Automata::S_COMPLETE:
	  return S_COMPLETE;
	default:					// case Automata::S_SKIP;
	  return S_SKIP;
	}
  }
  catch (_Error & err){
    err.addItem(new ErrItem(this, "Query::endDocument()", __LINE__));
    throw err;
  }
}

Query::FAStates
Query::startElement(PString * ps){
#if defined(DEBUG)
  char * debug = ps->getString(ps);
#endif
  try {
	switch (combinedAutomaton->startElement(ps)){
	case Automata::S_INACTIVE:
	  return S_INACTIVE;
	case Automata::S_COMPLETE:
	  return S_COMPLETE;
	default:					// case Automata::S_SKIP:
	  return S_SKIP;
	}
  }
  catch (_Error & err){
    err.addItem(new ErrItem(this, "Query::startElement()", __LINE__));
    throw err;
  }
}

Query::FAStates
Query::endElement(PString * ps){
#if defined(DEBUG)
  char * debug = ps->getString(ps);
#endif
  try {
	switch (combinedAutomaton->endElement()){
	case Automata::S_INACTIVE:
	  return S_INACTIVE;
	case Automata::S_COMPLETE:
	  return S_COMPLETE;
	default:		// case Automata::S_SKIP;
	  return S_SKIP;
	}
  }
  catch (_Error & err){
    err.addItem(new ErrItem(this, "Query::endElement()", __LINE__));
    throw err;
  }
}

Query::FAStates
Query::endElement(void){
  try {
	/*
	if (state != S_START_DOC){
	  cerr << errmes[E_STATE] << endl;
	  throw _Error(this, "Query::endElement()", __LINE__);
	}
	*/
	switch (combinedAutomaton->endElement()){
	case Automata::S_INACTIVE:
	  return S_INACTIVE;
	case Automata::S_COMPLETE:
	  return S_COMPLETE;
	default:		// case Automata::S_SKIP;
	  return S_SKIP;
	}
  }
  catch (_Error & err){
    err.addItem(new ErrItem(this, "Query::endElement()", __LINE__));
    throw err;
  }
}
