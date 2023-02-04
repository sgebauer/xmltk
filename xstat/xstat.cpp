#include "xmltk.h"

unsigned int elementCount = 0;
unsigned int elementCurrentDepth = 0;
unsigned int elementDepthMax = 0;
unsigned int elementDepthSum = 0;
char * xmlFile;

class myHandler : public ITSAXContentHandler
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

    myHandler(): m_cRef(1) {
	}
    virtual ~myHandler(){
	}

    ULONG m_cRef;
};

// *** IUnknown methods ***
bool myHandler::QueryInterface(RCLIID riid, void **ppvObj)
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
  elementCount = 0;
  elementCurrentDepth = 0;
  elementDepthMax = 0;
  elementDepthSum = 0;
  return true;
}

bool myHandler::endDocument(){
  cout << "<stat fileName=\"" << xmlFile << "\">" << endl;
  cout << "  <element count=\"" << elementCount << "\"/>"<< endl;
  cout << "  <depth max=\"" << elementDepthMax << "\" avg=\"" << (float)elementDepthSum/elementCount << "\"/>" << endl;
  cout << "</stat>" << endl;
  return true;
}

bool myHandler::startElement(XTOKEN xtName){
  elementCount++;
  elementDepthSum += ++elementCurrentDepth;
  if (elementCurrentDepth > elementDepthMax)
	elementDepthMax = elementCurrentDepth;
  return true;
}

bool myHandler::endElement(XTOKEN xtName){
  elementCurrentDepth--;
  return true;
}

bool myHandler::attribute(XTOKEN xtName, char *pszChars, int cchChars){
  return true;
}

bool myHandler::characters(char *pszChars, int cchChars){
  return true;
}

bool myHandler::cdata(char *pszChars, int cchChars){
  return true;
}

bool myHandler::extendedint(XTOKEN xt, int iInt){
  return true;
}


int main(int argc, char* argv[]){
  if (argc != 2){
	cout << "Usage: " << argv[0] << " xmlfile" << endl;
	return 1;
  }
  InitGlobalTokenTable();
  try {
	ITypeFilter *g_ptype = NULL;
	CreateTypeFilter(&IID_ITypeFilter, (void**)&g_ptype);

    // create a file stream object for this file
	xmlFile = argv[1];
    IFileStream *pstm = _CreateFileStream(xmlFile);
	if (pstm == 0) return 1;
    // create the sorter object
	myHandler *handler = new myHandler();
	IUnknownCL_SetHandler(g_ptype, handler);
	// parse 
	ParseUnknownStream(pstm, g_ptype);
	pstm->Release();
	ATOMICRELEASE(g_ptype);
  }
  catch (_Error &err) {
	err.perror();
  }
  CleanupGlobalTokenTable();
}

