
#include "xmltk.h"

//
// default number of elements to head/tail
//
#ifdef XHEAD
#define NUM_DEFAULT  (-10)
#else
#define NUM_DEFAULT  10  
#endif

Variable *g_pvRoot;

inline void _Assign(IUnknownCL **ppunk, IUnknownCL *punk)
{
    if (*ppunk)
        (*ppunk)->Release();
    *ppunk = punk;
    if (punk)
        punk->AddRef();
}

class CElementAccumulator
{
public:
    
    ITSAXContentHandler *GetHandlerForNewElement()
    {
        return m_bTail ? _TailGetHandler() : _HeadGetHandler();
    }

    void FlushAndReset(ITSAXContentHandler *pchOut)
    {
        m_bTail ? _TailFlush(pchOut) : _HeadFlush(pchOut);
    }
    
    void SetLimit(int iLimit)
    {
        m_bTail = iLimit >= 0;
        m_uLimit = abs(iLimit);
        m_vpos1.resize(m_uLimit);
        m_vpos2.resize(m_uLimit);
    }

    CElementAccumulator() : m_bFirstCurrent(false), m_uNext(0)
    {
        CreateFileStream(&IID_IFileStream, (void**)&m_pfs1);
        CreateFileStream(&IID_IFileStream, (void**)&m_pfs2);

        CreateTSAX2Bin(&IID_ITSAX2Bin, (void**)&m_p2bin);
        m_p2bin->Init(m_pfs1, false);

        CreateBin2TSAX(&IID_IBin2TSAX, (void**)&m_pbin2);
        m_pbin2->Init(false);

		CreateTSAX2Nil(&IID_ITSAXContentHandler, (void**)&m_p2nil);

        SetLimit(NUM_DEFAULT);
    }

private:

    void _PrepareFileStream()
    {
        //
        // make sure the file stream has a file
        //
        if (_pfsCur()->GetFile() == NULL)
        {
            _pfsCur()->SetFile(tmpfile());
        }
        m_p2bin->Init(_pfsCur(), false);
    }
    
    ITSAXContentHandler *_HeadGetHandler()
    {
        ITSAXContentHandler *pch;
        if (m_uNext < m_uLimit)
        {
            _PrepareFileStream();

            pch = m_p2bin;
            m_uNext++;
        }
        else
        {
            pch = m_p2nil;
        }
        pch->AddRef();
        return pch;
    }
    
    ITSAXContentHandler *_TailGetHandler()
    {
        _PrepareFileStream();

        //
        // mark the new starting position
        //
        (*_vposCur())[m_uNext] = _pfsCur()->Tell();
        
        //
        // update next position and flip-flop if needed
        //
        m_uNext = (m_uNext+1) % m_uLimit;
        if (m_uNext == 0)
        {
            _FlipFlop();
        }
        
        m_p2bin->AddRef();
        return m_p2bin;
    }
    
    void _HeadFlush(ITSAXContentHandler *pchOut)
    {
        if (m_uNext)
        {
            _pfsCur()->Seek(0, SEEK_SET);
        
            for (int i = 0; i < m_uNext; i++)
            {
                m_pbin2->Parse(_pfsCur(), pchOut);
            }
        }

        _pfsCur()->CloseFile();
        m_uNext = 0;
    }

    void _TailFlush(ITSAXContentHandler *pchOut)
    {
        if (_pfsPrev()->GetFile())
        {
            //
            // output elements from previous file
            //
            _pfsPrev()->Seek((*_vposPrev())[m_uNext], SEEK_SET);
            for (int i = m_uNext; i < m_uLimit; i++)
            {
                m_pbin2->Parse(_pfsPrev(), pchOut);
            }
        }

        if (_pfsCur()->GetFile())
        {
            //
            // output elements from current file
            //
            _pfsCur()->Seek(0, SEEK_SET);
            for (int i = 0; i < m_uNext; i++)
            {
                m_pbin2->Parse(_pfsCur(), pchOut);
            }
        }

        m_pfs1->CloseFile();
        m_pfs2->CloseFile();

        m_uNext = 0;
    }
        
    void _FlipFlop()
    {
        //
        // flip-flop between the two file streams
        //
        _pfsPrev()->CloseFile();
        m_bFirstCurrent = !m_bFirstCurrent;
    }

    virtual ~CElementAccumulator()
    {
        m_pfs1->CloseFile();
        m_pfs2->CloseFile();

        ATOMICRELEASE(m_pfs1);
        ATOMICRELEASE(m_pfs2);

        ATOMICRELEASE(m_p2bin);
        ATOMICRELEASE(m_pbin2);

        ATOMICRELEASE(m_p2nil);
    }
    
    vector<size_t> *_vposCur() { return m_bFirstCurrent ? &m_vpos1 : &m_vpos2; }
    vector<size_t> *_vposPrev() { return m_bFirstCurrent ? &m_vpos2 : &m_vpos1; }
    IFileStream *_pfsCur() { return m_bFirstCurrent ? m_pfs1 : m_pfs2; }
    IFileStream *_pfsPrev() { return m_bFirstCurrent ? m_pfs2 : m_pfs1; }

    IFileStream *m_pfs1;
    IFileStream *m_pfs2;
    bool m_bFirstCurrent;

    ITSAX2Bin *m_p2bin;
    IBin2TSAX *m_pbin2;
    
    unsigned int m_uLimit;
    unsigned int m_uNext;

    vector<size_t> m_vpos1;
    vector<size_t> m_vpos2;

    ITSAXContentHandler *m_p2nil;

    bool m_bTail;
};

class CTailHandler : public IFilteredTSAXHandler
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

    CTailHandler(ITSAXContentHandler *pchOut) : m_pvElement(NULL),
        m_pvContext(NULL), m_pchOut(pchOut), m_cRef(1),
        m_pchCur(pchOut)
        
    {
        m_pchOut->AddRef();
        m_pchCur->AddRef();
    }

