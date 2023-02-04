// -*- mode: c++ -*-
//  This is a Query module for xmatch processor.
//  copyright (C) 2001 Makoto Onizuka, University of Washington
//  $Id: Automata.h,v 1.4 2002/08/13 12:59:46 monizuka Exp $

#if ! defined(__AUTOMATA_H__)
#define __AUTOMATA_H__
#ifndef WIN32
#include <hash_map.h>
#include <list.h>
#else
#include <hash_map>
#include <list>
#endif
#include <string>

#include "Base.h"
#include "Error.h"
#include "List.h"
#include "XPath.h"
#include "Root.h"

using namespace std;

# define 	MAX_STACK_DEPTH	128

class Edge;
class State;
class DFAState;
class LazyDFAState;
class StackItem;
class Automata;
class LazyAutomata;

#if defined(TOKEN_PARSER)
typedef hash_map<XTOKEN, Edge> EdgeMap;
#else
typedef hash_map<char const *, Edge, HashCharPtr, EqualCharPtr> EdgeMap;
#endif
typedef list<Automata *> AutomataPtrList;
typedef	list<State *> StatePtrList;
typedef	list<DFAState *> DFAStatePtrList;
typedef list<Predicate *> PredPtrList;
typedef List<StackItem>	  StackItemArray;

class Edge : public Base {
  friend class State;
  friend class StackItem;
  friend class DFAState;
  friend class LazyDFAState;
  friend class Automata;
  friend class LazyAutomata;
  friend class treeAutomata;
protected:
  enum types {
    T_Element = 0,		// element edge
    T_Attribute,		// attribute edge
    T_Text,			// text edge
    T_Epsilon,			// epsilon edge
    T_AnyElement,		// anyElement edge created from '*'
    T_AnyAttribute,		// anyAttribute edge
    T_DocumentRoot		// Document root edge
  };

  State		* from;		// static: from State
  State 	* to;		// static: to State
  LocationStep	* sp;		// static: step including its predicate
				// if sp == 0, it is for double slash
  PredPtrList   * preds;	// static: all predicates for this Edge
				//         for predicate check/increment
  types		type;

private:
  enum errcodes {
    OK = 0,
    E_ALLOC,
    E_OTHERCLASS,
    E_NOT_SUPPORT,
    E_UNDER_IMPLEMENTING,
    E_INTERNAL			// system internal error
  };
  static char * errmes[];

public:
  Edge(void):Base(),from(0),to(0),sp(0)	// for DFAState::createEdge
	     ,preds(0)
  {}
  Edge(State * s, State * next, LocationStep * p):Base(),from(s),to(next),sp(p)
	     ,preds(0)
  {
    if (!sp) type = T_AnyElement; // AnyLoopElement
    else {
      switch (sp->getType()){
      case LocationStep::Element:
	type = T_Element;
	break;
      case LocationStep::Attribute:
	type = T_Attribute;
	break;
      case LocationStep::Text:
	type = T_Text;
	break;
      case LocationStep::AnyElement:
	type = T_AnyElement;
	break;
      case LocationStep::AnyAttribute:
	type = T_AnyAttribute;
	break;
      case LocationStep::Period:
	type = T_Epsilon;
	break;
      case LocationStep::DocumentRoot:
	type = T_DocumentRoot;
	break;
      default:			// DoubleSlash (from //e)
	cerr << errmes[E_INTERNAL] << endl;
	throw _Error((Base *)this, "Edge::Edge()", __LINE__);
      }	// end switch
    }
  }
  // for T_Epsilon, T_AnyElement(for //), 
  // and for T_AnyAttribute (for every non-terminal state)
  Edge(State * s, State * next, types t):Base(),from(s),to(next),sp(0)
				,preds(0)
				,type(t){}
  virtual ~Edge() {
    if (preds) delete preds;
  }

