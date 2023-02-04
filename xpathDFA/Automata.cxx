// -*- mode: c++ -*-
//  copyright (C) 2001 Makoto Onizuka, University of Washington
//  $Id: Automata.cxx,v 1.3 2002/08/13 12:59:46 monizuka Exp $

#define AUTOMATA_EMBODY

#include "Automata.h"
#include "List.cxx"

template class List<StackItem>;
template class List<unsigned int>;

void
Edge::resetPredicate(Automata * a){
  if (!preds) return;
  try {
    for (PredPtrList::iterator itr = preds->begin(); itr != preds->end(); ++itr){
      (*itr)->reset(a);
    }
    to->resetAccessCount(a);
  }
  catch (_Error & err){
    err.addItem(new ErrItem(this,"Edge::resetPredicate()",__LINE__));
    throw err;
  }
}

bool
Edge::contain(const Edge * e) const{ // for NFA State
  // this contains e like "*" contains "a"
  // "a" contains "a" or "a" contains "a[1]"
  if (e == 0) return false;
  switch(type){
  case T_Element:
    if ( e->type == T_Element &&
	 *sp == *(e->sp)) return true;
    else return false;
  case T_Attribute:
    if ( e->type == T_Attribute &&
	 *sp == *(e->sp)) return true;
    else return false;
  case T_Text:
    if ( e->type == T_Text )
      return true;
    else return false;
  case T_AnyElement:
    if ( e->type == T_Element ||
	 e->type == T_AnyElement )
      return true;
    else return false;
  case T_AnyAttribute:
    if ( e->type == T_Attribute || e->type == T_AnyAttribute )
      return true;
    else return false;
  default:			// case Epsilon
    cerr << errmes[E_NOT_SUPPORT] << endl;
    throw _Error((Base *)this, "Edge::contain()", __LINE__);
  }
}

// collect all states that can be reached by Edge e in one transit

StackItem::StackItem(Edge * e, PString * ps) : edge(e),label(*ps),edgeTable(0),stayDepth(0){
  if (e->to){
    stayDepth = e->to->stayDepth;
    e->to->stayDepth = 0;	// initialize
    PredPtrList * preds = e->to->getIndexPreds();
    if (preds){
      edgeTable = new List<unsigned int>(preds->size());
      for (PredPtrList::iterator itr = preds->begin(); itr != preds->end(); ++itr){
			// stack counter only for T_INDEX
	if ((*itr)->getType()==Predicate::T_INDEX){
	  AccessCount * item = (AccessCount *)malloc(sizeof(AccessCount));
	  * item = (*itr)->getAccessCount();
	  edgeTable->insertItem(item);
	}
      }
    }
  }
}

// for precise comparison with Mike
static StackItem	memoryPool[MAX_STACK_DEPTH];
static unsigned int	memoryCount = 0;

void *
StackItem::operator new (size_t s){
  if (memoryCount < MAX_STACK_DEPTH) return &memoryPool[memoryCount++];
  else {
    memoryCount++;
    return malloc(sizeof (StackItem));
  }
}

void
StackItem::operator delete (void * p){
  if (memoryCount > MAX_STACK_DEPTH) free(p);
  memoryCount--;
}

bool
State::endElement(void){
  try {
    if (sinkDepth>0){
      sinkDepth--;
      return false;
    }
    return true;
  }
  catch (_Error & err){
    err.addItem(new ErrItem((Base *)this, "State::endElement()", __LINE__));
    throw err;
  }
}

// collect all edges that can be reached by Edge e in one transit
void
State::addTransitiveStates(const Edge * e,
			   StatePtrArray * ss,
			   PredPtrList * pl) const{
  try {
    if (edges == 0) return;
    if (e == 0){		// means epsilon edge
      for (EdgeList::iterator itr = edges->begin();itr != edges->end();++itr){
	if (itr->type == Edge::T_Epsilon){
	  ss->insert(itr->to);
	  itr->to->addTransitiveStates(e,ss,pl);
	}
      }
    }
    else {
      for (EdgeList::iterator itr = edges->begin();itr != edges->end();++itr){
	if (itr->type == Edge::T_Epsilon){
	  itr->to->addTransitiveStates(e,ss,pl);
	}
	else if (itr->contain(e)){
	  /*
	  cout << "  itr contains e: " << itr->getLabel() << ", "
	       << e->getLabel() << endl;
	  cout << "  to->var = " << itr->to->var->varName << endl;
	  */
	  ss->insert(itr->to);
	  if (itr->sp){		// egde is not Epsilon nor AnyElement (//)
	    List<Predicate> * pl2 = itr->sp->getPredicates();
	    for (unsigned int i = 0; i < pl2->getCount(); i++){
	      pl->push_back(pl2->getItem(i));
	    }
	  }
	  itr->to->addTransitiveStates(0,ss,pl); // epsilon transition
	}
      }
    }
  }
  catch (_Error & err){
    err.addItem(new ErrItem((Base *)this, "State::addTransitiveStates()", __LINE__));
    throw err;
  }
}

