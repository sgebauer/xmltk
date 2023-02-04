#include "xmltk.h"
unsigned int elementCount;

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

    myHandler()
    {
        elementCount = 0;
        m_cRef = 1;
    }

private:
    virtual ~myHandler()
    {
        cout << "startElement count= " << elementCount << endl;
    }

    unsigned int m_cRef;
};

// *** IUnknown methods ***
bool myHandler::QueryInterface(RCLIID riid, void **ppvObj)
{
    if (IsEqualCLIID(riid, &IID_IUnknownCL) ||
        IsEqualCLIID(riid, &IID_ITSAXContentHandler) ||
        IsEqualCLIID(riid, &IID_IFilteredTSAXHandler)){
        *ppvObj = static_cast<ITSAXContentHandler*>(this);
    }
    else {
        *ppvObj = NULL;
        return false;
    }
    return true;
}

ULONG myHandler::AddRef(){
  return 0;
}

ULONG myHandler::Release(){
  if (m_cRef > 0)
  {
      m_cRef--;
      return m_cRef;
  }
  delete this;
  return 0;
}

// *** ITSAXContentHandler methods ***
bool myHandler::startDocument(){
    return true;
}

bool myHandler::endDocument(){
    return true;
}

bool myHandler::startElement(XTOKEN xtName){
  elementCount++;
  return true;
}

bool myHandler::endElement(XTOKEN xtName){
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

void usage()
{
    printf("usage: xparse FILE\n");
}

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
      usage();
      return 1;
  }

  InitGlobalTokenTable();
  {
    IParse2TSAX *pparseXML, *pparseBin;
    CreateXML2TSAX(&IID_IParse2TSAX, (void**)&pparseXML);
    CreateBin2TSAX(&IID_IParse2TSAX, (void**)&pparseBin);

    IFileStream *pfs;
    CreateFileStream(&IID_IFileStream, (void**)&pfs);
    ITSAXContentHandler *pch = new myHandler();

    if (pparseXML && pparseBin)
      {
	//
	// use specified files as input
	//
	if (pfs->OpenFile(argv[1], "r"))
	  {
	    if (!pparseBin->Parse(pfs, pch)) pparseXML->Parse(pfs, pch);
	    pfs->CloseFile();
	  }
	else
	  {
	    fprintf(stderr, "error opening %s for read\n", argv[1]);
	  }
      }
    if (pparseXML)
      pparseXML->Release();
    if (pparseBin)
      pparseBin->Release();

    pch->Release();
  }

  CleanupGlobalTokenTable();

  return 0;
}
