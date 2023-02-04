#include "xmltk.h"

Variable *g_pvRoot;

// global variable shared with cmdline.l
bool g_bRecursive = false;

inline void _Assign(IUnknownCL **ppunk, IUnknownCL *punk)
{
    if (ppunk)
	(*ppunk)->Release();
    *ppunk = punk;
    if (punk)
	punk->AddRef();
}


class CFlattenHandler : public IFilteredTSAXHandler
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

	CFlattenHandler(ITSAXContentHandler *pchOut)
	{
	    m_bOpenTagSkipped = false;
	    m_bCloseTagAvailable = false;
	    m_bSkipOpenTag = m_bSkipCloseTag = false;
	    m_cRef = 1;
	    m_pchOut = pchOut;
	    m_pchOut->AddRef();
	    m_pchCur = pchOut;
	    m_pchCur->AddRef();

	    m_ContextDepth = 0;
	}

    private:
	virtual ~CFlattenHandler()
	{
	    ATOMICRELEASE(m_pchOut);
	    ATOMICRELEASE(m_pchCur);
	}

	void _OnOpenElement(Variable *pv)
	{
	    m_bSkipOpenTag = true;
	}

	void _OnCloseElement()
	{
	    m_bSkipCloseTag = true;
	}

	bool m_bSkipOpenTag;
	bool m_bSkipCloseTag;
	bool m_bOpenTagSkipped;
	bool m_bCloseTagAvailable;

	int m_ContextDepth;

	ITSAXContentHandler *m_pchOut;
	ITSAXContentHandler *m_pchCur;
	ULONG m_cRef;

	XTOKEN m_xtoken;
};


// *** ITSAXContentHandler methods ***
bool CFlattenHandler::startDocument()
{
    m_pchCur->startDocument();
    return true;
}

bool CFlattenHandler::endDocument()
{
    if (m_bCloseTagAvailable)
    {
	m_bCloseTagAvailable = false;
	if (! m_bSkipCloseTag)
	{
#if defined(XMLTK_OUTPUT)	
	    cerr << "endElement(" << g_ptt->StrFromXTOKEN(m_xtoken) << ")" << endl;
#else
	    m_pchCur->endElement(m_xtoken);
#endif
	}
	else
	{
	    m_bSkipCloseTag = false;
	}
    }
    m_pchCur->endDocument();
    return true;
}

bool CFlattenHandler::startElement(XTOKEN xtName)
{
    if (m_bCloseTagAvailable)
    {
	m_bCloseTagAvailable = false;
	if (! m_bSkipCloseTag)
	{
#if defined(XMLTK_OUTPUT)	
	    cerr << "endElement(" << g_ptt->StrFromXTOKEN(m_xtoken) << ")" << endl;
#else
	    m_pchCur->endElement(m_xtoken);
#endif
	}
	else
	{
	    // skipping the close tag
	    //
	    m_bSkipCloseTag = false;
	}
    }

    if (!m_bSkipOpenTag || m_bOpenTagSkipped)
    {
#if defined(XMLTK_OUTPUT)	
	cerr << "startElement(" << g_ptt->StrFromXTOKEN(xtName) << ")" << endl;
#else
	m_pchCur->startElement(xtName); 
#endif
    }

    if (m_bSkipOpenTag)
    {
	if (m_bOpenTagSkipped)
	{
	    m_bSkipOpenTag = false;
	    m_bOpenTagSkipped = false;
	}
	else
	{
	    m_bOpenTagSkipped = true;
	}
    }

    return true;
}

bool CFlattenHandler::endElement(XTOKEN xtName)
{
    if (m_bCloseTagAvailable)
    {
	m_bCloseTagAvailable = false;
	if (! m_bSkipCloseTag)
	{
#if defined(XMLTK_OUTPUT)	
	    cerr << "endElement(" << g_ptt->StrFromXTOKEN(m_xtoken) << ")" << endl;
#else
	    m_pchCur->endElement(m_xtoken);
#endif
	}
	else
	{
	    // skipping the close tag
	    //
	    m_bSkipCloseTag = false;
	}
    }
    
    m_xtoken = xtName;
    m_bCloseTagAvailable = true;
    return true;
}

bool CFlattenHandler::attribute(XTOKEN xtName, char *pszChars, int cchChars)
{
    if (!m_bSkipOpenTag)
    {
#if defined(XMLTK_OUTPUT)	
	cerr << "attribute(" << g_ptt->StrFromXTOKEN(xtName) << ")" << endl;
#else
	m_pchCur->attribute(xtName, pszChars, cchChars);
#endif
    }
    return true;
}

bool CFlattenHandler::characters(char *pszChars, int cchChars)
{
#if defined(XMLTK_OUTPUT)
    cerr << "characters()" << endl;
#else
    m_pchCur->characters(pszChars, cchChars);
#endif
    return true;
}

bool CFlattenHandler::cdata(char *pszChars, int cchChars)
{
    m_pchCur->cdata(pszChars, cchChars);
    return true;
}

bool CFlattenHandler::extendedint(XTOKEN xt, int iInt)
{
    m_pchCur->extendedint(xt, iInt);
    return true;
}


// *** IFilteredTSAXHandler methods ***
bool CFlattenHandler::startContext(Variable *pv)
{
#if defined(XMLTK_OUTPUT)	
    cerr << "startContext(" << pv << ")" << endl;
#endif
    switch (GetVariableDepth(pv))
    {
	case 1:
	    if (pv != g_pvRoot)
	    {
		if (g_bRecursive || m_ContextDepth == 0)
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


bool CFlattenHandler::endContext(Variable *pv)
{
#if defined(XMLTK_OUTPUT)	
    cerr << "endContext(" << pv << ")" << endl;
#endif

    switch (GetVariableDepth(pv))
    {
	case 1:
	    m_ContextDepth--;
	    if (g_bRecursive || m_ContextDepth == 0)
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
bool CFlattenHandler::QueryInterface(RCLIID riid, void **ppvObj)
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

ULONG CFlattenHandler::AddRef()
{
    return ++m_cRef;
}

ULONG CFlattenHandler::Release()
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

void _FlattenFile()
{
    IFileStream *pstm = _CreateFileStream(g_pszFileName);
    if (pstm)
    {
	ITSAXContentHandler *pchOut = CreateStdoutStream(g_bBinary);
	if (pchOut)
	{
	    CFlattenHandler *ptail = new CFlattenHandler(pchOut);
	    if (ptail)
	    {
		IUnknownCL_SetHandler(g_pfilter, ptail);
		ParseUnknownStream(pstm, g_pfilter);

		ptail->Release();
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

	    _FlattenFile();
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
