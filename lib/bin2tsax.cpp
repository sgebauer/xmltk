#include "xmltk.h"
#include "tokenmap.h"

class CBin2TSAX : public IBin2TSAX
{
public:
    // *** IUnknownCL methods ***
    CL_STDMETHOD(QueryInterface) (RCLIID riid, void **ppvObj);
    CL_STDMETHOD_(ULONG,AddRef) ();
    CL_STDMETHOD_(ULONG,Release) ();

    // *** IParse2TSAX methods ***
    CL_STDMETHOD(Parse) (IStream *pstm, ITSAXContentHandler *pch);
    CL_STDMETHOD_(unsigned int, getSrcOffset) ();
    CL_STDMETHOD(skip) (unsigned int skipDepth);

    // *** IBin2TSAX methods ***
    CL_STDMETHOD(Init) (bool bExternalStream);
    
    CBin2TSAX() : m_cRef(1), m_cchBuf(0), m_bExternalStream(true)
    {
        m_pszBuf = (char*)calloc(1024, sizeof(char));
        m_cchBufMax = m_pszBuf ? 1024 : 0;
    }

    virtual ~CBin2TSAX()
    {
    }

private:

    bool _ReallocBuffer(int cch)
    {
        char *psz = (char*)realloc(m_pszBuf, cch * sizeof(char));
        if (psz)
        {
            m_pszBuf = psz;
            m_cchBufMax = cch;
        }
        return psz != NULL;
    }

    void _ReadString(IStream *pstm)
    {
        //
        // we always use the same buffer and dynamically
        // reallocate upon overflow
        //
        for (m_cchBuf = 0; ; m_cchBuf++)
        {
            if (m_cchBuf == m_cchBufMax)
            {
                if (!_ReallocBuffer(m_cchBufMax * 2))
                {
                    m_pszBuf[m_cchBufMax-1] = 0;
                    break;
                }
            }

            m_pszBuf[m_cchBuf] = pstm->ReadChar();
            if (m_pszBuf[m_cchBuf] == EOF || m_pszBuf[m_cchBuf] == 0)
            {
                m_pszBuf[m_cchBuf] = 0;
                break;
            }
        }
    }

    void _ReadCDATA(IStream *pstm)
    {
        //
        // cdata := length buffer
        //
        m_cchBuf = 0;

        unsigned int iLength;
        if (freadMultiByteUINT(pstm, &iLength))
        {
            if (iLength <= m_cchBufMax || _ReallocBuffer(iLength))
            {
                m_cchBuf = pstm->Read(m_pszBuf, iLength);
            }
            else
            {
                //
                // can't alloc a big enough buffer.  output error, skip section, keep going.
                //
                fprintf(stderr, "WARNING: CDATA section too large to fit in memory, skipping\n");
                pstm->Seek(iLength, SEEK_CUR);
            }
        }
    }

    void _Parse(IStream *pstm, ITSAXContentHandler *pch)
    {
        //
        // do the actual work of parsing
        //
        if (m_bExternalStream)
            pch->startDocument();

        bool fContinue = true;
        
        XTOKEN xt;
        while (fContinue && freadMultiByteUINT(pstm, &xt))
        {
            switch (xt)
            {
            case XT_TABLE:
                _ParseTableEntries(pstm);
                break;

            case XT_END:
                pch->endElement(m_sxt.top());
                m_sxt.pop();

                //
                // quit if this XT_END closed the root tag
                //
                fContinue = !m_sxt.empty();
                break;

            case XT_STRING:
                _ReadString(pstm);
                pch->characters(m_pszBuf, m_cchBuf);
                break;
            
            case XT_CDATA:
                _ReadCDATA(pstm);
                pch->cdata(m_pszBuf, m_cchBuf);
                break;

            default:
                {
                    XTOKEN xtMapped = m_tmap.GetMapping(xt);
                    if (xtMapped == XT_UNKNOWN)
                    {
                        //
                        // we must be parsing a stream that doesn't have
                        // table entries.
                        //
                        xtMapped = xt;
                    }

                    DWORD dwType = g_ptt->TypeFromXTOKEN(xtMapped);
                    switch (dwType)
                    {
                    case XST_ATTRIBUTE:
                        _ReadString(pstm);
                        pch->attribute(xtMapped, m_pszBuf, m_cchBuf);
                        break;

                    case XST_ELEMENT:
                        pch->startElement(xtMapped);
                        m_sxt.push(xtMapped);
                        break;

                    case XST_EXTENDEDINT:
                        {
                            int iInt;
                            freadMultiByteUINT(pstm, (unsigned int*)&iInt);
                            pch->extendedint(xtMapped, iInt);
                        }
                        break;

                    default:
                        myassert(0);  // what happened?
                        break;
                    }
                }
                break;
            }
        }

        if (m_bExternalStream)
            pch->endDocument();
    }

