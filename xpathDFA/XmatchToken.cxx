#include <xmltk.h>
#include "Error.h"

unsigned int outputLevel; // for checking output state and skip state
unsigned int elementCount;
unsigned int skipCount;
unsigned int skipMax;
unsigned int skipSum;

class CSortHandler : public IFilteredTSAXHandler
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

  CSortHandler(void) : m_cRef(1){}
  virtual ~CSortHandler() {}

  ULONG m_cRef;
};

// *** IUnknown methods ***
bool CSortHandler::QueryInterface(RCLIID riid, void **ppvObj)
{
    if (IsEqualCLIID(riid, &IID_IUnknownCL) ||
        IsEqualCLIID(riid, &IID_ITSAXContentHandler) ||
        IsEqualCLIID(riid, &IID_IFilteredTSAXHandler))
    {
        *ppvObj = static_cast<IFilteredTSAXHandler*>(this);
    }
    else
    {
        *ppvObj = NULL;
        return false;
    }
    
    AddRef();
    return true;
}

ULONG CSortHandler::AddRef()
{
    return ++m_cRef;
}

ULONG CSortHandler::Release()
{
    if (--m_cRef > 0)
    {
        return m_cRef;
    }
    delete this;
    return 0;
}

// *** ITSAXContentHandler methods ***
bool CSortHandler::startDocument() {
  outputLevel = 0;
  elementCount = 0;
  skipCount = 0;
  skipMax = 0;
  skipSum = 0;
#if defined(XMLTK_OUTPUT)
  cout << "<root>" << endl;
#endif
  return true;
}

bool CSortHandler::endDocument(){
#if defined(XMLTK_OUTPUT)
  cout << "</root>" << endl;
#endif
  cout << "startElement count= " << elementCount << ",";
  cout << "skip count= " << skipCount << ",";
  cout << "max skip= " << skipMax  << ",";
  cout << "avg skip= " << (float)skipSum/skipCount << endl;
  return true;
}

bool CSortHandler::startElement(XTOKEN xtName){
#if defined(XMLTK_OUTPUT)
  cout << "<" << g_ptt->StrFromXTOKEN(xtName) << ">";
#endif
  elementCount++;
  return true;
}

bool CSortHandler::endElement(XTOKEN xtName){
#if defined(XMLTK_OUTPUT)
  cout << "</>" << endl;
#endif
  return true;
}

bool CSortHandler::attribute(XTOKEN xtName, char *pszChars, int cchChars){
  return true;
}

bool CSortHandler::characters(char *pszChars, int cchChars){
#if defined(XMLTK_OUTPUT)
  char * s = (char *)malloc(cchChars+1);
  strncpy(s, pszChars, cchChars);
  s[cchChars] = '\0';
  cout << s;
  free(s);
#endif
  return true;
}

bool CSortHandler::cdata(char *pszChars, int cchChars){
    return true;
}

bool CSortHandler::extendedint(XTOKEN xt, int iInt){
    return true;
}

// *** IFilteredTSAXHandler methods ***
bool CSortHandler::startContext(Variable *pv){
    return true;
}

bool CSortHandler::endContext(Variable *pv){
    return true;
}

static void usage()
{
    fprintf(stderr, "usage: xmatch xmlFile queryFile\n");
}

int main(int argc, char* argv[])
{
    if (argc == 3)
    {
        InitGlobalTokenTable();
        
	    char *xmlFile = argv[1];
	    char *queryFile = argv[2];
        
        IParse2TSAX *pparser;
        if (CreateXML2TSAX(&IID_IParse2TSAX, (void**)&pparser))
        {
            IDfaFilter *pfilter;
			if (CreateDfaFilter(&IID_IDfaFilter, (void**)&pfilter, pparser, xmlFile))
			//            if (CreateDfaFilter(&IID_IDfaFilter, (void**)&pfilter))
            {  
                try
                {
				  pfilter->RegisterQueryFile(queryFile);
				  CSortHandler *psort = new CSortHandler();
				  if (psort) {
					IUnknownCL_SetHandler(pfilter, psort);
					IFileStream* pstm;
					if (CreateFileStream(&IID_IFileStream, (void**)&pstm)){
					  pstm->OpenFile(xmlFile, "r");
					  pparser->Parse(pstm, static_cast<ITSAXContentHandler*>(pfilter));
					}
					psort->Release();
				  }
				  pfilter->Release();
                }
                catch (_Error &err){
				  err.perror();
                }
            }
            pparser->Release();
        }

        CleanupGlobalTokenTable();
    }
    else
    {
        usage();
        return 1;
    }
 
    return 0;
}

