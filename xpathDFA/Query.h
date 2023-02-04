// -*- mode: c++ -*-
//  This is a Query module for xmatch processor.
//  copyright (C) 2001 Makoto Onizuka, University of Washington
//  $Id: Query.h,v 1.3 2002/08/13 12:59:46 monizuka Exp $

/***********************************************************
Query object offers an interface for dfafilter.cxx to process
a collection of XPath expressions. The processing works as follows.
- construct
- XPath expression registration
- create a parse tree for each XPath expression
- create a non-deteministic finite automaton (NFA) for each parse tree
- combine all NFAs to one large NFA
- powerset construction and create one large deteministic finite automaton (DFA)
- receive SAX events from XML parser and returns a status of the DFA
- destruct

We can switch the DFA construction either eager mode or lazy mode
in the Query::combineNFAs(). 
 **********************************************************/

#if ! defined(__QUERY_H__)
#define __QUERY_H__
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
#include "Root.h"
#include "Automata.h"

#define	PSTRING_STATIC_MEMORY	2 // this is only for the dfafilter.cxx, so
				  // new PString() is very restricted.

class Query;

class Query : public Base {
private:
  enum states {					// Query object state
	S_CONSTRUCT,
	S_PARSED,
	S_START_DOC,
	S_END_DOC
  };
  states state;					// to check an user invoke SAX event method
  enum types {
	T_FILE = 0,
	T_STRING,
  };
  types	type;			// query type
				// before invoking parse().
  Root * 		parseTree;
  Automata * 	combinedAutomaton; // combined automata
  AutomataPtrList *	automata;