void
State::addTransitiveStatesCount(const Edge * e, unsigned int & c) const{
  try {
    if (edges == 0) return;
    if (e == 0){
      for (EdgeList::iterator itr = edges->begin(); itr != edges->end(); ++itr){
	if (itr->type == Edge::T_Epsilon){
	  c+=1;
	  itr->to->addTransitiveStatesCount(e, c);
	}
      }
    }
    else {
      for (EdgeList::iterator itr = edges->begin(); itr != edges->end(); ++itr){
	if (itr->type == Edge::T_Epsilon){
	  itr->to->addTransitiveStatesCount(e, c);
	}
	else if (itr->contain(e)){
	  c+=1;
	  itr->to->addTransitiveStatesCount(0, c);	// epsilon transition
	}
      }
    }
  }
  catch (_Error & err){
    err.addItem(new ErrItem((Base *)this, "State::addTransitiveStates()", __LINE__));
    throw err;
  }
}

//
// collect all edges that can be reached by one transit 
// (attribute, element, or text)
//
void
State::addTransitiveEdges(EdgeList * el, PredPtrList * pl) const{
  try {
    if (edges == 0) return;
    for (EdgeList::iterator itr = edges->begin(); itr != edges->end(); ++itr){
      if (itr->type == Edge::T_Epsilon) itr->to->addTransitiveEdges(el,pl);
      else {
	el->push_back(*itr);
	if (itr->sp){		// egde is not Epsilon nor AnyElement (//)
	  Predicate * p = itr->sp->getIndexPredicate();
	  if (p) pl->push_back(p);
	}
      }
    }
  }
  catch (_Error & err){
    err.addItem(new ErrItem((Base *)this, "State::addTransitiveEdges()", __LINE__));
    throw err;
  }
}

//
// get all edges that can be reached by one transit (attribute or element)
// from the current dfa state (from all states).
//
void
DFAState::getUniqEdges(EdgeList *& el){
  try {
    preds = new PredPtrList();
    el = new EdgeList();
    for (unsigned int i = 0; i<stateCount;i++){
      State * s = states->getItem(i);
      s->addTransitiveEdges(el,preds);
    }
    el->sort();
    el->unique();
    edges = new EdgeMap(el->size());
  }
  catch (_Error & err){
    err.addItem(new ErrItem((Base *)this, "DFAState::getUniqEdges()", __LINE__));
    throw err;
  }
}


//
// collect all edges that can be reached by Edge e in one transit
// (element or attribute)
//
void
DFAState::addTransitiveStates(const Edge * e,
			      StatePtrArray * ss,
			      PredPtrList * pl) const{
  try {
    if (edges == 0) return;
    for (EdgeMap::iterator itr = edges->begin(); itr != edges->end(); ++itr){
      Edge * e2 = &(itr->second);
      if (e2->type == Edge::T_Epsilon){
	if (e == 0) ss->insert(e2->to);
	e2->to->addTransitiveStates(e,ss,pl);
      }
      else if (e2->contain(e)){
	ss->insert(e2->to);
	e2->to->addTransitiveStates(0,ss,pl);	// epsilon transition
      }
    }
    if (anyAttributeEdge && anyAttributeEdge->contain(e)){
      ss->insert(anyAttributeEdge->to);
      // no epsilon transition
    }
    if (anyElementEdge){
      ss->insert(anyElementEdge->to);
      anyElementEdge->to->addTransitiveStates(0,ss,pl); // epsilon transition
      // This is for a nested expreesion $a = /a, $b = $a/b
    }
  }
  catch (_Error & err){
    err.addItem(new ErrItem((Base *)this, "DFAState::addTransitiveStates()", __LINE__));
    throw err;
  }
}


void
DFAState::addTransitiveStatesCount(const Edge * e, unsigned int & c) const{
  try {
    if (edges == 0) return;
    for (EdgeMap::iterator itr = edges->begin(); itr != edges->end(); ++itr){
      Edge * e2 = &(itr->second);
      if (e2->type == Edge::T_Epsilon){
	if (e == 0) c+=1;
	e2->to->addTransitiveStatesCount(e, c);
      }
      else if (e2->contain(e)){
	c+=1;
	e2->to->addTransitiveStatesCount(0, c);	// epsilon transition
      }
    }
    if (anyAttributeEdge && anyAttributeEdge->contain(e)){
      c+=1;
    }
    if (anyElementEdge){
      c+=1;
      anyElementEdge->to->addTransitiveStatesCount(0, c);
		// epsilon transition
		// This is for a nested expreesion $a = /a, $b = $a/b
    }
  }
  catch (_Error & err){
    err.addItem(new ErrItem((Base *)this, "DFAState::addTransitiveStatesCount()", __LINE__));
    throw err;
  }
}