  inline bool
  operator <(const Edge & e) const{
    if (type != e.type) return type < e.type; // ANY < ELEMENT
    else {
      if (type == T_Element || type == T_Attribute)
	return *(sp) < *(e.sp);
      else return false;
    }
  }
  inline bool
  operator ==(const Edge & e) const{ 
    if (type != e.type) return false;
    else {
      if (type == T_Element || type == T_Attribute)
	return *(sp) == *(e.sp);
      else return true;
    }
  }

protected:
  void 	resetPredicate(Automata * a);
  State * getToState(void) { return to; }
  bool	contain(const Edge * e) const;
  // this contains e like "*" contains "a",
  // "a" contains "a" or "a" contains "a[1]"
public:
  types	getType(void) const { return type; }
  void	setPredicates(PredPtrList * prs){ preds = prs; }
  char * getLabel(void) const{
    switch(type){
    case T_Element:
    case T_Attribute:
    case T_Text:
    case T_DocumentRoot:
      return sp->getLabel();
    case T_AnyElement:
      return "*";
    case T_AnyAttribute:
      return "@*";
    case T_Epsilon:
      return "eplison";
    }
    return "";
  }
};

class StatePtrArray: public List<State> {
private:
  unsigned int signature;
public:
  StatePtrArray(unsigned int s): List<State>(s),signature(0) {}
  ~StatePtrArray() {}
  inline bool
  operator ==(StatePtrArray & ss) {
    if (List<State>::count != ss.count) return false;
    if (signature != ss.signature) return false;
    if (List<State>::count){
      for (unsigned int i = List<State>::count - 1;; i--){
	if (List<State>::list[i] != ss.list[i]) {
	  return false;
	}
	if (i == 0) break;
      }
    }
    return true;
  }
  void sortDistinct(void){
    signature = List<State>::sortDistinct();
  }
  unsigned long  insert(const State * s){
    signature ^= (unsigned int)s;
    return List<State>::insertItem(s);
  }
};

typedef	list<Edge> EdgeList;	// for NFA Edges and transitiveEdges
//typedef	hash_set<State *, HashStatePtr, EqualStatePtr> StatePtrArray;
//typedef	list<State *> 	StatePtrArray;// for DFAs
//typedef	set<State *> 	StatePtrArray;// for DFA

class State : public Base {
  friend class Edge;
  friend class StackItem;
  friend class DFAState;
  friend class LazyDFAState;
  friend class Automata;
  friend class treeAutomata;
private:
  enum types {
    T_TERMINAL = 0,
    T_OTHER
  };
  EdgeList 	* edges;	// static: going out edges (OR standard form)
protected:
  types		  type;		// static: terminal type (T_TERMINAL, T_OTHER)
  unsigned short sinkDepth;	// dynamic: instead of using "other edge"
  unsigned short stayDepth;	// dynamic: the number staying the same state
private:
  Variable	* var;		// static: NFA this State is derived from var

  enum errcodes {
    OK = 0,
    E_ALLOC,
    E_OTHERCLASS,
    E_NOT_SUPPORT,
    E_UNDER_IMPLEMENTING,
    E_INTERNAL			// system internal errror
  };
  static char * errmes[];

public:
  State(void) : Base(),edges(0),type(T_OTHER),sinkDepth(0),stayDepth(0),var(0){
#if defined(XMLTK_DEBUG)
    cout << "State() address = " << this << endl;
#endif
  }
  virtual ~State() {
    try {
      if (edges) delete edges;
    }
    catch (_Error & err){
      err.addItem(new ErrItem((Base *)this, "State::~State()", __LINE__));
      throw err;
    }
  }
  virtual VarPtrArray * getStateVariables(void) { return 0; }
protected:
  virtual void insertStateVariable(Variable * v){}
  void 	setTerminal(void) { type = T_TERMINAL; }
  virtual void	addTransitiveStates(const Edge * e, StatePtrArray * ss,
				    PredPtrList * pl) const;
  virtual void	addTransitiveStatesCount(const Edge * e, unsigned int & c) const;

