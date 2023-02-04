#include "xmltk.h"


class CTSAX2Nil : public ITSAXContentHandler
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

    CTSAX2Nil() : m_cRef(1)
    {
    }

private:

    ULONG m_cRef;
};

bool CTSAX2Nil::QueryInterface(RCLIID riid, void **ppvObj)
{
    if (IsEqualCLIID(riid, &IID_IUnknownCL) || 
        IsEqualCLIID(riid, &IID_ITSAXContentHandler))
    {
        *ppvObj = static_cast<ITSAXContentHandler*>(this);
    }
    else
    {
        return false;
    }
    AddRef();
    return true;
}

ULONG CTSAX2Nil::AddRef()
{
    return ++m_cRef;
}

ULONG CTSAX2Nil::Release()
{
    --m_cRef;
    if (m_cRef > 0)
    {
        return m_cRef;
    }
    delete this;
    return 0;
}

bool CTSAX2Nil::characters(char *pszChars, int cchChars)
{
    return true;
}

bool CTSAX2Nil::extendedint(XTOKEN xt, int iInt)
{
    return true;
}

bool CTSAX2Nil::endDocument()
{
    return true;
}

bool CTSAX2Nil::startDocument()
{
    return true;
}

bool CTSAX2Nil::endElement(XTOKEN xtName)
{
    return true;
}

bool CTSAX2Nil::startElement(XTOKEN xtName)
{
    return true;
}

bool CTSAX2Nil::attribute(XTOKEN xtName, char *pszChars, int cchChars)
{
    return true;
}

bool CTSAX2Nil::cdata(char *pszChars, int cchChars)
{
    return true;
}

bool CreateTSAX2Nil(RCLIID riid, void **ppvObj)
{
    bool bResult = false;
    
    CTSAX2Nil *ptb = new CTSAX2Nil();
    if (ptb)
    {
        bResult = ptb->QueryInterface(riid, ppvObj);
        ptb->Release();
    }

    return bResult;
}