//
// collect all edges that can be reached by one transit
// This is only for NFAs -> DFAs -> DFA construction
//
void
DFAState::addTransitiveEdges(EdgeList * el, PredPtrList * pl) const{
  try {
    if (anyElementEdge) {
      el->push_back(*anyElementEdge);
      Predicate * p = anyElementEdge->sp->getIndexPredicate();
      if (p) pl->push_back(p);
    }
    if (anyAttributeEdge){
      el->push_back(*anyAttributeEdge);
      Predicate * p = anyAttributeEdge->sp->getIndexPredicate();
      if (p) pl->push_back(p);
    }
    if (edges){
      for (EdgeMap::iterator itr = edges->begin();
	   itr != edges->end(); ++itr){
	Edge * e = &(itr->second);
	el->push_back(*e);
	Predicate * p = itr->second.sp->getIndexPredicate();
	if (p && p->getType() != Predicate::T_SIMPLE_PREDICATE) pl->push_back(p);
      }
    }
  }
  catch (_Error & err){
    err.addItem(new ErrItem((Base *)this, "DFAState::addTransitiveEdges()", __LINE__));
    throw err;
  }
}

// get all states that can be reached by the specified Edge "e" 
// from the current DFA state
PredPtrList * 
DFAState::getTransitiveStates(const Edge * e, StatePtrArray * ss) const{
  try {
    PredPtrList * pl = new PredPtrList();
    for (unsigned int i = 0; i<stateCount;i++){	// for-each NFA state
      State * s = states->getItem(i);
      s->addTransitiveStates(e,ss,pl);
    }
    return pl;
  }
  catch (_Error & err){
    err.addItem(new ErrItem((Base *)this, "DFAState::getTransitiveStates()", __LINE__));
    throw err;
  }
}

unsigned int
DFAState::getTransitiveStatesCount(const Edge * e) const{
  try {
    unsigned int count = 0;
    for (unsigned int i = 0; i<stateCount;i++){
      State * s = states->getItem(i);
      // for each DFA state, collect all states using Edge "e"
      s->addTransitiveStatesCount(e, count);
    }
    return count;
  }
  catch (_Error & err){
    err.addItem(new ErrItem((Base *)this, "DFAState::getTransitiveStatesCount()", __LINE__));
    throw err;
  }
}

void
DFAState::createEdge(DFAState * next, const Edge * e){
#if defined(XMLTK_DEBUG)
  cout << "createEdge(" << this << "->" << next << ": label = " << e->sp
       << endl;
#endif
  switch (e->getType()){
  case Edge::T_Element:
  case Edge::T_Attribute:
    (*edges)[e->sp->getPath()] = Edge(this, next, e->sp);
    break;
  case Edge::T_Text:
    textEdge = new Edge(this, next, e->sp);
    break;
  case Edge::T_AnyAttribute:
    anyAttributeEdge = new Edge(this, next, Edge::T_AnyAttribute);
    break;
  case Edge::T_AnyElement:
    anyElementEdge = new Edge(this, next, e->sp);
    break;
  default:
    cerr << errmes[E_NOT_SUPPORT] << endl;
    throw _Error((Base *)this, "DFAState::createEdge()", __LINE__);
  }
}


Edge *				// return next state or 0 (no transit)
DFAState::startElement(PString * ps){
  try {
    if (sinkDepth>0){
      sinkDepth++;
      return 0;
    }
    EdgeMap::iterator itr = edges->find(ps->getKey());
    if (itr != edges->end()){
      PredPtrList * prds = itr->second.preds;
      PredPtrList::iterator itr2;
      for (itr2 = prds->begin(); itr2 != prds->end(); ++itr2){
	(*itr2)->apply(parent,ps);
      }
      return &(itr->second);
    }
    else {			// not found in the edgeHashTable
      if (anyElementEdge){
	PredPtrList * prds = anyElementEdge->preds;
	PredPtrList::iterator itr2;
	for (itr2 = prds->begin(); itr2 != prds->end(); ++itr2){
	  (*itr2)->apply(parent,ps);
	}
	return anyElementEdge;
      }
      sinkDepth++;
      return 0;
    }
  }
  catch (_Error & err){
    err.addItem(new ErrItem((Base *)this, "DFAState::startElement()", __LINE__));
    throw err;
  }
}

