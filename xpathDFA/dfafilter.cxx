// -*- mode: c++ -*-
//  This is a CDfaFilter module for DFA processor.
//  This invokes application events according to the
//  Automata state transitted for each XML parser SAX 
//  events.
//
//  copyright (C) 2001-2002 Makoto Onizuka, University of Washington
//

#include "xmltk.h"
#include "Query.h"

IParse2TSAX * g_xmlparse;			// for SIX operation
static unsigned int counter;

class CDfaFilter : public IDfaFilter
                 , public ISetHandler
{
public:
  // *** IUnknownCL methods ***
  CL_STDMETHOD(QueryInterface) (RCLIID riid, void **ppvObj);
  CL_STDMETHOD_(ULONG,AddRef) ();
  CL_STDMETHOD_(ULONG,Release) ();

  // *** ITSAXContentHandler methods ***
  CL_STDMETHOD(startDocument) ();
  CL_STDMETHOD(endDocument) ();
  CL_STDMETHOD(startElement) (XTOKEN xtName);
  CL_STDMETHOD(endElement) (XTOKEN xtName);
  CL_STDMETHOD(attribute) (XTOKEN xtName, char *pszChars, int cchChars);
  CL_STDMETHOD(characters) (char *pszChars, int cchChars);
  CL_STDMETHOD(cdata) ( char *pszChars, int cchChars);
  CL_STDMETHOD(extendedint) ( XTOKEN xt, int iInt);

  // *** IDfaFilter methods ***
  CL_STDMETHOD_(Variable*,RegisterQuery) (Variable *pv, const char *psz, bool bEvents, float precedence);
  CL_STDMETHOD_(Variable*,RegisterQuery) (Variable *pv, const char *psz, bool bEvents);
  CL_STDMETHOD(RegisterQueryFile) (const char *psz);

  // *** ISetHandler methods ***
  CL_STDMETHOD(SetHandler) (IUnknownCL *punk);

  CDfaFilter(void) : m_pfh(NULL), m_cRef(1), m_iOutputLevel(0)
		   , m_pvCompleteList(NULL)
  {
  }

private:
    
  virtual ~CDfaFilter(){
    if (m_pfh) m_pfh->Release();
  }
    
  Query m_queryMgr;
  IFilteredTSAXHandler *m_pfh;
  ULONG m_cRef;
  int m_iOutputLevel;
  VarPtrArray * m_pvCompleteList;
};

// *** IUnknownCL methods ***
bool CDfaFilter::QueryInterface(RCLIID rcliid, void **ppvObj)
{
  if (IsEqualCLIID(rcliid, &IID_IUnknownCL) ||
      IsEqualCLIID(rcliid, &IID_ITSAXContentHandler) ||
      IsEqualCLIID(rcliid, &IID_IDfaFilter))
    {
      *ppvObj = static_cast<IDfaFilter*>(this);
    }
  else if (IsEqualCLIID(rcliid, &IID_ISetHandler))
    {
      *ppvObj = static_cast<ISetHandler*>(this);
    }
  else
    {
      *ppvObj = NULL;
      return false;
    }
  AddRef();
  return true;
}

ULONG CDfaFilter::AddRef()
{
  return ++m_cRef;
}

ULONG CDfaFilter::Release()
{
  if (--m_cRef > 0)
    return m_cRef;

  delete this;
  return 0;
}

// *** IDfaFilter methods ***
Variable *CDfaFilter::RegisterQuery(Variable *pv, const char *psz, bool bEvents, float fPrec)
{
  return m_queryMgr.registerQuery(pv, psz, bEvents, fPrec);
}

Variable *CDfaFilter::RegisterQuery(Variable *pv, const char *psz, bool bEvents)
{
  Variable * v = m_queryMgr.registerQuery(pv, psz, bEvents);
#if defined(XMLTK_OUTPUT)
  cout << "Query::registerQuery(" << psz << ") for lazyDFA: ";
  cout << "parent = " << pv << ", self = " << v << endl;
#endif
  return v;
}

bool CDfaFilter::RegisterQueryFile(const char *queryFile)
{
  m_queryMgr.registerQueryFile(queryFile);
  return true;
}

