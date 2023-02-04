#include "xmltk.h"


class CTSAX2XML : public ITSAXContentHandler
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

    CTSAX2XML(FILE *pfOut, int iIndentSize) : m_cRef(1), m_pfOut(pfOut),
                                              m_iIndent(0), 
                                              m_iIndentSize(iIndentSize),
            m_bLastWasClose(false), m_bBracketOpen(false), m_bAttribute(false)
    {
        GetGlobalTokenTable(&IID_ITokenTable, (void**)&m_ptt);
    }

private:
    virtual ~CTSAX2XML()
    {
        assert(m_ptt);
        m_ptt->Release();
    }

    void _PrintIndent()
    {
        int iIndent = m_iIndent * m_iIndentSize;
        for (int i = 0; i < iIndent; i++)
        {
            fputc(' ', m_pfOut);
        }
    }

    void _CloseBracketIfNeeded()
    {
        if (m_bBracketOpen)
        {
            fputc('>', m_pfOut);
            m_bBracketOpen = false;
            m_bAttribute = false;
        }
    }

    ULONG m_cRef;
    FILE *m_pfOut;
    ITokenTable *m_ptt;

    int m_iIndent;
    int m_iIndentSize;
    bool m_bLastWasClose;
    bool m_bBracketOpen;
    bool m_bAttribute;
};


bool CTSAX2XML::QueryInterface(RCLIID riid, void **ppvObj)
{
    if (IsEqualCLIID(riid, &IID_IUnknownCL))
    {
        *ppvObj = static_cast<IUnknownCL*>(this);
    }
    else if (IsEqualCLIID(riid, &IID_ITSAXContentHandler))
    {
        *ppvObj = static_cast<ITSAXContentHandler*>(this);
    }
    else
    {
        *ppvObj = NULL;
        return false;
    }
    AddRef();
    return true;
}

ULONG CTSAX2XML::AddRef()
{
    return ++m_cRef;
}

ULONG CTSAX2XML::Release()
{
    --m_cRef;
    if (m_cRef > 0)
    {
        return m_cRef;
    }
    delete this;
    return 0;
}

bool CTSAX2XML::characters(char* pszChars, int cchChars)
{
    _CloseBracketIfNeeded();

    m_bLastWasClose = false;

    fputs(pszChars, m_pfOut);

    return true;
}

bool CTSAX2XML::cdata(char* pszChars, int cchChars)
{
    _CloseBracketIfNeeded();

    fputs("<![CDATA[", m_pfOut);
    fwrite(pszChars, cchChars * sizeof(*pszChars), 1, m_pfOut);
    fputs("]]>", m_pfOut);

    return true;
}

bool CTSAX2XML::extendedint(XTOKEN xt, int iInt)
{
    _CloseBracketIfNeeded();

    STRPAIR *ppair = g_ptt->PairFromXTOKEN(xt);
    if (ppair)
    {
        fprintf(m_pfOut, "%s%d%s", ppair->first, iInt, ppair->second);
    }
    return true;
}

bool CTSAX2XML::endDocument()
{
    fputc('\n', m_pfOut); 
    return true;
}

bool CTSAX2XML::startDocument()
{
    return true;
}

bool CTSAX2XML::endElement(XTOKEN xtName)
{
    m_iIndent--;

    if (m_bBracketOpen && !m_bAttribute)
    {
        //
        // the element tag is still open, and there were no
        // attributes, so we can output it in "<name/>" form.
        //
        m_bBracketOpen = false;
        fputs("/>", m_pfOut);
    }
    else
    {
        _CloseBracketIfNeeded();
        
        if (m_bLastWasClose)
        {
            fprintf(m_pfOut, "\n");
            _PrintIndent();
        }
        
        fprintf(m_pfOut, "</%s>", g_ptt->StrFromXTOKEN(xtName));
    }

    m_bLastWasClose = true;
    
    return true;
}

bool CTSAX2XML::startElement(XTOKEN xtName)
{
    _CloseBracketIfNeeded();

    m_bLastWasClose = false;
    m_bBracketOpen = true;

    if (m_iIndent > 0)
    {
        fprintf(m_pfOut, "\n");
        _PrintIndent();
    }
    m_iIndent++;

    fputc('<', m_pfOut);
    fputs(g_ptt->StrFromXTOKEN(xtName), m_pfOut);
    return true;
}

bool CTSAX2XML::attribute(XTOKEN xtName, char *pszChars, int cchChars)
{
    m_bAttribute = true;
    
    fprintf(m_pfOut, " %s=\"%s\"", g_ptt->StrFromXTOKEN(xtName), pszChars);
    return true;
}


bool CreateTSAX2XML(RCLIID riid, void **ppvObj, FILE *pfOut, int iIndent)
{
    bool bResult = false;

    CTSAX2XML *ptx = new CTSAX2XML(pfOut, iIndent);
    if (ptx)
    {
        bResult = ptx->QueryInterface(riid, ppvObj);
        ptx->Release();
    }

    return bResult;
}

