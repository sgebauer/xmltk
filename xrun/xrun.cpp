#include "xmltk.h"

class myHandler : public IFilteredTSAXHandler
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
    CL_STDMETHOD(cdata) (char *pszChars, int cchChars);
    CL_STDMETHOD(extendedint) (XTOKEN xt, int iInt);

    // *** IFilteredTSAXHandler methods ***
    CL_STDMETHOD(startContext) (Variable *pv);
    CL_STDMETHOD(endContext) (Variable *pv);

    myHandler(ITSAXContentHandler *pchOut) {
	  m_pchOut = pchOut;
	  m_pchOut->AddRef();
	}
    virtual ~myHandler(){
	  ATOMICRELEASE(m_pchOut); 
	}

    ITSAXContentHandler *m_pchOut;
    ULONG m_cRef;
};

// *** IUnknown methods ***
bool myHandler::QueryInterface(RCLIID riid, void **ppvObj)
{
    if (IsEqualCLIID(riid, &IID_IUnknownCL) ||
        IsEqualCLIID(riid, &IID_ITSAXContentHandler) ||
        IsEqualCLIID(riid, &IID_IFilteredTSAXHandler)){
        *ppvObj = static_cast<IFilteredTSAXHandler*>(this);
    }
    else {
        *ppvObj = NULL;
        return false;
    }
    AddRef();
    return true;
}

ULONG myHandler::AddRef(){
    return ++m_cRef;
}

ULONG myHandler::Release(){
    if (--m_cRef > 0)
    {
        return m_cRef;
    }
    delete this;
    return 0;
}

// *** ITSAXContentHandler methods ***
bool myHandler::startDocument(){
    m_pchOut->startDocument();
    return true;
}

bool myHandler::endDocument(){
  //    m_pchOut->endDocument();
  // This is for not output the last newline
    return true;
}

bool myHandler::startElement(XTOKEN xtName){
#if defined(XMLTK_OUTPUT)	
  cerr << "startElement(" << xtName << ", " << g_ptt->StrFromXTOKEN(xtName) << ")" << endl;
#else
  m_pchOut->startElement(xtName); 
#endif
  return true;
}

bool myHandler::endElement(XTOKEN xtName){
#if defined(XMLTK_OUTPUT)	
  cerr << "endElement(" << xtName << ", " << g_ptt->StrFromXTOKEN(xtName) << ")" << endl;
#else
  m_pchOut->endElement(xtName);
#endif
  return true;
}

bool myHandler::attribute(XTOKEN xtName, char *pszChars, int cchChars){
#if defined(XMLTK_OUTPUT)	
  cerr << "attribute(" << xtName << ", " << g_ptt->StrFromXTOKEN(xtName) << ")" << endl;
#else
  m_pchOut->attribute(xtName, pszChars, cchChars);
#endif
  return true;
}

bool myHandler::characters(char *pszChars, int cchChars){
#if defined(XMLTK_OUTPUT)	
#else
    m_pchOut->characters(pszChars, cchChars);
#endif
    return true;
}

bool myHandler::cdata(char *pszChars, int cchChars){
    m_pchOut->cdata(pszChars, cchChars);
    return true;
}

bool myHandler::extendedint(XTOKEN xt, int iInt){
    m_pchOut->extendedint(xt, iInt);
    return true;
}

// *** IFilteredTSAXHandler methods ***
bool myHandler::startContext(Variable *pv){
#if defined(XMLTK_OUTPUT)	
  cerr << "startContext(" << pv << ")" << endl;
#endif
  return true;
}

bool myHandler::endContext(Variable *pv){
#if defined(XMLTK_OUTPUT)	
  cerr << "endContext(" << pv << ")" << endl;
#endif
  return true;
}

int main(int argc, char* argv[]){
  if (argc != 3){
	cout << "Usage: " << argv[0] << " xpathExpr xmlfile" << endl;
	return 1;
  }
  InitGlobalTokenTable();
  try {
	IDfaFilter *g_pfilter = NULL;
	CreateDfaFilter(&IID_IDfaFilter, (void**)&g_pfilter);
	Variable * g_pvRoot = g_pfilter->RegisterQuery(NULL, argv[1], true);

    // create a file stream object for this file
    IFileStream *pstm = _CreateFileStream(argv[2]);
	if (pstm == 0) return 1;
	ITSAXContentHandler *pchOut = CreateStdoutStream(false);
    // create the handler object
	myHandler *handler = new myHandler(pchOut);
	IUnknownCL_SetHandler(g_pfilter, handler);

	// parse
	ParseUnknownStream(pstm, g_pfilter);

	// clean-up
	pchOut->Release();
	pstm->Release();
	ATOMICRELEASE(g_pfilter);
  }
  catch (_Error &err) {
	err.perror();
  }
  CleanupGlobalTokenTable();
}