Edge *
DFAState::attribute(PString * ps, PString * val){
  try {
    if (sinkDepth>0) return 0;
    EdgeMap::iterator itr = edges->find(ps->getKey());
    if (itr != edges->end()){
      PredPtrList * prds = itr->second.preds;
      PredPtrList::iterator itr2;
      for (itr2 = prds->begin(); itr2 != prds->end(); ++itr2){
	(*itr2)->apply(parent,val);
      }
      return &(itr->second);
    }
    else {			// not found in the EdgeHashTable
      if (anyAttributeEdge){
	PredPtrList * prds = anyAttributeEdge->preds;
	PredPtrList::iterator itr2;
	for (itr2 = prds->begin(); itr2 != prds->end(); ++itr2){
	  (*itr2)->apply(parent,val);
	}
	return anyAttributeEdge;
      }
      return 0;
    }
  }
  catch (_Error & err){
    err.addItem(new ErrItem((Base *)this, "DFAState::attribute()", __LINE__));
    throw err;
  }
}

Edge *
DFAState::characters(PString * ps){
  try {
    if (sinkDepth>0) return 0;
    if (textEdge){
      PredPtrList * prds = textEdge->preds;
      PredPtrList::iterator itr2;
      for (itr2 = prds->begin(); itr2 != prds->end(); ++itr2){
	(*itr2)->apply(parent,ps);
      }
      return textEdge;
    }
    return 0;
  }
  catch (_Error & err){
    err.addItem(new ErrItem((Base *)this, "DFAState::characters()", __LINE__));
    throw err;
  }
}

Edge *				// return next state or 0 (no transit)
LazyDFAState::startElement(PString * ps){
  try {
    if (sinkDepth>0){
      sinkDepth++;
      return 0;
    }
    if (edges == 0){		// this is the first access
      edges = new EdgeMap();
      EdgeList * el;
      getUniqEdges(el);
      for (EdgeList::iterator itr = el->begin(); itr != el->end(); ++itr){
	createEdge(0, &(*itr));	// next state is NULL because it is Lazy
      }
      delete el;
    }
    EdgeMap::iterator itr = edges->find(ps->getKey());
    if (itr != edges->end()){
      if (itr->second.to == 0){	// needs to generate next State
	StatePtrArray * ss =
	  new StatePtrArray(getTransitiveStatesCount(&(itr->second)));
	PredPtrList * pl = getTransitiveStates(&(itr->second), ss);
	itr->second.setPredicates(pl);
	itr->second.to = registerDFAState(ss);
      }
      PredPtrList * prds = itr->second.preds;
      PredPtrList::iterator itr2;
      for (itr2 = prds->begin(); itr2 != prds->end(); ++itr2){
	(*itr2)->apply(parent,ps);
      }
      return &(itr->second);
    }
    else {			// not found or "*" matches everything
      if (anyElementEdge){
	if (anyElementEdge->to == 0){ // needs to generate next State
	  StatePtrArray * ss =
	    new StatePtrArray(getTransitiveStatesCount(anyElementEdge));
	  PredPtrList * pl = getTransitiveStates(anyElementEdge, ss);
	  anyElementEdge->setPredicates(pl);
	  anyElementEdge->to = registerDFAState(ss);
	}
	PredPtrList * prds = anyElementEdge->preds;
	PredPtrList::iterator itr2;
	for (itr2 = prds->begin(); itr2 != prds->end(); ++itr2){
	  (*itr2)->apply(parent,ps);
	}
	return anyElementEdge;
      }
      sinkDepth++;
      return 0;
    }
  }
  catch (_Error & err){
    err.addItem(new ErrItem((Base *)this, "LazyDFAState::startElement()", __LINE__));
    throw err;
  }
}

Edge *
LazyDFAState::attribute(PString * ps, PString * val){
  try {
    if (sinkDepth>0) return 0;
    if (edges == 0){		// this is the first access
      edges = new EdgeMap();
      EdgeList * el;
      getUniqEdges(el);
      for (EdgeList::iterator itr = el->begin(); itr != el->end(); ++itr){
	createEdge(0, &(*itr));	// next state is NULL
      }
      delete el;
    }
    EdgeMap::iterator itr = edges->find(ps->getKey());
    if (itr != edges->end()){
      if (itr->second.to == 0){	// needs to generate next State
	StatePtrArray * ss =
	  new StatePtrArray(getTransitiveStatesCount(&(itr->second)));
	PredPtrList * pl = getTransitiveStates(&(itr->second), ss);
	itr->second.setPredicates(pl);
	itr->second.to = registerDFAState(ss);
      }
      PredPtrList * prds = itr->second.preds;
      PredPtrList::iterator itr2;
      for (itr2 = prds->begin(); itr2 != prds->end(); ++itr2){
	(*itr2)->apply(parent,val);
      }
      return &(itr->second);
    }
    else {			// not found or "@*" matches everything
      if (anyAttributeEdge){
	if (anyAttributeEdge->to == 0){ // needs to generate next State
	  StatePtrArray * ss =
	    new StatePtrArray(getTransitiveStatesCount(anyAttributeEdge));
	  PredPtrList * pl = getTransitiveStates(anyAttributeEdge, ss);
	  anyAttributeEdge->setPredicates(pl);
	  anyAttributeEdge->to = registerDFAState(ss);
	}
	PredPtrList * prds = anyAttributeEdge->preds;
	PredPtrList::iterator itr2;
	for (itr2 = prds->begin(); itr2 != prds->end(); ++itr2){
	  (*itr2)->apply(parent,val);
	}
	return anyAttributeEdge;
      }
      return 0;
    }
  }
  catch (_Error & err){
    err.addItem(new ErrItem((Base *)this, "LazyDFAState::attribute()", __LINE__));
    throw err;
  }
}