  virtual void	addTransitiveEdges(EdgeList * el, PredPtrList * pl) const;

private:
  State * createEdge(LocationStep * sp){
    try {
      if (edges == 0) edges = new EdgeList();
      State * next = new State();
      switch(sp->getType()){
      case LocationStep::Element:
      case LocationStep::Attribute:
      case LocationStep::Text:
      case LocationStep::AnyElement:
				// add @* edge to all states
	edges->push_back(Edge(this, this, Edge::T_AnyAttribute));
	edges->push_back(Edge(this, next, sp));
	break;
      case LocationStep::AnyAttribute:
      case LocationStep::Period:
	edges->push_back(Edge(this, next, sp));
	break;
      case LocationStep::DoubleSlash:
      case LocationStep::DocumentRoot:
	cerr << errmes[E_INTERNAL] << endl;
	throw _Error((Base *)this, "State::createEdge()", __LINE__);
      default:// DoublePeriod, AltElement
	cerr << errmes[E_NOT_SUPPORT] << endl;
	throw _Error((Base *)this, "State::createEdge()", __LINE__);
      }
      return next;
    }
    catch (_Error & err){
      err.addItem(new ErrItem((Base *)this, "State::createEdge()", __LINE__));
      throw err;
    }
  }
  void	createEdge(State * to, Edge::types t){
    try {
      if (edges == 0) edges = new EdgeList();
      edges->push_back(Edge(this, to, t));
      return;
    }
    catch (_Error & err){
      err.addItem(new ErrItem((Base *)this, "State::createEdge()", __LINE__));
      throw err;
    }
  }

protected:				// for debug
  virtual void print(ofstream* of);

protected:				// for SAX event
  virtual Edge * startElement(PString * ps) { return 0; }
  bool	endElement(void);
  virtual Edge * attribute(PString * ps, PString * val) { return 0; }
  virtual Edge * characters(PString * ps) { return 0; }

public:
  virtual PredPtrList * getIndexPreds(void) { return 0; }
protected:				// DFA library interface
  virtual bool 	isTerminal(void) const {
    if (type == T_TERMINAL && sinkDepth == 0) return true;
    else return false;
  }
  unsigned int skipDepth(void){ return sinkDepth; }
  unsigned int skip(void){		// skip & return the skip element level
    unsigned int skipDepth = sinkDepth;
    sinkDepth = 0;
    return skipDepth;
  }
  virtual void resetAccessCount(Automata * a){ return; }
  virtual bool isSatisfiable(void){ return false; }
};

class DFAState : public State {
  friend class Automata;
  friend class treeAutomata;
  friend class LazyAutomata;
protected:
  // static: going out edges (OR standard form)
  Automata* parent;
  EdgeMap * edges;		// pair (path, Edge)
  EdgeMap * textEdges;		// pair (text value, Edge)
  Edge	  * anyAttributeEdge;	// static: anyAttribute edge
  Edge	  * anyElementEdge;	// static: this is for anyElement edge
  Edge	  * textEdge;		// static: this is for anyElement edge
  StatePtrArray 	* states; // static: a collection of NFA states
  VarPtrArray  * vars;		// static: this State is terminal for vars
  unsigned int	stateCount;	// static: number of NFA state count
  PredPtrList * preds;		// static: index predicates for all Edge
				//         is set at getUniqEdges
				//         for StackItem push/pop operations
private:
  enum errcodes {
    OK = 0,
    E_ALLOC,
    E_OTHERCLASS,
    E_NOT_SUPPORT,
    E_UNDER_IMPLEMENTING,
    E_INTERNAL			// system internal errror
  };
  static char * errmes[];

public:
  DFAState(Automata * a, StatePtrArray* ss): State(),parent(a),edges(0),textEdges(0),anyAttributeEdge(0),anyElementEdge(0),textEdge(0),states(ss)
			       ,preds(0)
  {
    //	edges and textEdges is constructed in DFAState::getUniqEdges()
    stateCount = ss->getCount();
    unsigned int vc = 0;
    float	minPrec = 0.0;
    for (unsigned int i = 0; i<stateCount; i++){
      State * s = ss->getItem(i);
      if (s->isTerminal()){
	if (vc == 0 || minPrec > s->var->getPrecedence())
	  minPrec = s->var->getPrecedence();
	vc++;
      }
    }
    vars = new VarPtrArray(vc);
    for (unsigned int j = 0; j<stateCount; j++){
      State * s = ss->getItem(j);
      if (s->isTerminal() && s->var->getPrecedence() == minPrec){
	vars->insertItem(s->var);
	if (type != T_TERMINAL) type = T_TERMINAL;
      }
    }
  }
  ~DFAState() {
    if (edges) delete edges;
    if (textEdges) delete textEdges;
    if (anyAttributeEdge) delete anyAttributeEdge;
    if (anyElementEdge) delete anyElementEdge;
    if (textEdge) delete textEdge;
    delete states;
    delete vars;
    if (preds) delete preds;
  }
  VarPtrArray * getStateVariables(void) { return vars; }
  void	addTransitiveEdges(EdgeList * el, PredPtrList * pl) const;
  void	deleteEdges(void);

