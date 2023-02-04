#include "xmltk.h"

class CMemoryStream : public IMemoryStream
{
public:
   // *** IUnknownCL methods ***
    CL_STDMETHOD(QueryInterface) (RCLIID riid, void **ppvObj);
    CL_STDMETHOD_(ULONG,AddRef) ();
    CL_STDMETHOD_(ULONG,Release) ();

    // *** IStream methods ***
    CL_STDMETHOD_(long, CopyTo) (IStream *pstm, long cb);
    CL_STDMETHOD_(long, Peek) (void *pv, long cb);
    CL_STDMETHOD_(long, Read) (void *pv, long cb);
    CL_STDMETHOD_(int, ReadChar) ();
    CL_STDMETHOD_(int, Seek) (long offset, int origin);
    CL_STDMETHOD_(long, SetSize) (long lsize);
    CL_STDMETHOD_(long, GetSize) ();
    CL_STDMETHOD_(long, Tell) ();
    CL_STDMETHOD_(long, Write) (void *pv, long cb);
    CL_STDMETHOD(WriteChar) (int ch);

    // *** IMemoryStream methods ***
    CL_STDMETHOD_(int, getSize) ();
    CL_STDMETHOD_(char*, getBuffer) ();
    CL_STDMETHOD(setBuffer) (void *pv, int iSize);
    CL_STDMETHOD_(int, getGrowSize) ();
    CL_STDMETHOD(setGrowSize) (int cchGrow);
    
    // *** misc public methods ***
    CMemoryStream() : m_cRef(1),  m_pszBuf(NULL), m_cchBuf(0),
        m_iCur(0), m_cchGrow(256), m_iLast(0)
    {
        m_cchGrow = 256;
        _SizeBuffer(m_cchGrow);
    }

private:

    virtual ~CMemoryStream()
    {
        if (m_pszBuf)
            free(m_pszBuf);
    }

    void _SetPosition(long iPos)
    {
        m_iCur = iPos;
        m_iLast = max(m_iCur, m_iLast);
    }

    bool _SizeBuffer(int cchNew)
    {
        bool bRet = false;
        
        if (m_pszBuf == NULL)
        {
            m_pszBuf = (char*)malloc(cchNew * sizeof(*m_pszBuf));
            if (m_pszBuf)
            {
                m_cchBuf = cchNew;
                bRet = true;
            }
        }
        else
        {
            char *pszNew = (char*)realloc(m_pszBuf, cchNew / sizeof(*pszNew));
            if (pszNew)
            {
                m_pszBuf = pszNew;
                m_cchBuf = cchNew;
                bRet = true;
            }
        }
        return bRet;
    }
    
    bool _GrowBuffer(int cchNew)
    {
        cchNew += m_iCur;
        if (cchNew >= m_cchBuf)
        {
#ifdef USE_BACKOFF
            cchNew = max(m_cchBuf*2, cchNew);
#else
            cchNew += m_cchGrow - (cchNew % m_cchGrow);
#endif
            return _SizeBuffer(cchNew);
        }

        return true;
    }
   
    long _Read(void *pv, long cb, bool fUpdatePosition)
    {
        cb = min(cb, m_iLast - m_iCur);
        memcpy(pv, &m_pszBuf[m_iCur], cb);
        if (fUpdatePosition)
            _SetPosition(m_iCur + cb);
        return cb;
    }
    
    ULONG m_cRef;
    char *m_pszBuf;
    int m_cchBuf;
    int m_iCur;
    int m_cchGrow;
    int m_iLast;
};

// *** IUnknown methods ***
// 
bool CMemoryStream::QueryInterface(RCLIID riid, void **ppvObj)
{
    if (IsEqualCLIID(riid, &IID_IUnknownCL) ||
        IsEqualCLIID(riid, &IID_IStream) ||
        IsEqualCLIID(riid, &IID_IMemoryStream))
    {
        *ppvObj = static_cast<IMemoryStream*>(this);
    }
    else
    {
        *ppvObj = NULL;
        return false;
    }
    AddRef();
    return true;
}

ULONG CMemoryStream::AddRef()
{
    return ++m_cRef;
}

ULONG CMemoryStream::Release()
{
    if (--m_cRef > 0)
        return m_cRef;

    delete this;
    return 0;
}

// *** IStream methods ***
long CMemoryStream::CopyTo(IStream *pstm, long cb)
{
    cb = min(cb, (long)(m_iLast - m_iCur));
    return pstm->Write((void*)&m_pszBuf[m_iCur], cb);
}

long CMemoryStream::Peek(void *pv, long cb)
{
    return _Read(pv, cb, false);
}

long CMemoryStream::Read(void *pv, long cb)
{
    return _Read(pv, cb, true);
}

int CMemoryStream::ReadChar()
{
    if (m_iCur < m_iLast)
    {
        int iRet = m_pszBuf[m_iCur];
        _SetPosition(m_iCur+1);
        return iRet;
    }
    return EOF;
}

int CMemoryStream::Seek(long offset, int origin)
{
    switch (origin)
    {
    case SEEK_SET:
        _SetPosition(offset);
        break;
        
    case SEEK_END:
        _SetPosition(m_iLast - offset);
        break;
        
    case SEEK_CUR:
        _SetPosition(m_iCur + offset);
        break;
    }

    return 0;
}

long CMemoryStream::SetSize(long lsize)
{
    _SetPosition(min(m_iCur, lsize-1));
    return _SizeBuffer(lsize) ? lsize : 0;
}

long CMemoryStream::GetSize()
{
    return m_cchBuf * sizeof(char);
}

long CMemoryStream::Tell()
{
    return m_iCur;
}

long CMemoryStream::Write(void *pv, long cb)
{
    if (_GrowBuffer(cb))
    {
        memcpy(&m_pszBuf[m_iCur], pv, cb);
        _SetPosition(m_iCur + cb);
        return cb;
    }
    return EOF;
}

bool CMemoryStream::WriteChar(int ch)
{
    if (_GrowBuffer(1))
    {
        m_pszBuf[m_iCur] = ch;
        _SetPosition(m_iCur+1);
        return true;
    }
    return false;
}

// *** IMemoryStream methods ***
int CMemoryStream::getSize()
{
    return m_iLast;
}

char *CMemoryStream::getBuffer()
{
    return m_pszBuf;
}

bool CMemoryStream::setBuffer(void *pv, int iSize)
{
    m_pszBuf = (char*)pv;
    m_cchBuf = iSize;
    m_iCur = 0;
    m_iLast = iSize - 1;
    return true;
}

int CMemoryStream::getGrowSize()
{
    return m_cchGrow;
}

bool CMemoryStream::setGrowSize(int cchGrow)
{
    m_cchGrow = cchGrow;
    return true;
}


bool CreateMemoryStream(RCLIID riid, void **ppvObj)
{
    bool bRet = false;
    CMemoryStream* pms = new CMemoryStream();
    if (pms)
    {
        bRet = pms->QueryInterface(riid, ppvObj);
        pms->Release();
    }
    
    return bRet;
}

