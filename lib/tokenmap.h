#include "xmltk.h"

#ifndef _TOKENMAP_H
#define _TOKENMAP_H

class CXTOKEN
{
public:
    CXTOKEN() : m_xt(XT_UNKNOWN) {}
    CXTOKEN(XTOKEN xt) : m_xt(xt) {}
    CXTOKEN& operator= (const CXTOKEN& cxt) { m_xt = cxt.m_xt; return *this; }
    CXTOKEN& operator= (XTOKEN xt) { m_xt = xt; return *this; }
    ~CXTOKEN() {}
    
    XTOKEN operator ()() const { return m_xt; }

private:
    XTOKEN m_xt;
};

class CTokenMap
{
public:
    void AddMapping(XTOKEN xtFrom, XTOKEN xtTo);
    XTOKEN GetMapping(XTOKEN xtFrom);

private:
    inline int _XTOKENToIndex(XTOKEN xt);
    inline XTOKEN _IndexToXTOKEN(int iIndex);

    vector<CXTOKEN> m_vxtMap;
};

#endif  // _TOKENMAP_H
