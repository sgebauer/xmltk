#include "xmltk.h"
#include "tokenmap.h"

class CTSAX2Bin : public ITSAX2Bin
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

    // *** ITSAX2Bin methods ***
    CL_STDMETHOD(Init) (IStream *pws, bool bEmitTableEntries);
    
    CTSAX2Bin() : m_pstm(NULL), m_cRef(1), m_xtNext(XT_FIRST), 
        m_bEmitTableEntries(true)
    {
    }

private:

    virtual ~CTSAX2Bin()
    {
        if (m_pstm)
            m_pstm->Release();
    }

    void _fputsTerm(char *psz)
    {
        m_pstm->Write(psz, strlen(psz));
        m_pstm->WriteChar(0);
    }

    XTOKEN _GetNewXTOKEN()
    {
        //
        // be sure to skip 0x1A (end-of-file character)
        //
        if (m_xtNext == 0x1A)
        {
            m_xtNext++;
        }
        return m_xtNext++;
    }

    void _OutputTableEntry(XTOKEN xt, XTOKEN xtOut)
    {
        //
        // output the table entry for this dude
        //
        // table := XT_TABLE (mapping)* XT_END
        // mapping := token tokentype termstr [ termstr ]       second termstr is present if tokentype is XST_EXTENDEDINT
        //

        m_pstm->WriteChar(XT_TABLE);
            
        fwriteMultiByteUINT(m_pstm, xtOut);

        DWORD dwType = g_ptt->TypeFromXTOKEN(xt);
        m_pstm->WriteChar(dwType);

        STRPAIR* ppair = g_ptt->PairFromXTOKEN(xt);
        _fputsTerm(ppair->first);

        if (dwType == XST_EXTENDEDINT)
        {
            _fputsTerm(ppair->second);
        }
        else
        {
            assert(ppair->second == NULL);
        }

        m_pstm->WriteChar(XT_END);
    }
    
    void _EmitToken(XTOKEN xt)
    {
        XTOKEN xtOut;
        if (m_bEmitTableEntries)
        {
            xtOut = m_tmap.GetMapping(xt);
            if (xtOut == XT_UNKNOWN)
            {
                xtOut = _GetNewXTOKEN();
                m_tmap.AddMapping(xt, xtOut);
                _OutputTableEntry(xt, xtOut);
            }
        }
        else
        {
            xtOut = xt;
        }
        fwriteMultiByteUINT(m_pstm, xtOut);
    }
   
    CTokenMap m_tmap;
    IStream *m_pstm;
    ULONG m_cRef;
    XTOKEN m_xtNext;
    bool m_bEmitTableEntries;
};


bool CTSAX2Bin::QueryInterface(RCLIID riid, void **ppvObj)
{
    if (IsEqualCLIID(riid, &IID_IUnknownCL) || 
        IsEqualCLIID(riid, &IID_ITSAXContentHandler) ||
        IsEqualCLIID(riid, &IID_ITSAX2Bin))
    {
        *ppvObj = static_cast<ITSAX2Bin*>(this);
    }
    else
    {
        return false;
    }
    AddRef();
    return true;
}

ULONG CTSAX2Bin::AddRef()
{
    return ++m_cRef;
}

ULONG CTSAX2Bin::Release()
{
    --m_cRef;
    if (m_cRef > 0)
    {
        return m_cRef;
    }
    delete this;
    return 0;
}

bool CTSAX2Bin::characters(char *pszChars, int cchChars)
{
    m_pstm->WriteChar(XT_STRING);
    _fputsTerm(pszChars);
    return true;
}

bool CTSAX2Bin::extendedint(XTOKEN xt, int iInt)
{
    _EmitToken(xt);
    fwriteMultiByteUINT(m_pstm, (UINT)iInt);

    return true;
}

bool CTSAX2Bin::endDocument()
{
    return true;
}

bool CTSAX2Bin::startDocument()
{
    XBINHEADER xh = { XBIN_VERSION };
    m_pstm->Write(&xh, sizeof(xh));
    return true;
}

bool CTSAX2Bin::endElement(XTOKEN xtName)
{
    m_pstm->WriteChar(XT_END);
    return true;
}

bool CTSAX2Bin::startElement(XTOKEN xtName)
{
    _EmitToken(xtName);
    return true;
}

bool CTSAX2Bin::attribute(XTOKEN xtName, char *pszChars, int cchChars)
{
    _EmitToken(xtName);
    _fputsTerm(pszChars);
    return true;
}

bool CTSAX2Bin::cdata(char *pszChars, int cchChars)
{
    m_pstm->WriteChar(XT_CDATA);
    fwriteMultiByteUINT(m_pstm, (UINT)cchChars);
    m_pstm->Write(pszChars, cchChars);
    return true;
}

// *** ITSAX2Bin methods ***
bool CTSAX2Bin::Init(IStream *pstm, bool bEmitTableEntries)
{
    if (m_pstm)
        m_pstm->Release();
    m_pstm = pstm;
    if (m_pstm)
        m_pstm->AddRef();

    m_bEmitTableEntries = bEmitTableEntries;
    return true;
}

bool CreateTSAX2Bin(RCLIID riid, void **ppvObj)
{
    bool bResult = false;
    
    CTSAX2Bin *ptb = new CTSAX2Bin();
    if (ptb)
    {
        bResult = ptb->QueryInterface(riid, ppvObj);
        ptb->Release();
    }

    return bResult;
}