  // SAX interface
  virtual Edge * startElement(PString * ps);
  virtual Edge * attribute(PString * ps, PString * val);
  virtual Edge * characters(PString * val);

				// DFA library interface
public:
  PredPtrList * getIndexPreds(void) { return preds; }
protected:
  bool 	isTerminal(void) const { // for attribute [@atr...][n] expression
    if (State::isTerminal()){
      unsigned int c = vars->getCount();
      for (unsigned int i = 0; i<c;i++){
	Variable * v = vars->getItem(i);
	if (v->enableFlag) return true;
      }
    }
    return false;
  }
  void 	resetAccessCount(Automata * a){
    if (!preds) return;
    try {
      for (PredPtrList::iterator itr = preds->begin(); itr != preds->end(); ++itr){
	(*itr)->resetAccessCount(a);
      }
    }
    catch (_Error & err){
      err.addItem(new ErrItem(this,"DFAState::resetAccessCount()",__LINE__));
      throw err;
    }
  }
  bool isSatisfiable(void){		// for skip (for author[1])
    try {
      if (preds == 0 || preds->size() == 0) return false; // no edges
      for (PredPtrList::iterator itr = preds->begin(); itr != preds->end(); ++itr){
	if ((*itr)->isSatisfiable()== false) return false;
      }
      return true;
    }
    catch (_Error & err){
      err.addItem(new ErrItem(this,"DFAState::isSatisfiable()",__LINE__));
      throw err;
    }
  }
public:
  EdgeMap * getEdges(void){ return edges; }

public:				// for debug
  void print(ofstream* of);

protected:			// for NFAs -> DFAs -> DFA operations
  void getUniqEdges(EdgeList *& el);
  PredPtrList * getTransitiveStates(const Edge * e, StatePtrArray * ss) const;
  void		addTransitiveStates(const Edge * e, StatePtrArray * ss,
				    PredPtrList * pl) const;
  unsigned int getTransitiveStatesCount(const Edge * e) const;
  void		addTransitiveStatesCount(const Edge * e, unsigned int & c) const;
  void 		createEdge(DFAState * next, const Edge * e);
};


class StackItem {
  friend class Automata;
  friend class treeAutomata;
private:
  Edge	*	edge;
  PString 	label;		// startElement tag label
  List<unsigned int>* edgeTable;// the same order as State::edges
				// this is for a case there is a cycle in DFA
				// (same state in different transition)
public:
  unsigned int	stayDepth;

public:
  StackItem(void):edge(0),edgeTable(0){} // for memoryPool
  StackItem(Edge * e, PString * ps);
  ~StackItem(){
    if (edgeTable){
      unsigned int c = edgeTable->getCount();
      for (unsigned int i = 0; i<c;i++){
	unsigned int * item = edgeTable->getItem(i);
	free(item);
      }
      delete edgeTable;
      edgeTable = 0;		// to avoid the second destruction,
				// because it is statically allocated
    }
  }
  void * operator new(size_t);
  void 	 operator delete(void *);

  Edge * getPopEdge(void) {
    PredPtrList * preds = edge->to->getIndexPreds();
    unsigned int i = 0;
    for (PredPtrList::iterator itr = preds->begin(); itr != preds->end(); ++itr){
      (*itr)->setAccessCount(* (edgeTable->getItem(i++)));
    }
    edge->to->stayDepth = stayDepth;
    return edge;
  }
};


