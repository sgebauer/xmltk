#include "xmltk.h"
#include "tokenmap.h"

bool operator==(const CXTOKEN& cxt1, const CXTOKEN& cxt2) 
{ 
    return cxt1() == cxt2(); 
}
bool operator<(const CXTOKEN& cxt1, const CXTOKEN& cxt2)
{ 
    return cxt1() < cxt2();
}

void CTokenMap::AddMapping(XTOKEN xtFrom, XTOKEN xtTo)
{
    myassert(m_bExternalStream);

    int iFrom = _XTOKENToIndex(xtFrom);
    if (iFrom >= m_vxtMap.size())
    {
        m_vxtMap.resize((iFrom+1)*2);
    }
    m_vxtMap[iFrom] = xtTo;
}

XTOKEN CTokenMap::GetMapping(XTOKEN xtFrom)
{
    int iIndex = _XTOKENToIndex(xtFrom);
    if (iIndex < m_vxtMap.size())
    {
        return m_vxtMap[iIndex]();
    }
    return XT_UNKNOWN;
}
   
int CTokenMap::_XTOKENToIndex(XTOKEN xt)
{
    myassert(xt >= XT_FIRST);
    return xt - XT_FIRST;
}

XTOKEN CTokenMap::_IndexToXTOKEN(int iIndex)
{
    myassert(iIndex >= 0);
    return iIndex + XT_FIRST;
}