Edge *
LazyDFAState::characters(PString * ps){
  try {
    if (sinkDepth>0) return 0;
    if (edges == 0){		// this is the first access
      edges = new EdgeMap();
      EdgeList * el;
      getUniqEdges(el);
      for (EdgeList::iterator itr = el->begin(); itr != el->end(); ++itr){
	createEdge(0, &(*itr));	// next state is NULL
      }
      delete el;
    }
    if (textEdge){		// "text()" matches everything
      if (textEdge->to == 0){	// needs to generate next State
	StatePtrArray * ss =
	  new StatePtrArray(getTransitiveStatesCount(textEdge));
	PredPtrList * pl = getTransitiveStates(textEdge, ss);
	textEdge->setPredicates(pl);
	textEdge->to = registerDFAState(ss);
      }
      PredPtrList * prds = textEdge->preds;
      PredPtrList::iterator itr2;
      for (itr2 = prds->begin(); itr2 != prds->end(); ++itr2){
	(*itr2)->apply(parent,ps);
      }
      return textEdge;
    }
    return 0;
  }
  catch (_Error & err){
    err.addItem(new ErrItem((Base *)this, "LazyDFAState::characters()", __LINE__));
    throw err;
  }
}

DFAState *
LazyDFAState::registerDFAState(StatePtrArray * ss){
  return parent->registerDFAState(ss);
}

//
// convert an XPath expression into a NFA
//
void
Automata::create(XPath * path, AutomataPtrList * as) {
  try {
    xpath = path;
    xpath->setAutomata(this);
    State * top, * s, * next = 0;
    top = s = (* states->begin());
    for (unsigned int i = 0; i<xpath->getLStepCount();i++){
      LocationStep * sp = xpath->getLStep(i);
      switch (sp->getType()){
      case LocationStep::Attribute:
      case LocationStep::Text:
      case LocationStep::AnyAttribute:
	if (i != xpath->getLStepCount() -1 ){
	  cerr << errmes[E_LEAF_PATH_SYNTAX] << endl;
	  throw _Error(this, "Automata::create()", __LINE__);
	}
      case LocationStep::Element:
      case LocationStep::AnyElement:
      case LocationStep::Period:
	next = s->createEdge(sp); // next is a new state
	states->push_back(next);
	s = next;
	break;
      case LocationStep::DoublePeriod:
	cerr << errmes[E_PERIOD_NOT_SUPPORT] << endl;
	throw _Error(this, "Automata::create()", __LINE__);
      case LocationStep::AltElement:
	for (unsigned int j = 0; j<sp->getAltPathCount();j++){
	  XPath * altPath = sp->getAltPath(j);
	  State * stop = createSub(altPath);
	  stop = stop;
	}
	cerr << errmes[E_CHOICE_UNDER_IMPLEMENTING] << endl;
	throw _Error(this, "Automata::create()", __LINE__);
	break;
      case LocationStep::DoubleSlash:
	sp = xpath->getLStep(++i); // look next
	sp->setPredTypeDoubleSlash();
	next = s->createEdge(sp);  // next Edge
	states->push_back(next);
	s->createEdge(s, Edge::T_AnyElement); // loop Edge
	s = next;
	break;
      case LocationStep::DocumentRoot:
	cerr << errmes[E_INTERNAL] << endl;
	throw _Error(this, "Automata::create()", __LINE__);
      }	// end switch
				// 
				// converting a predicate to NFA
				// 
      List<Predicate> * lp = sp->getPredicates();
      for (unsigned int j = 0; j<lp->getCount();j++){
	if (i==xpath->getLStepCount()-1){
	  cerr << errmes[E_PREDICATE_SYNTAX] << endl;
	  throw _Error(this, "Automata::create()", __LINE__);
	}
				// Only simple predicate is supported
				// The style is [attribute opr value] or
				//              [text() opr value].
				// (both are checked out at xmatch.y)
	Predicate * p = lp->getItem(j);
	/*
	if (p->getType()==Predicate::T_PREDICATE){
	  cerr << errmes[E_NOT_SUPPORT] << endl;
	  throw _Error(this, "Automata::create()", __LINE__);
	  
	}
	*/
	if (p->getType()==Predicate::T_SIMPLE_PREDICATE){
	  AtomPredicate * atom = p->getTopAtom();
	  XPath * xp = atom->left->getXPath();
	  for (unsigned int k = 0; k<xp->getLStepCount();k++){
	    LocationStep * psp = xp->getLStep(k);
	    switch (psp->getType()){
	    case LocationStep::Attribute:
	    case LocationStep::AnyAttribute:
	    case LocationStep::Text:
	      // move the current Predicate to the attribute (in the pred)
	      lp->deleteItem(j);
	      psp->addPredicate(p);
	      // if the following predicate is index,
	      // then it also moves to the attribute
	      p = lp->getItem(j); // look ahead to check p is a index Predicate
	      if (p && p->getType()== Predicate::T_INDEX){
		lp->deleteItem(j);
		psp->addPredicate(p);
		cerr << errmes[E_POSITION_PREDICATE] << endl;
		throw _Error(this, "Automata::create()", __LINE__);
	      }
	      j--;		// ajust j for the deletions
	      next = s->createEdge(psp);
	      states->push_back(next);
	      s = next;
	      break;
	    default:
	      cerr << errmes[E_NOT_SUPPORT] << endl;
	      throw _Error(this, "Automata::create()", __LINE__);
	    }
	  }
	}
      }
    }	// end for (unsigned int i = 0; i<xpath->getLStepCount();i++){
    s->setTerminal();
    // set var for ternimal state
    s->var = xpath->getVariable();
    /*
    for (StatePtrList::iterator itr = states->begin(); itr != states->end(); ++itr){
      (*itr)->var = xpath->getVariable();
    }
    */
    return;
  }
  catch (_Error & err){
    err.addItem(new ErrItem(this, "Automata::create()", __LINE__));
    throw err;
  }
}

