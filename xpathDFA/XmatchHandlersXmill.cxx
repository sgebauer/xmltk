// -*- mode: c++ -*-
//  This is a DfaHandler module for DFA processor.
//  copyright (C) 2001 Makoto Onizuka, University of Washington
//  $Id: XmatchHandlersXmill.cxx,v 1.1.1.1 2002/05/04 12:54:02 tjgreen Exp $

#include <xmlparse.hpp>
#include "XmatchHandlersXmill.hpp"
#include "Query.h"
#include "Depth2offset.hpp"
extern Query 			* gQuery;
extern XMLParse			* parser;
extern Depth2offset		* d2o;
unsigned int outputLevel; // for checking output state and skip state
VarPtrArray * vpl;		  // for startContext, endContext

unsigned int contextCount;
unsigned int elementCount;
unsigned int skipCount;
unsigned int skipMax;
unsigned int skipSum;

XmatchHandlers::XmatchHandlers (void): SAXClient() {
  gQuery->startDocument();
  outputLevel = 0;
  vpl = 0;
  
  elementCount = 0;
  contextCount = 0;
  skipCount = 0;
  skipMax = 0;
  skipSum = 0;
#if defined(XMLTK_OUTPUT)
  cout << "<root>" << endl;
#endif
}

XmatchHandlers::~XmatchHandlers(void) {
  gQuery->endDocument();
#if defined(XMLTK_OUTPUT)
  cout << "</root>" << endl;
  cout << "startElement count= " << elementCount << ",";
  cout << "startContext count= " << contextCount << ",";
  cout << "skip count= " << skipCount << ",";
  cout << "max skip= " << skipMax  << ",";
  cout << "avg skip= " << (float)skipSum/skipCount << endl;
#endif
}


// ---------------------------------------------------------------------------
//  XmatchHandlers: Overrides of the Xmill's DefaultHanders interface
// ---------------------------------------------------------------------------
void
XmatchHandlers::HandleAttribName(char *str,int len,char iscont){}

void
XmatchHandlers::HandleAttribValue(char *str,int len,char iscont){}

void
XmatchHandlers::HandleAttribWhiteSpaces(char *str,int len,char iscont){}

void
XmatchHandlers::HandleStartLabel(char *name,int len,char iscont){
  elementCount++;
  vpl = 0;						// to store the current completed Variables
  if (d2o) d2o->startElement(parser->getSrcOffset()); // for streamIndex
  PString * ps = new PString(name, len);
#if defined(XMLTK_DEBUG)
  cout << "startElement(" << ps->getString() << ", " << len << ") outputLevel=" << outputLevel << ", ";
#endif
  switch(gQuery->startElement(ps)){
  case Query::S_INACTIVE:
	if (outputLevel > 0) outputLevel++;
#if defined(XMLTK_DEBUG)
	cout << "INACTIVE" << endl;
#endif
	break;
  case Query::S_COMPLETE:{
	outputLevel++;
#if defined(XMLTK_DEBUG)
	cout << "COMPLETE" << endl;
#endif
	vpl = gQuery->getStateVariables();
#if defined(XMLTK_ITERATION)
	unsigned int c = vpl->getCount();
	for (unsigned int i = 0; i<c; i++){
	  Variable *v = vpl->getItem(i);
	  contextCount++;
#if defined(XMLTK_OUTPUT)
	  cout << "startContext(" << v->varName << ")" << endl;
#endif
	}
#endif
	break;
  }
  case Query::S_SKIP:
	//
	// skip XML stream (both parser, Depth2offset, automata)
	// 1. just skip one level if there is no schema information.
	//    This is just for index skip (author[2] for example).
	// 2. skip some level using schema information.
	//
#if defined(XMLTK_DEBUG)
	cout << "SKIP" << endl;
#endif
	if (outputLevel == 0 && d2o){
	  if (d2o->isAppliable()==true){
		unsigned int skipDepth = gQuery->failSkip();
		unsigned int offset = d2o->failSkip(skipDepth);
		parser->skipReader(offset, skipDepth);
		skipCount++;
		if (skipMax<offset) skipMax = offset;
		skipSum += offset;
#if defined(XMLTK_DEBUG)
		cout << "  FAIL SKIP(level=" << skipDepth << ", offset=" << offset << ") start:" << ps->getString() << endl;
#endif
	  }
	}
	else if (outputLevel > 0) outputLevel++;
	break;
  }
#if defined(XMLTK_OUTPUT)
  if (outputLevel>0) cout << "<" << ps->getString() << ">";
#endif
  delete ps;
}

void
XmatchHandlers::HandleEndLabel(char *name,int len,char iscont){
#if defined(XMLTK_OUTPUT)
  if (outputLevel>0){
	cout << "</>";
	if (outputLevel == 1) cout << endl;
  }
#endif

  if (vpl != 0 && vpl->getCount() != 0){
#if defined(XMLTK_ITERATION)
	for (unsigned int i = vpl->getCount()-1;; i--){
	  Variable *v = vpl->getItem(i);
#if defined(XMLTK_OUTPUT)
	  cout << "endContext(" << v->varName << ")" << endl;
#endif
	  if (i==0) break;
	}
#endif
	vpl = 0;
  }

#if defined(XMLTK_DEBUG)
  cout << "endElement() outputLevel=" << outputLevel << ", ";
#endif
  if (outputLevel > 0) outputLevel--;
  if (d2o) d2o->endElement(parser->getSrcOffset());// for streamingIndex
  PString * ps = new PString(name, len);
  switch(gQuery->endElement(ps)){
  case Query::S_INACTIVE:
#if defined(XMLTK_DEBUG)
	cout << "INACTIVE" << endl;
#endif
	break;
  case Query::S_COMPLETE:
#if defined(XMLTK_DEBUG)
	cout << "COMPLETE" << endl;
#endif
	vpl = gQuery->getStateVariables();
	break;
  case Query::S_SKIP:
#if defined(XMLTK_DEBUG)
	cout << "SKIP" << endl;
#endif
	if (outputLevel == 0 && d2o){
#if defined(XMLTK_DEBUG)
	  cout << "suceed skip" << endl;
#endif
	  if (d2o->isAppliable()==true){
		unsigned int skipDepth = gQuery->succeedSkip();
		unsigned int offset = d2o->succeedSkip(skipDepth, parser->getSrcOffset());
		parser->skipReader(offset, skipDepth);
		skipCount++;
		if (skipMax<offset) skipMax = offset;
		skipSum += offset;
#if defined(XMLTK_DEBUG)
		cout << "  SUCEED SKIP(level=" << skipDepth << ", offset=" << offset << ") start:" << endl;
#endif
	  }
	}
	break;
  }
  delete ps;
}

void
XmatchHandlers::HandleText(char *chars,int len,char iscont,int leftwslen,int rightwslen){
#if defined(XMLTK_OUTPUT)
  if (outputLevel>0){
	PString * ps = new PString(chars, len);
	gQuery->characters(ps);
	cout << ps->getString();
	delete ps;
  }
#endif
}

void
XmatchHandlers::HandleComment(char *str,int len,char iscont){}

void
XmatchHandlers::HandlePI(char *str,int len,char iscont){}

void
XmatchHandlers::HandleDOCTYPE(char *str,int len,char iscont){}

void
XmatchHandlers::HandleCDATA(char *str,int len,char iscont){}