private:
        
    void _OnOpenContext(Variable *pv)
    {
        m_pvContext = pv;
    }

    void _OnCloseContext()
    {
        //
        // go through and flush all the accumulated elements
        //
	    VarPtrList * plv = m_pvContext->getChildren();
        if (plv)
        {
		    for (VarPtrList::iterator itr = plv->begin(); itr != plv->end(); ++itr){
			    Variable *pv = *itr;
                CElementAccumulator *pec = (CElementAccumulator*)pv->getlParam();
                pec->FlushAndReset(m_pchOut);
            }
        }

        m_pvContext = NULL;
        _Assign(&static_cast<IUnknownCL*>(m_pchCur), m_pchOut);
    }

    void _OnOpenElement(Variable *pv)
    {
        m_pvElement = pv;
        CElementAccumulator *pec = (CElementAccumulator*)pv->getlParam();
        ATOMICRELEASE(m_pchCur);
        m_pchCur = pec->GetHandlerForNewElement();
    }

    void _OnCloseElement()
    {
        m_pvElement = NULL;
        _Assign(&static_cast<IUnknownCL*>(m_pchCur), m_pchOut);
    }
    
    virtual ~CTailHandler()
    {
        ATOMICRELEASE(m_pchOut);
        ATOMICRELEASE(m_pchCur);
    };

    Variable *m_pvElement;
    Variable *m_pvContext;
    
    ITSAXContentHandler *m_pchOut;
    ULONG m_cRef;
    ITSAXContentHandler *m_pchCur;
};

// *** IUnknown methods ***
bool CTailHandler::QueryInterface(RCLIID riid, void **ppvObj)
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

ULONG CTailHandler::AddRef()
{
    return ++m_cRef;
}

ULONG CTailHandler::Release()
{
    if (--m_cRef > 0)
    {
        return m_cRef;
    }
    delete this;
    return 0;
}

// *** ITSAXContentHandler methods ***
bool CTailHandler::startDocument()
{
    m_pchCur->startDocument();
    return true;
}

bool CTailHandler::endDocument()
{
    m_pchCur->endDocument();
    return true;
}

bool CTailHandler::startElement(XTOKEN xtName)
{
    m_pchCur->startElement(xtName);
    return true;
}

bool CTailHandler::endElement(XTOKEN xtName)
{
    if (m_pvContext && m_pchCur == m_pchOut)
    {
        //
        // this endElement is closing the tail context.
        //
        _OnCloseContext();
    }
    m_pchCur->endElement(xtName);
    return true;
}

bool CTailHandler::attribute(XTOKEN xtName, char *pszChars, int cchChars)
{
    m_pchCur->attribute(xtName, pszChars, cchChars);
    return true;
}

bool CTailHandler::characters(char *pszChars, int cchChars)
{
    m_pchCur->characters(pszChars, cchChars);
    return true;
}

bool CTailHandler::cdata(char *pszChars, int cchChars)
{
    m_pchCur->cdata(pszChars, cchChars);
    return true;
}

bool CTailHandler::extendedint(XTOKEN xt, int iInt)
{
    m_pchCur->extendedint(xt, iInt);
    return true;
}

// *** IFilteredTSAXHandler methods ***
bool CTailHandler::startContext(Variable *pv)
{
    switch (GetVariableDepth(pv))
    {
    case 1:
        if (pv != g_pvRoot)
        {
            _OnOpenContext(pv);
        }
        break;

    case 2:
        _OnOpenElement(pv);
        break;

    default:			// for root XPE
        break;
    }
    return true;
}
bool CTailHandler::endContext(Variable *pv)
{
    switch (GetVariableDepth(pv))
    {
    case 1:     // just closed context     
	  //        _OnCloseContext();
        break;

    case 2:     // just closed element
        _OnCloseElement();
        break;

    default:			// for root XPE
        break;
    }
    return true;
}

//
// global variables shared with cmdline.l
//
char *g_pszProgName;
char **g_targv;
char **g_arglim;
bool g_bBinary = false;

//
// global variables used here only
//
IDfaFilter *g_pfilter = NULL;
Variable *g_pvContext = NULL, *g_pvElement = NULL;
char *g_pszFileName = NULL;

void _UseContext(char *psz)
{
    g_pvContext = g_pfilter->RegisterQuery(g_pvRoot, psz, false);
    g_pvElement = NULL;
}

void _UseElement(char *psz)
{
    if (g_pvContext == NULL)
    {
        //
        // coerce context to root
        //
        _UseContext("/");
    }

    g_pvElement = g_pfilter->RegisterQuery(g_pvContext, psz, true);

    CElementAccumulator *pec = new CElementAccumulator();
    g_pvElement->setlParam((size_t)pec);
}

void _UseNumber(int i)
{
    if (g_pvElement)
    {
        CElementAccumulator *pec = (CElementAccumulator*)g_pvElement->getlParam();
#ifdef XHEAD
        pec->SetLimit(-i);
#else
        pec->SetLimit(i);
#endif
    }
    else
    {
        fprintf(stderr, "usage error - number specified with no element\n");
        exit(0);
    }
}

void _UseFile(char *psz)
{
    g_pszFileName = strdup(psz);
}

void _HeadTailFile()
{
    IFileStream *pstm = _CreateFileStream(g_pszFileName);
    if (pstm)
    {
        ITSAXContentHandler *pchOut = CreateStdoutStream(g_bBinary);
        if (pchOut)
        {
            CTailHandler *ptail = new CTailHandler(pchOut);
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
            //
            // mylex drives everything via callbacks _UseContext etc
            //
            mylex();

            _HeadTailFile();
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