// *** ISetHandler methods ***
bool CDfaFilter::SetHandler(IUnknownCL *punk)
{
  ATOMICRELEASE(m_pfh);
  return punk->QueryInterface(&IID_IFilteredTSAXHandler, (void**)&m_pfh);
}

// *** ITSAXContentHandler methods ***
bool CDfaFilter::startDocument()
{
  counter=0;
  m_queryMgr.createAutomata();
  //	ofstream * ofs = new ofstream("dfa.xml", ios::out);
  //	m_queryMgr.printAutomaton(ofs);
  //	ofs->close();

  switch (m_queryMgr.startDocument()){
  case Query::S_INACTIVE:	// the Automata is not in COMPLETE state
  case Query::S_SKIP:		// avoid the compiler warning
    break;
  case Query::S_COMPLETE:{	// the Automata is in COMPLETE state
    VarPtrArray *plv = m_queryMgr.getStateVariables();
    unsigned int c = plv->getCount();
    for (unsigned int i = 0; i<c; i++){
      Variable *pv = plv->getItem(i);
      if (pv->getEnableFlag()) m_pfh->startContext(pv);
#if defined(XMLTK_OUTPUT)
      cout << "startContext(" << pv << ")" << endl;
#endif
      m_iOutputLevel += pv->getOutputFlag();
    }
    // set the CompleteFlag as the variable list.
    m_pvCompleteList = plv;
    break;
  } // end COMPLETE
  } // end switch
#if defined(XMLTK_OUTPUT)
  cout << "<documentRoot>";
#endif

  //  m_pfh->startDocument();
  return true;
}

bool CDfaFilter::endDocument(){
  unsigned int c;
  if (m_pvCompleteList && (c=m_pvCompleteList->getCount())!= 0){
    // when the root element is matched with some XPEs,
    // then output the endContext in reverse order of
    // the startContext event.
    for (unsigned int i = c-1;; i--){
      Variable *pv = m_pvCompleteList->getItem(i);
      if (pv->getEnableFlag()) m_pfh->endContext(pv);
#if defined(XMLTK_OUTPUT)
      cout << "endContext(" << pv << ")" << endl;
#endif
      m_iOutputLevel -= pv->getOutputFlag();

      if (i==0) break;
    }
    m_pvCompleteList = NULL;
  }
  m_queryMgr.endDocument();
  //  m_pfh->endDocument();
  /* for DFA debug
     ofstream * ofs = new ofstream("dfa.out", ios::out);
     m_queryMgr.printAutomaton(ofs);
     ofs->close();
     delete ofs;
  */
#if defined(XMLTK_OUTPUT)
  cout << "</documentRoot>";
#endif
  return true;
}

bool CDfaFilter::startElement(XTOKEN xtName)
{
  if (counter++==0) m_pfh->startDocument();
  m_pvCompleteList = NULL;	// initialize the current completed Variables

  // create universal label PString object that
  // can be applied for xerces, xmill, xtoken
  PString * ps = new PString(xtName);

  // push the event to Automata
  switch (m_queryMgr.startElement(ps)){
  case Query::S_INACTIVE:{	// the Automata is not in COMPLETE state
    if (m_iOutputLevel > 0){	// if the output flag is on,
      // call application events.
      m_pfh->startElement(xtName);
    }
    break;
  }
  case Query::S_SKIP:		// the Automata is in potencially skip-able state.
    if (m_iOutputLevel > 0){	// if the output flag is on,
				// call application events.
      m_pfh->startElement(xtName);
    }
    else {			// try to skip (fail skip)
      unsigned int skipDepth = m_queryMgr.getFailSkipDepth();
      if (g_xmlparse->skip(skipDepth)){
	m_queryMgr.failSkip();
	m_pvCompleteList = m_queryMgr.getStateVariables(); // reset
      }
    }
    break;
  case Query::S_COMPLETE:{	// the Automata is in COMPLETE state
    VarPtrArray *plv = m_queryMgr.getStateVariables();
    unsigned int c = plv->getCount();
    for (unsigned int i = 0; i<c; i++){
      Variable *pv = plv->getItem(i);
      if (pv->getEnableFlag()) m_pfh->startContext(pv);
#if defined(XMLTK_OUTPUT)
      cout << "startContext(" << pv << ")" << endl;
#endif
      // outputFlag override:
      // m_iOutputLevel += getOutputFlag(self) - getOutputFlag(parent).
      Variable * parent = pv->getParent();
      if (parent)
	m_iOutputLevel += pv->getOutputFlag() - parent->getOutputFlag();
      else
	m_iOutputLevel += pv->getOutputFlag();
    }
    // we invoke startElement event
    m_pfh->startElement(xtName);
    // set the CompleteFlag as the variable list.
    m_pvCompleteList = plv;
    break;
  } // end COMPLETE
  } // end switch
#if defined(XMLTK_OUTPUT)
  //  if (m_iOutputLevel > 0)
  cout << "<" << g_ptt->StrFromXTOKEN(xtName) << ">";
#endif
  delete ps;
  return true;
}