class Automata : public Base {
  friend class treeAutomata;
public:
  enum types {
    T_NFA = 0,			// construct
    T_DFA,			// NFA2DFA
    T_PROCESSING		// under processing XML input
  };
protected:
  types		  type;
  Edge		  * current;	// dynamic: for push operation
  Edge		  * prev;	// dynamic: for pop operation
  StatePtrList	  * states;	// static: all states in the automata
  DFAStatePtrList * dfastates;	// static: all DFA states in the automata
  StackItemArray  * estack;	// dynamic: execution stack
  XPath		  * xpath;	// static: xpath for the Automata
  enum errcodes {
    OK = 0,
    E_ALLOC,
    E_OTHERCLASS,
    E_LEAF_PATH_SYNTAX,
    E_PREDICATE_SYNTAX,
    E_POSITION_PREDICATE,
    E_PERIOD_NOT_SUPPORT,
    E_NOT_SUPPORT,
    E_CHOICE_UNDER_IMPLEMENTING,
    E_UNDER_IMPLEMENTING,
    E_EDGE_STACK_NOT_EMPTY,
    E_INTERNAL			// system internal errror
  };
  static char * errmes[];

public:
  Automata(void): Base(),type(T_NFA),prev(0),xpath(0){
    try {
      states = new StatePtrList();
      dfastates = new DFAStatePtrList();
      State * currentState = new State();
      states->push_back(currentState);
      current = new Edge(0,currentState,Edge::T_DocumentRoot);
      estack = new StackItemArray(MAX_STACK_DEPTH);
    }
    catch (_Error & err){
      err.addItem(new ErrItem(this, "Automata::Autoata()", __LINE__));
      throw err;
    }
  }
  virtual ~Automata() {
    try {
      for (StatePtrList::iterator itr = states->begin(); itr != states->end(); ++itr){
	delete *itr;
      }
      delete states;
      for (DFAStatePtrList::iterator itr2 = dfastates->begin(); itr2 != dfastates->end(); ++itr2){
	delete *itr2;
      }
      delete dfastates;
      if (estack->getCount()>0){
	cerr << errmes[E_EDGE_STACK_NOT_EMPTY] << endl;
	throw _Error(this, "State::State()", __LINE__);
      }
      delete estack;
    }
    catch (_Error & err){
      err.addItem(new ErrItem(this, "Automata::~Automata()", __LINE__));
      throw err;
    }
  }
  // convert the xpath to a NFA
  void	create(XPath * path, AutomataPtrList * as);
  void	combineNFAs(AutomataPtrList * as);
  void	combineDFAs(AutomataPtrList * as);
  virtual void	NFA2DFA(void);	// static powerset construct
  VarPtrArray * getStateVariables(void) {
    if (prev) return ((DFAState *)prev->to)->getStateVariables();
    else return ((DFAState *)current->to)->getStateVariables();
  }
  virtual DFAState * registerDFAState(StatePtrArray * ss){
    try {
      ss->sortDistinct();
      for (DFAStatePtrList::iterator itr = dfastates->begin(); itr != dfastates->end(); ++itr){
	if ( (*itr)->states && *((*itr)->states) == *ss){
	  delete ss;
	  return *itr;
	}
      }
      DFAState * dfas = new DFAState(this,ss);
      dfastates->push_back(dfas);
      return dfas;
    }
    catch (_Error & err){
      err.addItem(new ErrItem(this, "Automata::registerDFAState()", __LINE__));
      throw err;
    }
  }

protected:
  State * createSub(XPath * altPath);
  void	createDFAStates(DFAState * dfas);
  Automata * getParent(void){
    Variable * v = xpath->getVariable();
    Variable * p = v->parent;
    if (p == 0) return 0;
    XPath * path = p->getXPath();
    return path->getAutomata();
  }
  bool	isTerminal(void){	// only for endElement()
    if (current->to->stayDepth>0 || current->to->sinkDepth>0)
      return false;
    prev = current;
    if (prev->getType() == Edge::T_Element ||
	prev->getType() == Edge::T_AnyElement||
	prev->getType() == Edge::T_DocumentRoot){
      return prev->to->isTerminal();      
    }
    unsigned int i = estack->getCount()-1;
    while(1){
      prev = estack->getItem(i)->edge;
      if (prev->to->stayDepth>0 || prev->to->sinkDepth>0)
	return false;
      if (prev->getType() == Edge::T_Element ||
	  prev->getType() == Edge::T_AnyElement||
	  prev->getType() == Edge::T_DocumentRoot) break;
      if (i--==0){
	cerr << errmes[E_INTERNAL] << endl;
	throw _Error(this, "Automata::isTerminal()", __LINE__);
	break;
      }
    }
    return prev->to->isTerminal();
  }

public:				// for debug
  void  printAutomaton(ofstream* of);
protected:
  //  void  depthCheck(void);

public:							// for SAX event
  enum FAStates {
    S_INACTIVE = 0,
    S_COMPLETE,
    S_SKIP						// no transition (including after COMPLETE)
  };
  FAStates	startDocument(void);
  FAStates	endDocument(void);
  FAStates	startElement(PString * ps);
  FAStates	endElement(void);
  VarPtrArray *	attribute(PString * ps, PString * val);
  VarPtrArray * characters(PString * ps);
  void   	endCharacters(void){ // reset the Variables enableFlags
    current->resetPredicate(this);
    StackItem * si = estack->pop();
    current = si->getPopEdge();
    delete si;
  }

public:				// FA library interface
  unsigned int getStackDepth(void) { return estack->getCount(); }
  unsigned int getFailSkipDepth(void){ return current->to->skipDepth(); }
  unsigned int failSkip(void){ return current->to->skip(); } // 1step
  unsigned int getSucceedSkipDepth(void){
    unsigned int depth = 1;
    unsigned int c = estack->getCount();
    if (c <= 1) return depth;	// for like "/bib[1]"
    for (unsigned int i = c - 1;;i--){
      StackItem * si = estack->getItem(i);
      if (si->edge->from->isSatisfiable()) depth++;
      if (i == 1) break;
    }
    return depth;
  }
  PString * succeedSkip(void){	// application skip (1step)
    endElement();
    StackItem * si = estack->last();
    return &(si->label);
  }
};

