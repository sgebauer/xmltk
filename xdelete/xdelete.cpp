#include "xmltk.h"

Variable *g_pvRoot;

inline void _Assign(IUnknownCL **ppunk, IUnknownCL *punk)
{
    if (ppunk)
	(*ppunk)->Release();
    *ppunk = punk;
    if (punk)
	punk->AddRef();
}


class CDeleteHandler : public IFilteredTSAXHandler
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

	CDeleteHandler(ITSAXContentHandler *pchOut)
	{
	    m_cRef = 1;
	    m_pchOut = pchOut;
	    m_pchOut->AddRef();
	    m_pchCur = pchOut;
	    m_pchCur->AddRef();

	    m_ContextDepth = 0;
	    m_bSkipping = false;
	}

    private:
	virtual ~CDeleteHandler()
	{
	    ATOMICRELEASE(m_pchOut);
	    ATOMICRELEASE(m_pchCur);
	}

	void _OnOpenElement(Variable *pv)
	{
	    m_bSkipping = true;
	}

	void _OnCloseElement()
	{
	    m_bSkipping = false;
	}

	ITSAXContentHandler *m_pchOut;
	ITSAXContentHandler *m_pchCur;
	ULONG m_cRef;

	bool m_bSkipping;
	int m_ContextDepth;
};


// *** ITSAXContentHandler methods ***
bool CDeleteHandler::startDocument()
{
    m_pchCur->startDocument();
    return true;
}

bool CDeleteHandler::endDocument()
{
    m_pchCur->endDocument();
    return true;
}

bool CDeleteHandler::startElement(XTOKEN xtName)
{
    if (m_bSkipping)
	return true;

#if defined(XMLTK_OUTPUT)	
    cerr << "startElement(" << g_ptt->StrFromXTOKEN(xtName) << ")" << endl;
#else
    m_pchCur->startElement(xtName);
#endif
    return true;
}

bool CDeleteHandler::endElement(XTOKEN xtName)
{
    if (m_bSkipping)
	return true;

#if defined(XMLTK_OUTPUT)	
    cerr << "endElement(" << g_ptt->StrFromXTOKEN(xtName) << ")" << endl;
#else
    m_pchCur->endElement(xtName);
#endif
    return true;
}

bool CDeleteHandler::attribute(XTOKEN xtName, char *pszChars, int cchChars)
{
    if (m_bSkipping)
	return true;

#if defined(XMLTK_OUTPUT)	
    cerr << "attribute(" << g_ptt->StrFromXTOKEN(xtName) << ")" << endl;
#else
    m_pchCur->attribute(xtName, pszChars, cchChars);
#endif
    return true;
}

bool CDeleteHandler::characters(char *pszChars, int cchChars)
{
    if (m_bSkipping)
	return true;

#if defined(XMLTK_OUTPUT)
    cerr << "characters()" << endl;
#else
    m_pchCur->characters(pszChars, cchChars);
#endif
    return true;
}

bool CDeleteHandler::cdata(char *pszChars, int cchChars)
{
    if (m_bSkipping)
	return true;

    m_pchCur->cdata(pszChars, cchChars);
    return true;
}

bool CDeleteHandler::extendedint(XTOKEN xt, int iInt)
{
    if (m_bSkipping)
	return true;

    m_pchCur->extendedint(xt, iInt);
    return true;
}


// *** IFilteredTSAXHandler methods ***
bool CDeleteHandler::startContext(Variable *pv)
{
#if defined(XMLTK_OUTPUT)	
    cerr << "startContext(" << pv << ")" << endl;
#endif

    switch (GetVariableDepth(pv))
    {
	case 1:
	    if (pv != g_pvRoot)
	    {
		if (m_ContextDepth == 0)
		{
		    _OnOpenElement(pv);
		}
		m_ContextDepth++;
	    }
	    break;
	case 2:
	    break;
	default:
	    break;
    }

    return true;
}


bool CDeleteHandler::endContext(Variable *pv)
{
#if defined(XMLTK_OUTPUT)	
    cerr << "endContext(" << pv << ")" << endl;
#endif

    switch (GetVariableDepth(pv))
    {
	case 1:
	    m_ContextDepth--;
	    if (m_ContextDepth == 0)
	    {
		_OnCloseElement();
	    }
	    break;
	case 2:
	    break;
	default:
	    break;
    }

    return true;
}


// *** IUnknown methods ***
bool CDeleteHandler::QueryInterface(RCLIID riid, void **ppvObj)
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

ULONG CDeleteHandler::AddRef()
{
    return ++m_cRef;
}

ULONG CDeleteHandler::Release()
{
    if (--m_cRef > 0)
    {
	return m_cRef;
    }

    delete this;
    return 0;
}



// global variables shared with cmdline.l
//
char *g_pszProgName;
char **g_targv;
char **g_arglim;
bool g_bBinary = false;

// global variables used here only
//
IDfaFilter *g_pfilter = NULL;
Variable *g_pvElement = NULL;
char *g_pszFileName = NULL;

void _UseElement(char *psz)
{
    g_pvElement = g_pfilter->RegisterQuery(g_pvRoot, psz, true);
}

void _UseFile(char *psz)
{
    g_pszFileName = strdup(psz);
}

void _DeleteFile()
{
    IFileStream *pstm = _CreateFileStream(g_pszFileName);
    if (pstm)
    {
	ITSAXContentHandler *pchOut = CreateStdoutStream(g_bBinary);
	if (pchOut)
	{
	    CDeleteHandler *pdelete = new CDeleteHandler(pchOut);
	    if (pdelete)
	    {
		IUnknownCL_SetHandler(g_pfilter, pdelete);
		ParseUnknownStream(pstm, g_pfilter);

		pdelete->Release();
	    }
	    pchOut->Release();
	}
	pstm->Release();
    }
}

extern int mylex();

int main(int argc, char* argv[])
{
    InitGlobalTokenTable();

    if (CreateDfaFilter(&IID_IDfaFilter, (void**)&g_pfilter))
    {
	g_pszProgName = *argv;
	g_targv = argv + 1;
	g_arglim = argv + argc;

	try
	{
	    g_pvRoot = g_pfilter->RegisterQuery(NULL, "/", true);
	    
	    // mylex drives everything via callbacks
	    //
	    mylex();

	    _DeleteFile();
	}
	catch (_Error &err)
	{
	    err.perror();
	}

	ATOMICRELEASE(g_pfilter);

	if (g_pszFileName)
	    free(g_pszFileName);
    }

    CleanupGlobalTokenTable();
}