    bool _ReadString(IStream *pstm, char *pszBuf, int cch)
    {
        int ch;
        for (ch = pstm->ReadChar(); ch != EOF && ch != 0; ch = pstm->ReadChar())
        {
            if (cch > 0)
            {
                *pszBuf++ = ch;
                cch--;
            }

            if (ch == 0)
            {
                break;
            }
        }

        *pszBuf = 0;
        return ch != EOF;
    }

    void _ParseTableEntries(IStream *pstm)
    {
        myassert(m_bExternalStream);    // else shouldn't have table entries
        
        //
        // table := XT_TABLE (mapping)* XT_END
        // mapping := token tokentype termstr [ termstr ]       second termstr is present if tokentype is XST_EXTENDEDINT
        //

        XTOKEN xt;
        for (freadMultiByteUINT(pstm, &xt); xt != XT_END; freadMultiByteUINT(pstm, &xt))
        {
            DWORD dwType = pstm->ReadChar();

            char sz[512];
            _ReadString(pstm, sz, ARRAYSIZE(sz));

            if (dwType == XST_EXTENDEDINT)
            {
                char szSuffix[512];
                _ReadString(pstm, szSuffix, ARRAYSIZE(szSuffix));
                
                XTOKEN xtMap = g_ptt->XTOKENFromPair(sz, szSuffix, dwType);
                m_tmap.AddMapping(xt, xtMap);
            }
            else
            {
                XTOKEN xtMap = g_ptt->XTOKENFromStr(sz, dwType);
                m_tmap.AddMapping(xt, xtMap);
            }
        }
    }

    bool _ValidateHeader(XBINHEADER* pxbh)
    {
        return pxbh->uVersion == XBIN_VERSION;
    }

    ULONG m_cRef;
    stack<XTOKEN> m_sxt;
    char *m_pszBuf;
    unsigned int m_cchBufMax;
    unsigned int m_cchBuf;
    CTokenMap m_tmap;
    bool m_bExternalStream;
};

bool CBin2TSAX::Parse(IStream *pstm, ITSAXContentHandler *pch)
{
    bool fRet = true;

    if (m_bExternalStream)
    {
        //
        // first read and validate the header
        //
        XBINHEADER xbh;
        fRet = pstm->Peek(&xbh, sizeof(XBINHEADER)) &&
            _ValidateHeader(&xbh);
        if (fRet)
        {
            pstm->Seek(sizeof(XBINHEADER), SEEK_CUR);
            _Parse(pstm, pch);
        }
    }
    else
    {
        _Parse(pstm, pch);
    }

    return fRet;
}

unsigned int CBin2TSAX::getSrcOffset(void)
{
  return 0;			// not implemented yet
}

bool CBin2TSAX::skip(unsigned int skipDepth)
{
  return false;			// not implemented yet
}

bool CBin2TSAX::Init(bool bExternalStream)
{
    m_bExternalStream = bExternalStream;
    return true;
}

bool CBin2TSAX::QueryInterface(RCLIID riid, void **ppvObj)
{
    if (IsEqualCLIID(riid, &IID_IUnknownCL) ||
        IsEqualCLIID(riid, &IID_IParse2TSAX) ||
        IsEqualCLIID(riid, &IID_IBin2TSAX))
    {
        *ppvObj = static_cast<IBin2TSAX*>(this);
    }
    else
    {
        return false;
    }
    AddRef();
    return true;
}

ULONG CBin2TSAX::AddRef()
{
    return ++m_cRef;
}

ULONG CBin2TSAX::Release()
{
    --m_cRef;
    if (m_cRef > 0)
    {
        return m_cRef;
    }
    delete this;
    return 0;
}

bool CreateBin2TSAX(RCLIID riid, void **ppvObj)
{
    bool fRet = false;

    CBin2TSAX* pbin2tsax = new CBin2TSAX();
    if (pbin2tsax)
    {
        fRet = pbin2tsax->QueryInterface(riid, ppvObj);
        pbin2tsax->Release();
    }

    return fRet;
}