void
Automata::combineNFAs(AutomataPtrList * as){
  try {
    State * top = (* states->begin());
    for (AutomataPtrList::iterator itr = as->begin(); itr != as->end(); ++itr){
      // "as" is already sorted as
      // [[root Automata], [Automata with parent],...]
      Automata * a = (*itr);
      Automata * parent = a->getParent();
      if (parent==0){		// this is a root Automaton
	State * start = (* a->states->begin());
	top->createEdge(start,Edge::T_Epsilon);
      }
      else {			// Automaton with parent Automaton
	State * terminal = (* --(parent->states->end()));
	State * start = (* a->states->begin());
	terminal->createEdge(start,Edge::T_Epsilon);
      }
    }
  }
  catch (_Error & err){
    err.addItem(new ErrItem(this, "Automata::combineNFAs()", __LINE__));
    throw err;
  }
}

void
Automata::combineDFAs(AutomataPtrList * as){
  try {
    State * top = (* states->begin());
    for (AutomataPtrList::iterator itr = as->begin(); itr != as->end(); ++itr){
      // "as" is already sorted as
      // [[root Automata], [Automata with parent],...]
      Automata * a = (*itr);
      Automata * parent = a->getParent();
      if (parent==0){		// this is a root Automaton
	State * start = (* a->states->begin());
	top->createEdge(start,Edge::T_Epsilon);
      }
      else {			// Automaton with parent Automaton
	State * terminal = (* --(parent->states->end()));
	State * start = (* a->states->begin());
	terminal->createEdge(start,Edge::T_Epsilon);
      }
    }
  }
  catch (_Error & err){
    err.addItem(new ErrItem(this, "Automata::combineNFAs()", __LINE__));
    throw err;
  }
}

void
Automata::NFA2DFA(void){		// static powerset construct
  try {							
    if (type != T_NFA) return;
    // create inital DFA state
    State * s = (* states->begin());
    //StatePtrArray * ss = new StatePtrArray();	// for set
    StatePtrArray * ss = new StatePtrArray(1); // for hash_set and List
    ss->insert(s);				// for list
    DFAState * top = registerDFAState(ss);
    current = new Edge(0,top,Edge::T_DocumentRoot);
    // create the other DFA states
    // dfastates->getCount varies during the execution
    for (DFAStatePtrList::iterator itr = dfastates->begin(); itr != dfastates->end(); ++itr){
      createDFAStates(*itr);
    }
    type = T_DFA;
    return;
  }
  catch (_Error & err){
    err.addItem(new ErrItem(this, "Automata::NFA2DFA()", __LINE__));
    throw err;
  }
}

