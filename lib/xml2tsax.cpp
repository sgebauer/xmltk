#include "xmltk.h"
#include "Depth2offset.hpp"

class SAXClient
{
public:

    void HandleAttribName(char *str,int len,char iscont)
    {
        // Handles a given attribute name
        // 'iscont' is one, if the string is not complete and more is to follow

        myassert(iscont != 1);    // not handled

        _CopyStringToBuffer(str, len);
        m_xtAttribute = g_ptt->XTOKENFromStr(m_psz, XST_ATTRIBUTE);
    }

    void HandleAttribValue(char *str,int len,char iscont)
    {
        // Handles an attribute value
        // 'iscont' is one, if the string is not complete and more is to follow

        //
        // BUGBUG TJGreen: iscount is *always* 1
        //

        _CopyStringToBuffer(str, len);

        m_pch->attribute(m_xtAttribute, m_psz, m_cch);
    }
    
    void HandleAttribWhiteSpaces(char *str,int len,char iscont)
    {
        // Handles white spaces between attribute-value pairs
        // 'iscont' is one, if the string is not complete and more is to follow
    }
    
    void HandleStartLabel(char *str,int len,char iscont)
    {
        myassert(iscont != 1);    // not handled
	if (indexMgr) indexMgr->startElement(parser->getSrcOffset());
        // Handles a start tag
        // 'iscont' is one, if the string is not complete and more is to follow

        _CopyStringToBuffer(str, len);
        XTOKEN xt = g_ptt->XTOKENFromStr(m_psz, XST_ELEMENT);
        m_pch->startElement(xt);
    }
    
    void HandleEndLabel(char *str,int len,char iscont)
    {
        myassert(iscont != 1);    // not handled
	if (indexMgr) indexMgr->endElement(parser->getSrcOffset());
        // Handles an end tag
        // 'iscont' is one, if the string is not complete and more is to follow

        _CopyStringToBuffer(str, len);
        XTOKEN xt = g_ptt->XTOKENFromStr(m_psz, XST_ELEMENT);
        m_pch->endElement(xt);
    }

    void HandleText(char *str,int len,char iscont,int leftwslen,int rightwslen)
    {
        // Handles a piece of text.
        // 'leftwslen' and 'rightwslen' specify how many white spaces (' ', '\n', ...)
        // are on the left and right end of the text.
        // If 'iscont=1', then 'rightwslen' must be zero and for the text piece
        // coming afterwards, 'leftwslen' must be zero.
        // It is also possible that 'len=leftwslen=rightwslen'

        //
        // hide the leading and trailing whitespace from the TSAX client
        //

        str += leftwslen;
        len = len - (leftwslen + rightwslen);
        len = max(0,len);
        
        _CopyStringToBuffer(str, len);

        if (m_cch > 0)
        {
            m_pch->characters(m_psz, m_cch);
        }
    }
    
    void HandleComment(char *str,int len,char iscont)
    {
        // Handles a piece of comment data
    }
    
    void HandlePI(char *str,int len,char iscont)
    {
        // Handles a piece of processing instruction data
    }
    
    void HandleDOCTYPE(char *str,int len,char iscont)
    {
        // Handles a piece of DOCTYPE data
    }
    
    void HandleCDATA(char *str,int len,char iscont)
    {
        // Handles a piece of CDATA data
        _CopyStringToBuffer(str, len);
        
        m_pch->cdata(m_psz, m_cch);
    }

    SAXClient(ITSAXContentHandler *pch, IParse2TSAX * p, Depth2offset * im) : m_xtAttribute(XT_UNKNOWN), m_pszBig(NULL), m_cch(0), parser(p), indexMgr(im)
    {
        m_pch = pch;
        m_pch->AddRef();
   }

    virtual ~SAXClient()
    {
        if (m_pch)
            m_pch->Release();
    }
private:

