#include "xmltk.h"
#include "SIndex.hpp"

unsigned int elementCount = 0;
unsigned int elementCurrentDepth = 0;
unsigned int elementDepthMax = 0;
unsigned int threshHold = 0;
char * xmlFile;
SIndex * ic;

class statHandler : public ITSAXContentHandler
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

    statHandler(): m_cRef(1){
	}
    virtual ~statHandler(){
	}

    ULONG m_cRef;
};

// *** IUnknown methods ***
bool statHandler::QueryInterface(RCLIID riid, void **ppvObj)
{
    if (IsEqualCLIID(riid, &IID_IUnknownCL) ||
        IsEqualCLIID(riid, &IID_ITSAXContentHandler)){
        *ppvObj = static_cast<ITSAXContentHandler*>(this);
    }
    else {
        *ppvObj = NULL;
        return false;
    }
    AddRef();
    return true;
}

ULONG statHandler::AddRef(){
    return ++m_cRef;
}

ULONG statHandler::Release(){
    if (--m_cRef > 0)
    {
        return m_cRef;
    }
    delete this;
    return 0;
}

// *** ITSAXContentHandler methods ***
bool statHandler::startDocument(){
  elementCount = 0;
  elementCurrentDepth = 0;
  elementDepthMax = 0;
  return true;
}

bool statHandler::endDocument(){
  return true;
}

bool statHandler::startElement(XTOKEN xtName){
  elementCount++;
  elementCurrentDepth++;
  if (elementCurrentDepth > elementDepthMax)
	elementDepthMax = elementCurrentDepth;
  return true;
}

bool statHandler::endElement(XTOKEN xtName){
  elementCurrentDepth--;
  return true;
}

bool statHandler::attribute(XTOKEN xtName, char *pszChars, int cchChars){
  return true;
}

bool statHandler::characters(char *pszChars, int cchChars){
  return true;
}

bool statHandler::cdata(char *pszChars, int cchChars){
  return true;
}

bool statHandler::extendedint(XTOKEN xt, int iInt){
  return true;
}


class createSindexHandler : public ITSAXContentHandler
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

    createSindexHandler() : m_cRef(1){
	}
    virtual ~createSindexHandler(){
	}

private:
    ULONG m_cRef;
};

// *** IUnknown methods ***
bool createSindexHandler::QueryInterface(RCLIID riid, void **ppvObj)
{
    if (IsEqualCLIID(riid, &IID_IUnknownCL) ||
        IsEqualCLIID(riid, &IID_ITSAXContentHandler)){
        *ppvObj = static_cast<ITSAXContentHandler*>(this);
    }
    else {
        *ppvObj = NULL;
        return false;
    }
    AddRef();
    return true;
}

ULONG createSindexHandler::AddRef(){
    return ++m_cRef;
}

ULONG createSindexHandler::Release(){
    if (--m_cRef > 0)
    {
        return m_cRef;
    }
    delete this;
    return 0;
}

// *** ITSAXContentHandler methods ***
bool createSindexHandler::startDocument(){
  return true;
}

bool createSindexHandler::endDocument(){
  return true;
}

bool createSindexHandler::startElement(XTOKEN xtName){
  ic->startElement(g_xmlparse->getSrcOffset());
  return true;
}

bool createSindexHandler::endElement(XTOKEN xtName){
  ic->endElement(g_xmlparse->getSrcOffset());
  return true;
}

bool createSindexHandler::attribute(XTOKEN xtName, char *pszChars, int cchChars){
  return true;
}

bool createSindexHandler::characters(char *pszChars, int cchChars){
  return true;
}

bool createSindexHandler::cdata(char *pszChars, int cchChars){
  return true;
}

bool createSindexHandler::extendedint(XTOKEN xt, int iInt){
  return true;
}

void Usage(char * com){
  cout << "Usage: " << com << " [-t threshHold] xmlfile;" << endl;
  //  cout << "       stdin | " << com << " [-t threshHold];" << endl;
}


int main(int argc, char* argv[]){
  /*
  if (argc == 1){
	xmlFile = 0;
  }
  else */ if (argc == 2){
	if (!strcmp(argv[1], "-h")){
	  Usage(argv[0]);
	  return 1;
	}
	xmlFile = argv[1];
  }
  /*
  else if (argc == 3 && !strcmp(argv[1], "-t")){	// threshHold is specified
	threshHold = atoi(argv[2]);
	xmlFile = 0;
  }
  */
  else if (argc == 4 && !strcmp(argv[1], "-t")){	// threshHold is specified
	threshHold = atoi(argv[2]);
	xmlFile = argv[3];
  }
  else {
	Usage(argv[0]);
	return 1;
  }

  InitGlobalTokenTable();
  try {
	ITypeFilter *g_ptype = NULL;
    // create a file stream object for this file
	IFileStream *pstm;
	if (pstm == 0) return 1;
								// 1st phase: collect statistics of XML data
	pstm = _CreateFileStream(xmlFile);
	CreateTypeFilter(&IID_ITypeFilter, (void**)&g_ptype);
    // create a handler for statistics
	statHandler *handler = new statHandler();
	IUnknownCL_SetHandler(g_ptype, handler);
	// parse 
	ParseUnknownStream(pstm, g_ptype);
	pstm->Release();
	ATOMICRELEASE(g_ptype);

								// 2nd phase: create streaming index
	ic = new SIndex(xmlFile, threshHold);
	ic->setiTable(elementCount,0);
	ic->setiPath(elementDepthMax,0);
	ic->init();
	pstm = _CreateFileStream(xmlFile);
	CreateTypeFilter(&IID_ITypeFilter, (void**)&g_ptype);
    // create the streaming index object
	createSindexHandler *handler2 = new createSindexHandler();
	IUnknownCL_SetHandler(g_ptype, handler2);
	// parse 
	ParseUnknownStream(pstm, g_ptype);
	ic->writeFile();
	// clean-up
	pstm->Release();
	ATOMICRELEASE(g_ptype);
	delete ic;
  }
  catch (_Error &err) {
	err.perror();
  }
  CleanupGlobalTokenTable();
}

