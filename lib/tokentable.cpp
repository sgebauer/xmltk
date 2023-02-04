
#include "xmltk.h"



typedef struct
{
    STRPAIR pair;
    DWORD dwType;
}
TOKENINFO, *PTOKENINFO;

typedef vector<TOKENINFO> VTOKENINFO;


struct tokeninfo_cmpeq
{
    bool operator() (const TOKENINFO &ti1, const TOKENINFO &ti2) const
    {
        return ti1.dwType == ti2.dwType &&
            mystrcmp(ti1.pair.first, ti2.pair.first) == 0 &&
            mystrcmp(ti1.pair.second, ti2.pair.second) == 0;
    }
};

inline size_t myhashstr(char const *psz)
{
    return psz ? std::hash<char const *>()(psz) : 0;
}

struct tokeninfo_hash
{
    size_t operator() (const TOKENINFO &ti) const
    {
        return myhashstr(ti.pair.first) + 
                myhashstr(ti.pair.second) + 
                ti.dwType;
    }
};

typedef hash_map<TOKENINFO, unsigned int, tokeninfo_hash, tokeninfo_cmpeq> MAP_TI_INDEX;



class CTokenTable : public ITokenTable
{
public:
    // *** IUnknownCL methods ***
    CL_STDMETHOD(QueryInterface) (RCLIID riid, void **ppvObj);
    CL_STDMETHOD_(ULONG,AddRef) ();
    CL_STDMETHOD_(ULONG,Release) ();

    // *** ITokenTable methods ***
    CL_STDMETHOD_(char*, StrFromXTOKEN) (XTOKEN xt);
    CL_STDMETHOD_(DWORD, TypeFromXTOKEN) (XTOKEN xt);
    CL_STDMETHOD_(STRPAIR*, PairFromXTOKEN) (XTOKEN xt);
    CL_STDMETHOD_(XTOKEN, XTOKENFromStr)  (char* psz, DWORD dwType);
    CL_STDMETHOD_(XTOKEN, XTOKENFromPair) (char *pszFirst, char *pszSecond, DWORD dwType);

    CTokenTable() : m_cRef(1)
    {
    }
    
    virtual ~CTokenTable()
    {
        for (int i = m_vtokeninfo.size() - 1; i >= 0; i--)
        {
            if (m_vtokeninfo[i].pair.first)
                free(m_vtokeninfo[i].pair.first);
            if (m_vtokeninfo[i].pair.second)
                free(m_vtokeninfo[i].pair.second);
        }
    }

private:

    unsigned int _XTOKENToIndex(XTOKEN xt)
    {
        return xt - XT_FIRST;
    }

    XTOKEN _IndexToXTOKEN(unsigned int iIndex)
    {
        return iIndex + XT_FIRST;
    }

    ULONG m_cRef;

    VTOKENINFO m_vtokeninfo;        // owns the STRPAIR strings.  indexed directly by XTOKEN (minus offset).
    MAP_TI_INDEX m_mapti;           // does not own the STRPAIR strings.  hash index into m_vtokeninfo for
                                    // fast string lookup.
};

// *** IUnknownCL methods ***
bool CTokenTable::QueryInterface(RCLIID riid, void **ppvObj)
{
    if (IsEqualCLIID(riid, &IID_IUnknownCL))
    {
        *ppvObj = (void*)static_cast<IUnknownCL*>(this);
    }
    else if (IsEqualCLIID(riid, &IID_ITokenTable))
    {
        *ppvObj = (void*)static_cast<ITokenTable*>(this);
    }
    else
    {
        return false;
    }
    AddRef();
    return true;
}

ULONG CTokenTable::AddRef()
{
    return ++m_cRef;
}

ULONG CTokenTable::Release()
{
    --m_cRef;
    if (m_cRef > 0)
    {
        return m_cRef;
    }
    delete this;
    return 0;
}


// *** ITokenTable methods ***
char* CTokenTable::StrFromXTOKEN(XTOKEN xt)
{
    UINT uIndex = _XTOKENToIndex(xt);
    if (uIndex >= 0 && uIndex < m_vtokeninfo.size())
    {
        return m_vtokeninfo[uIndex].pair.first;
    }
    return NULL;
}

DWORD CTokenTable::TypeFromXTOKEN(XTOKEN xt)
{
    UINT uIndex = _XTOKENToIndex(xt);
    if (uIndex >= 0 && uIndex < m_vtokeninfo.size())
    {
        return m_vtokeninfo[uIndex].dwType;
    }
    return 0;
}

STRPAIR *CTokenTable::PairFromXTOKEN(XTOKEN xt)
{
    UINT uIndex = _XTOKENToIndex(xt);
    if (uIndex >= 0 && uIndex < m_vtokeninfo.size())
    {
        return &m_vtokeninfo[uIndex].pair;
    }
    return NULL;
}

XTOKEN CTokenTable::XTOKENFromStr(char* psz, DWORD dwType)
{
    return XTOKENFromPair(psz, NULL, dwType);
}

XTOKEN CTokenTable::XTOKENFromPair(char* pszFirst, char *pszSecond, DWORD dwType)
{
    TOKENINFO ti;
    ti.dwType = dwType;
    ti.pair.first = pszFirst;
    ti.pair.second = pszSecond;

    MAP_TI_INDEX::const_iterator it = m_mapti.find(ti);
    if (it != m_mapti.end())
    {
        //
        // found, map index to XTOKEN and return
        //
        return _IndexToXTOKEN((*it).second);
    }
    else
    {
        //
        // not found, let's insert it now.
        //
        ti.pair.first = mystrdup(ti.pair.first);
        ti.pair.second = mystrdup(ti.pair.second);

        UINT iIndex = m_vtokeninfo.size();

        m_vtokeninfo.resize(iIndex+1);
        m_vtokeninfo[iIndex] = ti;

        m_mapti.insert(MAP_TI_INDEX::value_type(ti, iIndex));

        return _IndexToXTOKEN(iIndex);
    }
}

ITokenTable *g_ptt = NULL;

bool GetGlobalTokenTable(RCLIID riid, void** ppv)
{
    if (g_ptt)
    {
        return g_ptt->QueryInterface(riid, ppv);
    }

    return false;
}

void InitGlobalTokenTable()
{
    assert (g_ptt == NULL);
    CTokenTable *ptt = new CTokenTable();
    if (ptt)
    {
        ptt->QueryInterface(&IID_ITokenTable, (void**)&g_ptt);
        ptt->Release();
    }
}

void CleanupGlobalTokenTable()
{
    if (g_ptt)
    {
        g_ptt->Release();
        g_ptt = NULL;
    }
}