    void _CopyStringToBuffer(char *psz, int cch, bool fTrimWhitespace = false)
    {
        unsigned int cchBuf;

        m_psz = m_szBuf;
        cchBuf = ARRAYSIZE(m_szBuf);

        if ((unsigned int)cch >= ARRAYSIZE(m_szBuf)-1)
        {
            //
            // try to alloc a large enough block
            //
            if (m_pszBig)
                free(m_pszBig);

            m_pszBig = (char*)calloc(cch+1, sizeof(char));
            if (m_pszBig)
            {
                m_psz = m_pszBig;
                cchBuf = cch;
            }
            else
            {
                myassert(0);
                fprintf(stderr, "warning: low memory, data lost\n");
            }
        }

        m_cch = min((unsigned int)cch, cchBuf);
        strncpy(m_psz, psz, m_cch);
        m_psz[m_cch] = 0;
    }

    ITSAXContentHandler *m_pch;
    XTOKEN m_xtAttribute;
    char m_szBuf[4096];
    char *m_pszBig;
    char *m_psz;
    int m_cch;
    IParse2TSAX * parser;		// cache
    Depth2offset* indexMgr;		// cache
};

#include "xmlparse.hpp"     // not really a header, inline source

class CXML2TSAX : public IParse2TSAX
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
    
    CXML2TSAX() : m_cRef(1), indexMgr(0)
    {
    }
    
    virtual ~CXML2TSAX()
    {
	  if (indexMgr) delete indexMgr;
    }
    
private:
  ULONG m_cRef;
  XMLParse xmlparse;
  Depth2offset * indexMgr;
};

bool CXML2TSAX::QueryInterface(RCLIID riid, void **ppvObj)
{
    if (IsEqualCLIID(riid, &IID_IUnknownCL) ||
        IsEqualCLIID(riid, &IID_IParse2TSAX))
    {
        *ppvObj = static_cast<IParse2TSAX*>(this);
    }
    else
    {
        return false;
    }
    AddRef();
    return true;
}

ULONG CXML2TSAX::AddRef()
{
    return ++m_cRef;
}

ULONG CXML2TSAX::Release()
{
    --m_cRef;
    if (m_cRef > 0)
    {
        return m_cRef;
    }
    delete this;
    return 0;
}

bool CXML2TSAX::Parse(IStream *pstm, ITSAXContentHandler *pch)
{
    IFileStream *pfs;
    bool bRet = pstm->QueryInterface(&IID_IFileStream, (void**)&pfs);
    if (bRet){
	  try {         
        if (xmlparse.OpenFile(pfs->GetFileName()))
        {
		    indexMgr = new Depth2offset(pfs->GetFileName());
		    indexMgr = indexMgr->init();
            SAXClient saxclient(pch, this, indexMgr);
            pch->startDocument();
	
            xmlparse.DoParsing(&saxclient);
            pch->endDocument();
            xmlparse.CloseFile();
            bRet = true;
        }

        pfs->Release();
	  }
	  catch (XMillException const * err) {
		fprintf(stderr, "XML parser error. Is \"%s\" correct?\n", pfs->GetFileName());
	  }
	}
    
    return bRet;
}

unsigned int CXML2TSAX::getSrcOffset(void)
{
  return xmlparse.getSrcOffset();
}

bool CXML2TSAX::skip(unsigned int skipDepth)
{
  if (skipDepth == 0) return true;
  if (indexMgr && indexMgr->isAppliable()==true){
	unsigned int offset = indexMgr->skip(skipDepth, getSrcOffset());
	xmlparse.skipReader(offset, skipDepth);
	return true;
  }
  else return false;
}

bool CreateXML2TSAX(RCLIID riid, void **ppvObj)
{
    bool fRet = false;
    
    CXML2TSAX* pxml2tsax = new CXML2TSAX();
    if (pxml2tsax)
    {
        fRet = pxml2tsax->QueryInterface(riid, ppvObj);
        pxml2tsax->Release();
    }
    
    return fRet;
}