  enum errcodes {
    OK = 0,
    E_ALLOC,
    E_GLOBAL_STATE,
    E_STATE,
    E_OTHERCLASS,
    E_NOT_SUPPORT,
    E_UNDER_IMPLEMENTING,
    E_INTERNAL			// system internal errror
  };
  static char * errmes[];
public:
  char * query;
  char * top;					// this is refered from xmath.y
  static const char * version;

public:
  char * memoryUsage(void);
  Query(void);					  // for xpath
  virtual ~Query() {
    if (query) free(query);
    delete parseTree;
    if (automata){
      for (AutomataPtrList::iterator itr = automata->begin(); itr != automata->end(); ++itr){
	delete (*itr);
      }
      delete automata;
    }
    if (combinedAutomaton) delete combinedAutomaton;
  }
  void	 registerQueryFile(const char * fname){
    delete parseTree;
    type = T_FILE;
    parseTree = new Root(fname);
  }
			// for child xpath (query is char *) under context
  Variable * registerQuery(Variable * parent, const char * const q, bool b){
    if (type == T_FILE)
      throw _Error(this, "Query::registerQuery()", __LINE__);
    if (query) free(query);
    query = top = strdup(q);
    Variable * v = parse();
    if (parent) v->setParent(parent);
    v->setOutputFlag(b);
    return v;
  }
			// for child xpath (query is char *) under context
  Variable * registerQuery(Variable * parent, const char * const q, bool b, float prec){
	if (type == T_FILE)
	  throw _Error(this, "Query::registerQuery()", __LINE__);
	if (query) free(query);
	query = top = strdup(q);
	Variable * v = parse();
	if (parent) v->setParent(parent);
	v->setOutputFlag(b, prec);
	return v;
  }
  void createAutomata(void){
    try {
      if (type == T_FILE) {
	parseTree->open();			
	parse();
      }
      xpath2NFA();
      /*
	NFAs2DFAs();		// two steps : NFAs -> DFAs
	combineDFAs();		//           : DFAs -> DFA
      */
      combineNFAs();		// one step  : NFA -> NFAs -> DFA
      NFA2DFA();		// DFA construction
      if (type == T_FILE) parseTree->close();
    }
    catch (_Error & err){
      err.addItem(new ErrItem(this, "Query::createAutomata()", __LINE__));
      throw err;
    }
  }
  VarPtrArray * getStateVariables(void){
    return combinedAutomaton->getStateVariables();
  }
  Root * getRoot(void) { return parseTree; }
  void insertVariable(Variable * vdef){			// for xpath
    parseTree->insertVariable(vdef);
  }

private:
  Variable * parse(void);
  void xpath2NFA(void);
  void NFAs2DFAs(void);
  void combineNFAs(void){
	try {
	  // combinedAutomaton = new Automata(); // static construction (eagerDFA)
	  combinedAutomaton = new LazyAutomata(); // dynamic construction (lazyDFA)
	  combinedAutomaton->combineNFAs(automata);
	}
	catch (_Error & err){
	  err.addItem(new ErrItem(this, "Query::combineNFAs()", __LINE__));
	  throw err;
	}
  }
  void combineDFAs(void){
	try {
	  combinedAutomaton = new Automata();
	  combinedAutomaton->combineDFAs(automata);
	}
	catch (_Error & err){
	  err.addItem(new ErrItem(this, "Query::combineDFAs()", __LINE__));
	  throw err;
	}
  }
  void NFA2DFA(void) { combinedAutomaton->NFA2DFA(); }

public:				// for debug mode
  void printAutomaton(ofstream* ofs, char * qfile){
	if (state == S_CONSTRUCT){
	  cerr << errmes[E_STATE] << endl;
      throw _Error(this, "Query::printAutomaton()", __LINE__);
	}
    try {
	  *ofs << "<?xml version=\"1.0\" encoding=\"euc-jp\"?>" << endl << endl;
	  *ofs << "<query queryFile=\"" << qfile << "\" memoryUsage=\"" << memoryUsage() << "\" xpathCount=\"" << automata->size() << "\">" << endl;
	  /*
		for (AutomataPtrList::iterator itr = automata->begin(); itr != automata->end(); ++itr){
		(*itr)->printAutomaton(ofs);
	  }
	  */
	  combinedAutomaton->printAutomaton(ofs);
	  *ofs << "</query>" << endl;
    }
    catch (_Error & err){
      err.addItem(new ErrItem(this, "Query::printAutomaton()", __LINE__));
      throw err;
    }
  }
  void printQuery(ofstream* of, char * fname){
	if (state == S_CONSTRUCT){
	  cerr << errmes[E_STATE] << endl;
      throw _Error(this, "Query::printQuery()", __LINE__);
	}
    try {
      parseTree->printQuery(of, fname);
    }
    catch (_Error & err){
      err.addItem(new ErrItem(this, "Query::printQuery()", __LINE__));
      throw err;
    }
  }

public:				// for SAX event
  enum FAStates {
    S_INACTIVE = 0,
    S_COMPLETE,
	S_SKIP
  };
  FAStates	startDocument(void);
  FAStates	endDocument(void);
  FAStates	startElement(PString * ps);
  FAStates	endElement(PString * ps);
  FAStates	endElement(void);
  VarPtrArray *	attribute(PString * ps, PString * val){
    return combinedAutomaton->attribute(ps, val);
  }
  VarPtrArray * characters(PString * val){
    return combinedAutomaton->characters(val);
  }
  void		endCharacters(void){return combinedAutomaton->endCharacters();}

public:				// FA library interface
  unsigned int getSucceedSkipDepth(void){ return combinedAutomaton->getSucceedSkipDepth(); }
  PString *    succeedSkip(void){ return combinedAutomaton->succeedSkip(); }
  unsigned int getFailSkipDepth(void){ return combinedAutomaton->getFailSkipDepth(); }
  unsigned int failSkip(void){ return combinedAutomaton->failSkip(); }
};

extern Query * __gQuery;
#if defined (QUERY_EMBODY)
Query * __gQuery;// for the my_yyinput(char * buf, int max_size) in xmatch.y
const char * Query::version = "XPath2DFA processor v1.0 August/7/2002";

char * Query::errmes[] = {
  "",
  "Memory allocation error",
  "Query global state error: users must not invoke another constructor before invoking the previous parse method",
  "Query state error: state must transit construct, parse, startDocument,...,endDocument",
  "Other class error",
  "Not supported",
  "This function will be supported soon",
  "System internal error"
};
#endif

#endif