class LazyDFAState : public DFAState {
  friend class Automata;
  friend class treeAutomata;
private:
  enum errcodes {
    OK = 0,
    E_ALLOC,
    E_OTHERCLASS,
    E_NOT_SUPPORT,
    E_UNDER_IMPLEMENTING,
    E_INTERNAL			// system internal errror
  };
  static char * errmes[];

public:
  LazyDFAState(Automata * a, StatePtrArray* ss): DFAState(a,ss) {}
  ~LazyDFAState() {}
  Edge 		* startElement(PString * ps);
  Edge	 	* attribute(PString * ps, PString * val);
  Edge	 	* characters(PString * ps);
  DFAState 	* registerDFAState(StatePtrArray * ss);
};


class LazyAutomata : public Automata {
private:
  enum errcodes {
    OK = 0,
    E_ALLOC,
    E_OTHERCLASS,
    E_NOT_SUPPORT,
    E_UNDER_IMPLEMENTING,
    E_INTERNAL			// system internal errror
  };
  static char * errmes[];

public:
  LazyAutomata(void): Automata(){}
  virtual ~LazyAutomata() {}
  void	NFA2DFA(void);			// dynamic DFA construction
  DFAState * registerDFAState(StatePtrArray * ss){
    try {
      ss->sortDistinct();
      for (DFAStatePtrList::iterator itr = dfastates->begin(); itr != dfastates->end(); ++itr){
	if (*((*itr)->states) == *ss){
	  delete ss;
	  return *itr;
	}
      }
      LazyDFAState * dfas = new LazyDFAState(this,ss);
      dfastates->push_back(dfas);
      return dfas;
    }
    catch (_Error & err){
      err.addItem(new ErrItem(this, "LazyAutomata::registerDFAState()", __LINE__));
      throw err;
    }
  }
};


#if defined (AUTOMATA_EMBODY)
char * Automata::errmes[] = {
  "",
  "Memory allocation error",
  "Other class error",
  "Syntax error(not support): Any attribute or text() must be the last locationstep",
  "Syntax error(not support): A tail location step must not have any predicate.",
  "Syntax error(not supoort): The position predicate needs to follow an Element (not predicate, n.g /bib/book[@year][2])",
  "Syntax error(\".\" not support):",
  "Not supported",
  "\"|\" will be supported, but not yet",
  "This function will be supported soon",
  "System Internal error (The edge stack (estack) is not empty)",
  "System internal error"
};
char * Edge::errmes[] = {
  "",
  "Memory allocation error",
  "Other class error",
  "Not supported",
  "This function will be supported soon",
  "System internal error"
};
char * State::errmes[] = {
  "",
  "Memory allocation error",
  "Other class error",
  "Not supported",
  "This function will be supported soon",
  "System internal error"
};
char * DFAState::errmes[] = {
  "",
  "Memory allocation error",
  "Other class error",
  "Not supported",
  "This function will be supported soon",
  "System internal error"
};
#endif

#endif