void
LazyAutomata::NFA2DFA(void){	// dynamic powerset construct
  try {				
    if (type != T_NFA) return;
    // create inital DFA state only
    State * s = (* states->begin());
    StatePtrArray * ss = new StatePtrArray(1); // for hash_set and List
    ss->insert(s);			// for list
    DFAState * top = new LazyDFAState(this,ss);
    top->edges = new EdgeMap();
    EdgeList * el;
    top->getUniqEdges(el);
    for (EdgeList::iterator itr = el->begin(); itr != el->end(); ++itr){
      top->createEdge(0, &(*itr)); // next state is NULL because it is Lazy
    }
    delete el;
    // re-calcurate the ss.
    // collect all state using epsilon Edges
    ss = new StatePtrArray(top->getTransitiveStatesCount(0));
    top->getTransitiveStates(0, ss);
    delete top;
    top = new LazyDFAState(this,ss);
    current = new Edge(0,top,Edge::T_DocumentRoot);
    dfastates->push_back(top);
    type = T_DFA;
    return;
  }
  catch (_Error & err){
    err.addItem(new ErrItem(this, "LazyAutomata::NFA2DFA()", __LINE__));
    throw err;
  }
}

void
Automata::createDFAStates(DFAState * dfas){	// dfas is a parent DFA state
  try {
    EdgeList * el;
    dfas->getUniqEdges(el);
    for (EdgeList::iterator itr = el->begin(); itr != el->end(); ++itr){
      // collect all reachable states
      // using the Edge e.
      StatePtrArray * ss =
	new StatePtrArray(dfas->getTransitiveStatesCount(&(*itr)));
      PredPtrList * pl = dfas->getTransitiveStates(&(*itr), ss);
      itr->setPredicates(pl);
      DFAState * next = registerDFAState(ss);
      dfas->createEdge(next, &(*itr));
    }
    delete el;
    return;	
  }
  catch (_Error & err){
    err.addItem(new ErrItem(this, "Automata::createDFAStates()", __LINE__));
    throw err;
  }
}

State *
Automata::createSub(XPath * altPath) {
  State * stop = 0;
  return stop;
}

void
State::print(ofstream* ofs){
  if (edges == 0){
    *ofs << "    <nfaState address=\"" << this << "\"</>" << endl;
    return;
  }
  *ofs << "    <nfaState edgeCount=\"" << edges->size() << "\" address=\"" << this;
  if (type == T_TERMINAL) *ofs << "\" type=\"terminal";
  else *ofs << "\" type=\"non-terminal";
  *ofs << "\">" << endl;
  /*
  for (EdgeList::iterator itr = edges->begin(); itr != edges->end(); ++itr){
    Edge * e = &(*itr);
    *ofs << "      <edge address=\"" << e << "\" label=\"" << e->getLabel() << "\" toStateAddress=\"" << e->to << "\">" << endl;
  }
  */
  *ofs << "    </nfaState>" << endl;
}

void
DFAState::print(ofstream* ofs){
  if (edges == 0){
    *ofs << "    <dfaState address=\"" << this;
    *ofs << "\" varCount=\"" << vars->getCount() << "\"";
    if (type == T_TERMINAL){
      *ofs << " type=\"terminal\"";
    }
    else *ofs << " type=\"non-terminal\"";
    *ofs << " nfaStateCount=\"" << stateCount << "\"/>" << endl;
    return;
  }
  unsigned int edgeCount = edges->size();
  if (anyAttributeEdge) edgeCount++;
  if (anyElementEdge) edgeCount++;
  if (textEdge) edgeCount++;
  *ofs << "    <dfaState edgeCount=\"" << edgeCount << "\" address=\"" << this;
  *ofs << "\" varCount=\"" << vars->getCount() << "\"";
  if (type == T_TERMINAL) *ofs << " type=\"terminal\"";
  else *ofs << " type=\"non-terminal\"";
  *ofs << " nfaStateCount=\"" << stateCount << "\">" << endl;
    for (EdgeMap::iterator itr = edges->begin(); itr != edges->end(); ++itr){
    Edge * e = &(itr->second);
    *ofs << "      <edge address=\"" << e << "\" label=\"" << e->getLabel() << "\" toStateAddress=\"" << e->to << "\">" << endl;
    }
    if (anyAttributeEdge){
    Edge * e = anyAttributeEdge;
    *ofs << "      <edge address=\"" << e << "\" label=\"" << e->getLabel() << "\" toStateAddress=\"" << e->to << "\">" << endl;
    }
    if (anyElementEdge){
    Edge * e = anyElementEdge;
    *ofs << "      <edge address=\"" << e << "\" label=\"" << e->getLabel() << "\" toStateAddress=\"" << e->to << "\">" << endl;
    }
    if (textEdge){
    Edge * e = textEdge;
    *ofs << "      <edge address=\"" << e << "\" label=\"" << e->getLabel() << "\" toStateAddress=\"" << e->to << "\">" << endl;
    }
  *ofs << "    </dfaState>" << endl;
}