bool CDfaFilter::endElement(XTOKEN xtName)
{
#if defined(XMLTK_OUTPUT)
  /*
  if (m_iOutputLevel > 0){
    cout << "</>";
    if (m_iOutputLevel == 1) cout << endl;
  }
  */
  cout << "</>" << endl;
#endif
  // Depth2offset processing for streamingIndex
  PString * ps = new PString(xtName);
  if (m_iOutputLevel > 0){	// if the output flag is on,
      // call application events.
    m_pfh->endElement(xtName);
  }
  unsigned int c;
  if (m_pvCompleteList && (c=m_pvCompleteList->getCount())!= 0){
    if (m_iOutputLevel == 0) m_pfh->endElement(xtName);
      // the correspond startElement is COMPLETE.
      // output the endContext in reverse order of
      // the startContext event.
    for (unsigned int i = c-1;; i--){
      Variable *pv = m_pvCompleteList->getItem(i);
      if (pv->getEnableFlag()) m_pfh->endContext(pv);
#if defined(XMLTK_OUTPUT)
      cout << "endContext(" << pv << ")" << endl;
#endif
	// outputFlag override:
	// m_iOutputLevel += getOutputFlag(self) - getOutputFlag(parent).
      Variable * parent = pv->getParent();
      if (parent) m_iOutputLevel += parent->getOutputFlag() - pv->getOutputFlag();
      else m_iOutputLevel -= pv->getOutputFlag();
      if (i==0) break;		// because i is unsigned int.
    }
  }
  // push the event to Automata
  switch (m_queryMgr.endElement(ps)){
  case Query::S_INACTIVE:	// the Automata is not in COMPLETE state
    m_pvCompleteList = NULL;
    break;
  case Query::S_SKIP:		// the Automata is in potencially skip-able state.
				// try to skip (succeed skip)
    m_pvCompleteList = NULL;
    if (m_iOutputLevel == 0){
      unsigned int skipDepth = m_queryMgr.getSucceedSkipDepth();
      if (g_xmlparse->skip(skipDepth)){
	for (unsigned int i = 0; i < skipDepth; i++){
	  PString * label = m_queryMgr.succeedSkip();
	  if (m_iOutputLevel > 0) m_pfh->endElement(label->getKey());
	  m_pvCompleteList = m_queryMgr.getStateVariables(); // reset
	  if (i != skipDepth-1 && m_pvCompleteList && c!= 0){
	    if (m_iOutputLevel == 0) m_pfh->endElement(xtName);
	    // the correspond startElement is COMPLETE.
	    // output the endContext in reverse order of
	    // the startContext event.
	    for (unsigned int j = c-1;; j--){
	      Variable *pv = m_pvCompleteList->getItem(j);
	      if (pv->getEnableFlag()) m_pfh->endContext(pv);
	      // outputFlag override:
	      // m_iOutputLevel += getOutputFlag(self) - getOutputFlag(parent).
	      Variable * parent = pv->getParent();
	      if (parent)
		m_iOutputLevel += parent->getOutputFlag() - pv->getOutputFlag();
	      else
		m_iOutputLevel -= pv->getOutputFlag();
	      if (j==0) break;
	    }
	  }
	}
      }
    }
    break;
  case Query::S_COMPLETE:	// the Automata is in COMPLETE state
				// we invoke startElement event
    m_pvCompleteList = m_queryMgr.getStateVariables();
    break;
  } // end switch

  delete ps;
  if (--counter==0) m_pfh->endDocument();
  return true;
}

