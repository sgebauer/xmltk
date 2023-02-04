#include "xmltk.h"
#include "tokenmap.h"

//
// type filter in/out objects with dfa filter object in between 
// form a composite sandwich.   an application using type filter
// only sees one object (the CTypeFilterIn).
//


class CTypeFilterOut : public IFilteredTSAXHandler
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
    CL_STDMETHOD(cdata) (char *pszChars, int cchChars);
    CL_STDMETHOD(extendedint) (XTOKEN xt, int iInt);

    // *** IFilteredTSAXHandler methods ***
    CL_STDMETHOD(startContext) (Variable *pv);
    CL_STDMETHOD(endContext) (Variable *pv);

    // *** ISetHandler methods ***
    CL_STDMETHOD(SetHandler) (IUnknownCL *punk);
    
    CTypeFilterOut() : m_cRef(1), m_pch(NULL), m_xtContext(XT_UNKNOWN)
                     , m_iContextDepth(0)
    {
    }

private:

    virtual ~CTypeFilterOut()
    {
        ATOMICRELEASE(m_pch);
    }

    ULONG m_cRef;
    ITSAXContentHandler *m_pch;
    XTOKEN m_xtContext;
    int m_iContextDepth;
};

class CTypeFilterIn : public ITypeFilter
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
    CL_STDMETHOD(cdata) (char *pszChars, int cchChars);
    CL_STDMETHOD(extendedint) (XTOKEN xt, int iInt);

    // *** ITypeFilter methods ***
    CL_STDMETHOD(RegisterType) (THIS_ char *pszType);
    
    // *** ISetHandler methods ***
    CL_STDMETHOD(SetHandler) (IUnknownCL *punk);

    CTypeFilterIn() : m_cRef(1)
    {
        CreateDfaFilter(&IID_IDfaFilter, (void**)&m_pdfa);
        m_pdfa->QueryInterface(&IID_ITSAXContentHandler, (void**)&m_pch);
        
        m_pvRoot = m_pdfa->RegisterQuery(NULL, "/", true);
        m_pvRoot->setlParam(XT_UNKNOWN);
        
        CTypeFilterOut *pfOut = new CTypeFilterOut();
        if (pfOut)
        {
            pfOut->QueryInterface(&IID_ISetHandler, (void**)&m_pshOut);
            IUnknownCL_SetHandler(m_pdfa, (ITSAXContentHandler *)m_pshOut);
            pfOut->Release();
        }
    }
    
private:

    virtual ~CTypeFilterIn()
    {
        ATOMICRELEASE(m_pdfa);
        ATOMICRELEASE(m_pch);
        ATOMICRELEASE(m_pshOut);
    }
    
    ULONG m_cRef;
    IDfaFilter *m_pdfa;
    ITSAXContentHandler *m_pch;
    ISetHandler *m_pshOut;
    Variable *m_pvRoot;
};

bool CTypeFilterIn::QueryInterface(RCLIID riid, void **ppvObj)
{
    if (IsEqualCLIID(riid, &IID_IUnknownCL) || 
        IsEqualCLIID(riid, &IID_ITSAXContentHandler) ||
        IsEqualCLIID(riid, &IID_ITypeFilter))
    {
        *ppvObj = static_cast<ITSAXContentHandler*>(this);
    }
    else if (IsEqualCLIID(riid, &IID_ISetHandler))
    {
        *ppvObj = static_cast<ISetHandler*>(this);
    }
    else
    {
        return false;
    }
    AddRef();
    return true;
}

ULONG CTypeFilterIn::AddRef()
{
    return ++m_cRef;
}

ULONG CTypeFilterIn::Release()
{
    --m_cRef;
    if (m_cRef > 0)
    {
        return m_cRef;
    }
    delete this;
    return 0;
}

// *** ITSAXContentHandler methods ***
bool CTypeFilterIn::extendedint(XTOKEN xt, int iInt)
{
    return m_pch->extendedint(xt, iInt);
}

bool CTypeFilterIn::endDocument()
{
    return m_pch->endDocument();
}

bool CTypeFilterIn::startDocument()
{
    return m_pch->startDocument();
}

bool CTypeFilterIn::endElement(XTOKEN xtName)
{
    return m_pch->endElement(xtName);
}

bool CTypeFilterIn::startElement(XTOKEN xtName)
{
    return m_pch->startElement(xtName);
}

bool CTypeFilterIn::attribute(XTOKEN xtName, char *pszChars, int cchChars)
{
    return m_pch->attribute(xtName, pszChars, cchChars);
}

bool CTypeFilterIn::cdata(char *pszChars, int cchChars)
{
    return m_pch->cdata(pszChars, cchChars);
}

bool CTypeFilterIn::characters(char *pszChars, int cchChars)
{
    return m_pch->characters(pszChars, cchChars);
}