void
Automata::printAutomaton(ofstream* ofs){
  try{
    if (type == T_DFA){
      *ofs << "  <dfaStates count=\"" << dfastates->size() << "\">" << endl;
      for (DFAStatePtrList::iterator itr = dfastates->begin(); itr != dfastates->end(); ++itr){
	(*itr)->print(ofs);
      }
      *ofs << "  </dfaStates>" << endl;
    }
    else if (type == T_NFA){
      *ofs << "  <nfaStates count=\"" << states->size() << "\">" << endl;
      for (StatePtrList::iterator itr = states->begin(); itr != states->end(); ++itr){
	(*itr)->print(ofs);
      }
      *ofs << "  </nfaStates>" << endl;
    }
  }
  catch (_Error & err){
    err.addItem(new ErrItem(this, "Automaton::printAutomaton()", __LINE__));
    throw err;
  }
}

Automata::FAStates
Automata::startDocument(void){
  prev = 0;
  if (current->to->isTerminal()) return S_COMPLETE;
  else return S_INACTIVE;
}

Automata::FAStates
Automata::endDocument(void){
  return S_INACTIVE;
}

Automata::FAStates
Automata::startElement(PString * ps){
  try {
    Edge * edge = current->to->startElement(ps);
    prev = 0;
    // either transit (stack state and increment element index), 
    // no transit (increment sinkDepth, or increment element index)
    if (edge == 0) return S_SKIP;
    if (edge->to == current->to){
      PredPtrList * pl = current->to->getIndexPreds();
      if (!pl || pl->size()==0){ // if the next state is the same as current
				 // and no indexPred (no need to stack)
	current->to->stayDepth++;
	if (current->to->isTerminal()) return S_COMPLETE;
	else return S_INACTIVE;
      }
    }
    StackItem * si = new StackItem(current,ps);
    estack->insertItem(si);
    current = edge;
    if (current->to->isTerminal()) return S_COMPLETE;
    else return S_INACTIVE;
  }
  catch (_Error & err){
    err.addItem(new ErrItem(this, "Automata::startElement()", __LINE__));
    throw err;
  }
}

Automata::FAStates
Automata::endElement(void){
  try {
    if (current->to->stayDepth>0){ // fake pop check
      current->to->stayDepth--;
      if (isTerminal()) return S_COMPLETE; 
      else if (current->to->isSatisfiable()) return S_SKIP;
      else return S_INACTIVE;
    }
				  //  need to State::endElement()
    if (current->to->endElement()){
    // false: in case unmatch or after completing path expression (sink state)
    // true : real transition (need to real pop)
      current->resetPredicate(this);
      StackItem * si;
      while(1){			// pop all attribute edges
	if (current->to->stayDepth>0){
	  current->to->stayDepth--; 	// fake pop
	  if (isTerminal()) return S_COMPLETE; 
	  else if (current->to->isSatisfiable()) return S_SKIP;
	  else return S_INACTIVE;
	}
	else if (current->getType() == Edge::T_DocumentRoot){
	  cerr << errmes[E_INTERNAL] << endl;
	  throw _Error(this, "Automata::endElement()", __LINE__);
	}
	if (current->getType() == Edge::T_Element || // for element
	    current->getType() == Edge::T_AnyElement){
	  break;
	}
	else {			// for attribute
	  si = estack->pop();	// real pop for attribute
	  current = si->getPopEdge();
	  current->resetPredicate(this);
	  delete si;
	}
      }	// end while
				// now current is for element entry
      si = estack->pop();	// real pop
      current = si->getPopEdge();
      delete si;
    }
    if (isTerminal()) return S_COMPLETE; 
				// Automata::isTerminal() is different
				// with current->to->isTerminal()
    else if (current->to->isSatisfiable()) return S_SKIP;
    else return S_INACTIVE;
  }
  catch (_Error & err){
    err.addItem(new ErrItem(this, "Automata::endElement()", __LINE__));
    throw err;
  }
}

VarPtrArray *
Automata::attribute(PString * ps, PString * val){
  try {
    Edge * edge = current->to->attribute(ps,val);
    if (edge == 0) return 0;
				// don't rely on the stayDepth
				// because we need to know the increment is
				// occurred by element or attribute
    StackItem * si = new StackItem(current,ps);
    estack->insertItem(si);
    current = edge;
    prev = 0;
    if (current->to->isTerminal())
      return current->to->getStateVariables();
    else return 0;
  }
  catch (_Error & err){
    err.addItem(new ErrItem(this, "Automata::attribute()", __LINE__));
    throw err;
  }
}

VarPtrArray * 
Automata::characters(PString * val){
  try {
    Edge * edge = current->to->characters(val);
    if (edge == 0) return 0;
    StackItem * si = new StackItem(current,val);
    estack->insertItem(si);
    current = edge;
    prev = 0;
    if (current->to->isTerminal())
      return current->to->getStateVariables();
    else return 0;
  }
  catch (_Error & err){
    err.addItem(new ErrItem(this, "Automata::characters()", __LINE__));
    throw err;
  }
}