bool CDfaFilter::attribute(XTOKEN xtName, char *pszChars, int cchChars)
{
  PString * ps = new PString(xtName);
  PString * val = new PString(pszChars, cchChars);

  // push the event to Automata
  VarPtrArray *plv = m_queryMgr.attribute(ps,val);
  if (plv == 0){		// S_INACTIVE
    if (m_iOutputLevel > 0){	// if the output flag is on,
      // call application events.
      m_pfh->attribute(xtName, pszChars, cchChars);
    }
  }
  else{				// the Automata is in COMPLETE state
    // call attribute event for each variable.
    unsigned int c = plv->getCount();
    for (unsigned int i = 0; i<c; i++){
      Variable *pv = plv->getItem(i);
      if (pv->getEnableFlag()) m_pfh->startContext(pv);
#if defined(XMLTK_OUTPUT)
      cout << "startContext(" << pv << ")" << endl;
#endif
    }
    m_pfh->attribute(xtName, pszChars, cchChars);

    if (c != 0){
      for (unsigned int i = c-1;; i--){
	Variable *pv = plv->getItem(i);
	if (pv->getEnableFlag()) m_pfh->endContext(pv);
#if defined(XMLTK_OUTPUT)
	cout << "endContext(" << pv << ")" << endl;
#endif
	if (i==0) break;	// because i is unsigned int.
      }
    }
  } // end COMPLETE
#if defined(XMLTK_OUTPUT)
  //  if (m_iOutputLevel > 0)
  cout << "<" << g_ptt->StrFromXTOKEN(xtName) << ">";
#endif
  delete ps;
  delete val;
  return true;
}

bool CDfaFilter::characters(char *pszChars, int cchChars)
{
  PString * val = new PString(pszChars, cchChars);
#if defined(XMLTK_OUTPUT)
  cout << val->getString();
#endif
  // push the event to Automata
  VarPtrArray *plv = m_queryMgr.characters(val);
  if (plv == 0){				// S_INACTIVE
    if (m_iOutputLevel > 0){	// if the output flag is on,
      // call application events.
      m_pfh->characters(pszChars, cchChars);
    }
  }
  else{				// the Automata is in COMPLETE state
    // call attribute event for each variable.
    unsigned int c = plv->getCount();
    for (unsigned int i = 0; i<c; i++){
      Variable *pv = plv->getItem(i);
      if (pv->getEnableFlag()) m_pfh->startContext(pv);
#if defined(XMLTK_OUTPUT)
      cout << "characters(" << pv << ")" << endl;
#endif
    }
    m_pfh->characters(pszChars, cchChars);
    if (c != 0){
      for (unsigned int i = c-1;; i--){
	Variable *pv = plv->getItem(i);
	if (pv->getEnableFlag()) m_pfh->endContext(pv);
#if defined(XMLTK_OUTPUT)
	cout << "endContext(" << pv << ")" << endl;
#endif
	if (i==0) break;		// because i is unsigned int.
      }
    }
    m_queryMgr.endCharacters();	// needs to reset the Variables enableFlags.
  } // end COMPLETE
  delete val;
  return true;
}

bool CDfaFilter::cdata(char *pszChars, int cchChars)
{
  if (m_iOutputLevel > 0)
    {
      m_pfh->cdata(pszChars, cchChars);
    }
  return false;
}
   
bool CDfaFilter::extendedint(XTOKEN xt, int iInt)
{
  if (m_iOutputLevel > 0)
    {
      m_pfh->extendedint(xt, iInt);
    }
  return false;
}


bool CreateDfaFilter(RCLIID rcliid, void **ppvObj)
{
  bool bRet = false;
    
  CDfaFilter* pdxr = new CDfaFilter();
  if (pdxr)
    {
      bRet = pdxr->QueryInterface(rcliid, ppvObj);
      pdxr->Release();
    }
    
  return bRet;
}