// *** ITypeFilter methods ***
bool CTypeFilterIn::RegisterType(char *psz)
{
    bool bSuccess = false;
    
    //
    // parse form '//element:prefix%isuffix
    //

    //
    // identify xpath expression
    //
    char *pszXPath = psz;

    psz = strchr(psz, ':');
    if (psz)
    {
        *psz = 0;

        char *pszPrefix = ++psz;

        //
        // identify prefix
        //
        psz = strchr(psz, '%');
        if (psz)
        {
            *psz = 0;
            if (*(++psz) == 'i')
            {
                //
                // identify suffix
                //
                char *pszSuffix = ++psz;

                while (*psz && !isspace(*psz))
                {
                    psz++;
                }

                if (psz)
                {
                    *psz = 0;

                    //
                    // okay, we're ready to rock
                    //
                    XTOKEN xt = g_ptt->XTOKENFromPair(pszPrefix, pszSuffix, XST_EXTENDEDINT);
                    Variable *pv = m_pdfa->RegisterQuery(m_pvRoot, pszXPath, true);
                    pv->setlParam(xt);
                    bSuccess = true;
                }
            }
        }
    }

    return bSuccess;
}

// *** ISetHandler methods
bool CTypeFilterIn::SetHandler(IUnknownCL *punk)
{
    return m_pshOut->SetHandler(punk);
}





// *** IUnknownCL methods ***
bool CTypeFilterOut::QueryInterface(RCLIID riid, void **ppvObj)
{
    if (IsEqualCLIID(riid, &IID_IUnknownCL) || 
        IsEqualCLIID(riid, &IID_ITSAXContentHandler) ||
        IsEqualCLIID(riid, &IID_IFilteredTSAXHandler))
    {
        *ppvObj = static_cast<IFilteredTSAXHandler*>(this);
    }
    else if (IsEqualCLIID(riid, &IID_ISetHandler))
    {
        *ppvObj = static_cast<ISetHandler*>(this);
    }
    else
    {
        return false;
    }
    AddRef();
    return true;
}

ULONG CTypeFilterOut::AddRef()
{
    return ++m_cRef;
}

ULONG CTypeFilterOut::Release()
{
    --m_cRef;
    if (m_cRef > 0)
    {
        return m_cRef;
    }
    delete this;
    return 0;
}

// *** ITSAXContentHandler methods ***
bool CTypeFilterOut::characters(char *pszChars, int cchChars)
{
    bool fSendCharacters = true;
    
    if (m_xtContext != XT_UNKNOWN && m_iContextDepth == 1)
    {
        //
        // this is the identified element.  see if the template matches. 
        //
        STRPAIR *ppair = g_ptt->PairFromXTOKEN(m_xtContext);
        if (ppair)
        {
            char *psz = pszChars;

            //
            // see if the prefix matches
            //
            int iLen = strlen(ppair->first);
            if (strncmp(psz, ppair->first, iLen) == 0)
            {
                //
                // yes.  skip past any digits.
                //
                psz += iLen;
                char *pszInt = psz;
                while (isdigit(*psz))
                {
                    psz++;
                }

                //
                // see if there were any digits and if the suffix matches
                //
                if (psz != pszInt &&
                    strcmp(psz, ppair->second) == 0)
                {
                    //
                    // yes.  extract the int value.
                    //
                    char ch = *psz;
                    *psz = 0;
                    int iInt = atoi(pszInt);
                    *psz = ch;
                    
                    m_pch->extendedint(m_xtContext, iInt);
                    fSendCharacters = false;
                }
            }
        }
    }
    
    if (fSendCharacters)
        m_pch->characters(pszChars, cchChars);
    
    return true;
}

bool CTypeFilterOut::extendedint(XTOKEN xt, int iInt)
{
    return m_pch->extendedint(xt, iInt);
}

bool CTypeFilterOut::endDocument()
{
    return m_pch->endDocument();
}

bool CTypeFilterOut::startDocument()
{
    return m_pch->startDocument();
}

bool CTypeFilterOut::endElement(XTOKEN xtName)
{
    if (m_xtContext != XT_UNKNOWN)
        m_iContextDepth--;

    return m_pch->endElement(xtName);
}

bool CTypeFilterOut::startElement(XTOKEN xtName)
{
    if (m_xtContext != XT_UNKNOWN)
        m_iContextDepth++;
    
    return m_pch->startElement(xtName);
}

bool CTypeFilterOut::attribute(XTOKEN xtName, char *pszChars, int cchChars)
{
    return m_pch->attribute(xtName, pszChars, cchChars);
}

bool CTypeFilterOut::cdata(char *pszChars, int cchChars)
{
    return m_pch->cdata(pszChars, cchChars);
}

// *** IFilteredTSAXHandler methods ***
bool CTypeFilterOut::startContext(Variable *pv)
{
    m_xtContext = pv->getlParam();
    return true;
}

bool CTypeFilterOut::endContext(Variable *pv)
{
    m_xtContext = XT_UNKNOWN;
    return true;
}

// *** ISetHandler methods ***
bool CTypeFilterOut::SetHandler(IUnknownCL *punk)
{
    ATOMICRELEASE(m_pch);
    return punk->QueryInterface(&IID_ITSAXContentHandler, (void**)&m_pch);
}



bool CreateTypeFilter(RCLIID riid, void **ppvObj)
{
    bool bResult = false;
    
    CTypeFilterIn *ptf = new CTypeFilterIn();
    if (ptf)
    {
        bResult = ptf->QueryInterface(riid, ppvObj);
        ptf->Release();
    }

    return bResult;
}
