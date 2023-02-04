#include "xmltk.h"

int g_cbMaxBuf = 32 * 1024 * 1024;     // default to 32 MB sorting window
int g_cMaxKeys = 0;

typedef pair<int, string> KEYVALUE;
typedef vector<KEYVALUE> VKEYVALUE;

Variable *g_pvRoot = NULL;


class RECORDKEY
{
public:
    RECORDKEY(int iNumVals = g_cMaxKeys)
    {
        vvalues.resize(iNumVals);
        Clear();
    }

    RECORDKEY(const RECORDKEY& rk)
    {
        *this = rk;
    }

    const RECORDKEY& operator=(const RECORDKEY& rk)
    {
        iElement = rk.iElement;
        iOrdinal = rk.iOrdinal;
        vvalues = rk.vvalues;
        return *this;
    }

    void Clear()
    {
        iElement = INT_MAX;
        iOrdinal = INT_MAX;
        for (VKEYVALUE::iterator it = vvalues.begin(); it < vvalues.end(); it++)
        {
            (*it).first = INT_MAX;
            (*it).second.assign("");
        }
    }

    int GetSize()
    {
        int cb = sizeof(RECORDKEY);
        for (VKEYVALUE::iterator it = vvalues.begin();
             it < vvalues.end(); it++)
        {
            cb += sizeof(KEYVALUE) + (*it).second.length();
        }
        return cb;
    }

    int iElement;
    int iOrdinal;
    VKEYVALUE vvalues;
};

typedef pair<RECORDKEY, IStream*> RECORDPAIR;

int _CompareRecordKeys(RECORDKEY *prk1, RECORDKEY *prk2)
{
    int iCompare = prk1->iElement - prk2->iElement;
    myassert(prk1->vvalues.size() == prk2->vvalues.size());
    for (int i = 0; iCompare == 0 && i < prk1->vvalues.size(); i++)
    {
        iCompare = prk1->vvalues[i].first - prk2->vvalues[i].first;
        if (iCompare == 0)
        {
            iCompare = prk1->vvalues[i].second.compare(prk2->vvalues[i].second);
        }
    }
    if (iCompare == 0)
    {
        iCompare = prk1->iOrdinal - prk2->iOrdinal;
    }
        
    return iCompare;
}

struct gtrecordpair
{
    bool operator() (RECORDPAIR &rp1, RECORDPAIR &rp2) const
    {
        return _CompareRecordKeys(&rp1.first, &rp2.first) > 0;
    }
};

typedef priority_queue<RECORDPAIR, vector<RECORDPAIR>, gtrecordpair> RECORDQUEUE;

inline void fwriteMultiByteINT(IStream *pstm, int i)
{
    fwriteMultiByteUINT(pstm, (unsigned int)i);
}

inline bool freadMultiByteINT(IStream *pstm, int *pi)
{
    return freadMultiByteUINT(pstm, (unsigned int*)pi);
}

class CSortHandler : public IFilteredTSAXHandler
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

    CSortHandler(ITSAXContentHandler *pchOut) : m_pms(NULL), m_pfs(NULL), m_bCurrentFirst(true),
                                                m_iKeyContext(-1),
                                                m_cRef(1), m_iOrdinal(0), m_cbBaseline(0)
    {
        m_pchOut = pchOut;
        m_pchOut->AddRef();
        m_pchCur = m_pchOut;
        CreateTSAX2Bin(&IID_ITSAX2Bin, (void**)&m_p2Bin);
        CreateBin2TSAX(&IID_IBin2TSAX, (void**)&m_pBin2);
        m_pBin2->Init(false);

        m_rkCur.vvalues.resize(g_cMaxKeys);
        m_rkFile.vvalues.resize(g_cMaxKeys);

        m_rkCur.Clear();
        m_rkFile.Clear();
    }

private:
    virtual ~CSortHandler()
    {
        ATOMICRELEASE(m_pBin2);
        ATOMICRELEASE(m_pms);
        ATOMICRELEASE(m_p2Bin);
        ATOMICRELEASE(m_pchOut); 
        ATOMICRELEASE(m_pfs);
    };

    void _ReadString(IStream *pstm, string &str)
    {
        int ch;
        for (ch = pstm->ReadChar(); ch != EOF && ch != '\0'; ch = pstm->ReadChar())
        {
            str += ch;
        }
    }
    
    void _WriteKeyToStream(IStream *pstm, RECORDKEY *prk)
    {
        fwriteMultiByteINT(pstm, prk->iElement);
        for (VKEYVALUE::iterator it = prk->vvalues.begin(); it < prk->vvalues.end(); it++)
        {
            int iPos = it - prk->vvalues.begin();
            if ((*it).first != INT_MAX)
            {
                fwriteMultiByteINT(pstm, iPos*2);
                fwriteMultiByteINT(pstm, (*it).first);
            }
            else if ((*it).second.size() > 0)
            {
                fwriteMultiByteINT(pstm, iPos*2+1);
                pstm->Write((void*)(*it).second.c_str(), (*it).second.length());
                pstm->WriteChar(0);
            }
        }
        fwriteMultiByteINT(pstm, prk->iOrdinal+(g_cMaxKeys+1)*2);
    }

    bool _ReadKeyFromStream(IStream *pstm, RECORDKEY *prk)
    {
        bool bRet = false;
        prk->Clear();
        
        if (freadMultiByteINT(pstm, &prk->iElement))
        {
            int iOrdinalFirst = (g_cMaxKeys+1)*2;
            int iNext;
            while (freadMultiByteINT(pstm, &iNext))
            {
                if (iNext >= iOrdinalFirst)
                {
                    //
                    // it's the ordinal, which comes last
                    //
                    prk->iOrdinal = iNext - iOrdinalFirst;
                    bRet = true;
                    break;
                }
                else if (iNext % 2 == 0)
                {
                    //
                    // it's an int
                    //
                    freadMultiByteINT(pstm, &prk->vvalues[iNext/2].first);
                }
                else
                {
                    //
                    // it's a string
                    //
                    _ReadString(pstm, prk->vvalues[iNext/2].second);
                }
            }
        }
        return bRet;
    }

    
    int _WriteRecordToStream(RECORDKEY *prk, IStream *pstmIn, IStream *pstmOut)
    {
        //
        // write the record to disk.  (NULL-terminated string
        // followed by structured binary buffer.)
        //
        int iPos = pstmOut->Tell();
        _WriteKeyToStream(pstmOut, prk);
        pstmIn->CopyTo(pstmOut, INT_MAX);
        return pstmOut->Tell() - iPos;
    }
 
    void _OnOpenContext()
    {
        m_cbUsed = m_cbBaseline;
    }

    void _MoveMapToQueue(RECORDQUEUE *pqueue)
    {
        //
        // move the contents of this queue to the merge queue
        // 
        // BUGBUG we are paying the cost of popping the items
        // in sorted order, but it doesn't matter what order we
        // insert them into the merge queue.  Is there a way to
        // obtain the underlying vector container from the
        // RECORDQUEUE and iterate through that?
        //
        while (pqueue->size() > 0)
        {
            RECORDPAIR rp = pqueue->top();
            pqueue->pop();
            m_queueMerge.push(rp);
        }
    }
   
    void _AddFileStreamToQueue()
    {
        if (m_pfs)
        {
#ifdef XSORT_DEBUG
          cerr << "AFS2Q"
               << "\tfile size: " << m_pfs->Tell()/1024 << "KB"
               << "\tmodel mem: " << m_cbUsed/1024 << "KB"
               << "\tactual mem: " << QueryProcessMemoryUsage()/1024 << "KB"
               << endl;
#endif

            m_pfs->Seek(0, SEEK_SET);
            _AddStreamToQueue(m_pfs);
            ATOMICRELEASE(m_pfs);
        }
    }

    //
    // _MemoryTop() and _MemoryPop() exploit the invariant that
    // all items in the "next" queue come before all items in the
    // "current" queue
    //

    RECORDPAIR *_MemoryTop()
    {
        if (m_bCurrentFirst && m_queue2.size() > 0)
        {
            return &const_cast<RECORDPAIR&>(m_queue2.top());
        }
        else if (m_queue1.size() > 0)
        {
            return &const_cast<RECORDPAIR&>(m_queue1.top());
        }
        else if (m_queue2.size() > 0)
        {
            return &const_cast<RECORDPAIR&>(m_queue2.top());
        }
        return NULL;
    }

    void _MemoryPop()
    {
        if (m_bCurrentFirst && m_queue2.size() > 0)
        {
            m_queue2.pop();
        }
        else if (m_queue1.size())
        {
            m_queue1.pop();
        }
        else if (m_queue2.size())
        {
            m_queue2.pop();
        }
    }

    RECORDPAIR *_MergeTop()
    {
        return m_queueMerge.size() > 0 ? 
          &const_cast<RECORDPAIR&>(m_queueMerge.top()) : 
          NULL;
    }

    IStream *_PushMemoryTop()
    {
        IStream *pstm = NULL;

        RECORDPAIR *prpMemory = _MemoryTop();
        if (prpMemory)
        {
            //
            // queue takes a ref, and returned ptr has a ref
            //
            m_queueMerge.push(*prpMemory);
            pstm = prpMemory->second;
            pstm->AddRef();

            _MemoryPop();
        }

        return pstm;
    }

    void _FlushMemoryQueue(RECORDQUEUE *pqueue)
    {
        while (pqueue->size() > 0)
        {
            RECORDPAIR *prp = &const_cast<RECORDPAIR&>(pqueue->top());
            m_pBin2->Parse(prp->second, m_pchOut);
            prp->second->Release();
            pqueue->pop();
        }
    }

    void _MergeSortMemoryAndDisk()
    {
        IStream *pstmMemory = _PushMemoryTop();

        while (m_queueMerge.size() > 0)
        {
            RECORDPAIR *prp = &const_cast<RECORDPAIR&>(m_queueMerge.top());
            m_pBin2->Parse(prp->second, m_pchOut);

            if (prp->second == pstmMemory)
            {
                pstmMemory->Release();
                pstmMemory = _PushMemoryTop();
            }
            else 
            {
                _AddStreamToQueue(prp->second);
            }
            prp->second->Release(); 
            m_queueMerge.pop();
        }

        ATOMICRELEASE(pstmMemory);

        m_rkFile.Clear();
    }

    void _OnCloseContext()
    {
        //
        // sorting time! add the current file to queue
        //
        _AddFileStreamToQueue();

        //
        // merge the sorted runs and output
        //

        if (m_queueMerge.size() == 0)
        {
            _FlushMemoryQueue(_QueueNext());
            _FlushMemoryQueue(_QueueCur());
        }
        else
        {
            _MergeSortMemoryAndDisk();
        }

        m_iOrdinal = 0;
    }

    void _OnOpenElement(Variable *pv)
    {
        myassert(m_pms == NULL);
    
        m_pchCur = m_p2Bin;
        CreateMemoryStream(&IID_IMemoryStream, (void**)&m_pms);
        m_p2Bin->Init(m_pms, false);

        m_rkCur.iElement = pv->getlParam();
        m_rkCur.iOrdinal = m_iOrdinal++;
    }
    
    void _AddStreamToQueue(IStream *pstm)
    {
        if (pstm)
        {
            RECORDKEY rk;
            if (_ReadKeyFromStream(pstm, &rk))
            {
                //
                // queue takes ownership of key and stream
                //
                pstm->AddRef();
                m_queueMerge.push(RECORDPAIR(rk, pstm));
            }
        }
    }
    
    int _MoveSingleRecordToDisk()
    {
        int cbRecord = 0;
        
        //
        // if current map is empty, need to make the next map
        // current
        //
        if (_QueueCur()->empty())
        {
            _AddFileStreamToQueue();
            m_bCurrentFirst = !m_bCurrentFirst;
        }

        //
        // remove the first record from _QueueCur and write it to disk
        //
        if (!_QueueCur()->empty())
        {
            RECORDPAIR& rp = const_cast<RECORDPAIR&>(_QueueCur()->top());
            
            //
            // create the file if necessary
            //
            if (m_pfs == NULL)
            {
                CreateFileStream(&IID_IFileStream, (void**)&m_pfs);
                static int s_cfiles = 0;
                FILE *pfile = tmpfile();
                s_cfiles++;
                if (pfile == NULL)
                {
                    char szBuf[256];
                    snprintf(szBuf, ARRAYSIZE(szBuf), "failed to create tempfile number %d", s_cfiles);
                    perror(szBuf);
                    myassert(0);
                }
                m_pfs->SetFile(pfile);
            }

            _WriteRecordToStream(&rp.first, rp.second, m_pfs);
            cbRecord = rp.first.GetSize() + rp.second->GetSize() + sizeof(RECORDPAIR);

            m_rkFile = rp.first;
            rp.second->Release();
            
            _QueueCur()->pop();
        }

        return cbRecord;
    }

    int _MoveRecordsToDisk(int iMemSize)
    {
       int iRemaining = iMemSize;
       while (iRemaining > 0)
       {
           int iMemRecord = _MoveSingleRecordToDisk();
           if (iMemRecord == 0)
           {
               break;
           }
           else
           {
               iRemaining -= iMemRecord;
           }
       }

       return iMemSize - iRemaining;
    }
    
    void _OnCloseElement()
    {
        m_cbUsed += m_pms->GetSize() + m_rkCur.GetSize() + sizeof(RECORDPAIR);

        m_pms->Seek(0, SEEK_SET);

        //
        // figure out where this record will be inserted.
        //
        RECORDQUEUE* pqueue = 
            (m_pfs && _CompareRecordKeys(&m_rkCur, &m_rkFile) < 0) ?
            _QueueNext() : 
            _QueueCur();
        
        //
        // insert the record.  queue takes ownership of key and stream.
        //
        pqueue->push(RECORDPAIR(m_rkCur, m_pms));
        m_pms = NULL;
        m_rkCur.Clear();

        //
        // output some records to disk if necessary
        //

        if (m_cbUsed >= g_cbMaxBuf)
        {
            m_cbUsed -= _MoveRecordsToDisk(m_cbUsed - g_cbMaxBuf);
        }

        m_pchCur = m_pchOut;
    }

    void _OnOpenKey(Variable *pv)
    {
        m_iKeyContext = pv->getlParam();
    }
    
    void _OnCloseKey()
    {
        m_iKeyContext = -1;
    }
   
    RECORDQUEUE *_QueueCur()
    {
        return m_bCurrentFirst ? &m_queue1 : &m_queue2;
    }
    RECORDQUEUE *_QueueNext()
    {
        return m_bCurrentFirst ? &m_queue2 : &m_queue1;
    }

    ITSAXContentHandler *m_pchCur;  // N.B. not ref-counted, for convenience
    ITSAXContentHandler *m_pchOut;
    ITSAX2Bin *m_p2Bin;
    IMemoryStream *m_pms;           // in-memory current element
    IFileStream *m_pfs;             // on-disk part of current run

    IBin2TSAX *m_pBin2;
    RECORDQUEUE m_queue1;           // in-memory run 1 (current or next)
    RECORDQUEUE m_queue2;           // in-memory run 2 (next or current)
    bool m_bCurrentFirst;           // is m_queue1 or m_queue2 the current one?
    int m_iKeyContext;
    RECORDKEY m_rkCur;              // key-in-progress for current element
    RECORDKEY m_rkFile;             // key for last record written to disk
    
    ULONG m_cRef;
    int m_iOrdinal;                 // # element in the current context
    
    int m_cbUsed;
    int m_cbBaseline;

    RECORDQUEUE m_queueMerge;       // priority queue of runs to be merged
};

// *** IUnknown methods ***
bool CSortHandler::QueryInterface(RCLIID riid, void **ppvObj)
{
    if (IsEqualCLIID(riid, &IID_IUnknownCL) ||
        IsEqualCLIID(riid, &IID_ITSAXContentHandler) ||
        IsEqualCLIID(riid, &IID_IFilteredTSAXHandler))
    {
        *ppvObj = static_cast<IFilteredTSAXHandler*>(this);
    }
    else
    {
        *ppvObj = NULL;
        return false;
    }
    
    AddRef();
    return true;
}

ULONG CSortHandler::AddRef()
{
    return ++m_cRef;
}

ULONG CSortHandler::Release()
{
    if (--m_cRef > 0)
    {
        return m_cRef;
    }
    delete this;
    return 0;
}

// *** ITSAXContentHandler methods ***
bool CSortHandler::startDocument()
{
    m_cbBaseline = QueryProcessMemoryUsage();
    m_pchCur->startDocument();
    return true;
}

bool CSortHandler::endDocument()
{
    m_pchCur->endDocument();
    return true;
}

bool CSortHandler::startElement(XTOKEN xtName)
{
#if defined(XMLTK_OUTPUT)	
  cerr << "startElement(" << g_ptt->StrFromXTOKEN(xtName) << ")" << endl;
#else
    m_pchCur->startElement(xtName); 
#endif
    return true;
}

bool CSortHandler::endElement(XTOKEN xtName)
{
    if (!_QueueCur()->empty() && m_pms == NULL)
    {
        //
        // this endElement is closing the sort context.
        //
        _OnCloseContext();
    }

#if defined(XMLTK_OUTPUT)	
  cerr << "endElement(" << g_ptt->StrFromXTOKEN(xtName) << ")" << endl;
#else
    m_pchCur->endElement(xtName);
#endif
    return true;
}

bool CSortHandler::attribute(XTOKEN xtName, char *pszChars, int cchChars)
{
    if (m_iKeyContext != -1)
    {
        myassert(m_iKeyContext < m_rkCur.vvalues.size());
        m_rkCur.vvalues[m_iKeyContext].second.append(pszChars);
    }
#if defined(XMLTK_OUTPUT)	
  cerr << "attribute(" << g_ptt->StrFromXTOKEN(xtName) << ")" << endl;
#else
    m_pchCur->attribute(xtName, pszChars, cchChars);
#endif
    return true;
}

bool CSortHandler::characters(char *pszChars, int cchChars)
{
    if (m_iKeyContext != -1)
    {
        myassert(m_iKeyContext < m_rkCur.vvalues.size());
        m_rkCur.vvalues[m_iKeyContext].second.append(pszChars);
    }
#if defined(XMLTK_OUTPUT)
	cerr << "characters()" << endl;
#else
    m_pchCur->characters(pszChars, cchChars);
#endif
    return true;
}

bool CSortHandler::cdata(char *pszChars, int cchChars)
{
    m_pchCur->cdata(pszChars, cchChars);
    return true;
}

bool CSortHandler::extendedint(XTOKEN xt, int iInt)
{
    if (m_iKeyContext != -1)
    {
        myassert(m_iKeyContext < m_rkCur.vvalues.size());
        m_rkCur.vvalues[m_iKeyContext].first = iInt;
    }
    m_pchCur->extendedint(xt, iInt);
    return true;
}

// *** IFilteredTSAXHandler methods ***
bool CSortHandler::startContext(Variable *pv)
{
#if defined(XMLTK_OUTPUT)	
  cerr << "startContext(" << pv << ")" << endl;
#endif
    switch (GetVariableDepth(pv))
    {
    case 1:
        if (pv != g_pvRoot)
        {
            _OnOpenContext();
        }
        break;

    case 2:
        _OnOpenElement(pv);
        break;

    case 3:
        _OnOpenKey(pv);
        break;
    
    default:			// for root XPE
        break;
    }
    return true;
}
bool CSortHandler::endContext(Variable *pv)
{
#if defined(XMLTK_OUTPUT)	
  cerr << "endContext(" << pv << ")" << endl;
#endif
    switch (GetVariableDepth(pv))
    {
    case 1:     // just closed context     
	  //        _OnCloseContext();
        break;

    case 2:     // just closed element
        _OnCloseElement();
        break;

    case 3:     // just closed key
        _OnCloseKey();
        break;

    default:	// for root XPE
        break;
    }
    return true;
}


//
// global variables needed by command-line parser
//
char *g_pszProgName;
char **g_targv;
char **g_arglim;
bool g_bBinary = false;
int g_iIndent = 0;

//
// global variables needed here only
//
IDfaFilter *g_pfilter = NULL;
Variable *g_vContextLast = NULL, *g_vElementLast = NULL;
float	  g_fContextPrec = 0.00, g_fElementPrec = 0.00;
int g_cElement = 0, g_cKey = 0;
char *g_pszFileName = NULL;

extern int mylex();

//
// note how we store the relative order of elements and keys
// in the Variable lParam field.  we use this information
// later when constructing sort keys.
//
void _UseContext(char *psz)
{
    g_vContextLast = g_pfilter->RegisterQuery(g_pvRoot, psz, false, g_fContextPrec);
#if defined(XMLTK_OUTPUT)
	cerr << "context Var = " << g_vContextLast << endl;
#endif
    g_vElementLast = NULL;
    g_cElement = 0;
	g_fContextPrec += 1.00;			 // increment
	g_fElementPrec = g_fContextPrec; // initialize
}

void _UseElement(char *psz)
{
    if (g_vContextLast == NULL)
    {
        //
        // coerce root context
        //
        _UseContext("/");
    }
    g_vElementLast = g_pfilter->RegisterQuery(g_vContextLast, psz, true, g_fElementPrec);
#if defined(XMLTK_OUTPUT)	
	cerr << "element Var = " << g_vElementLast << endl;
#endif
    if (g_vElementLast) g_vElementLast->setlParam(g_cElement++);
    g_cKey = 0;
	g_fElementPrec += 0.01;		// increment
}

void _UseKey(char *psz)
{
    if (g_vElementLast)
    {
        Variable *pvar = g_pfilter->RegisterQuery(g_vElementLast, psz, true, g_fElementPrec);
#if defined(XMLTK_OUTPUT)	
	cerr << "key Var = " << pvar << endl;
#endif
        if (pvar) pvar->setlParam(g_cKey++);
        g_cMaxKeys = max(g_cKey, g_cMaxKeys);
    }
    else
    {
        fprintf(stderr, "error, key \"%s\" specified without element\n",
                psz);
        exit(1);
    }
}

ITypeFilter *g_ptype = NULL;

void _UseType(char *psz)
{
    if (g_ptype == NULL)
    {
        CreateTypeFilter(&IID_ITypeFilter, (void**)&g_ptype);
    }
    g_ptype->RegisterType(psz);
}

void _UseFile(char *psz)
{
    g_pszFileName = strdup(psz);
}

void _SortFile()
{
    //
    // create a file stream object for this file
    //
    IFileStream *pstm = _CreateFileStream(g_pszFileName);
    if (pstm)
    {
        ITSAXContentHandler *pchOut = CreateStdoutStream(g_bBinary, g_iIndent);

        if (pchOut)
        {
            //
            // create the sorter object
            //
            CSortHandler *psort = new CSortHandler(pchOut);
            if (psort)
            {
                //
                // parse, sort, and output!
                //
                IUnknownCL_SetHandler(g_pfilter, psort);
                if (g_ptype)
                {
                    IUnknownCL_SetHandler(g_ptype, g_pfilter);

                    //
                    // we have a type-filter.  parse to that. 
                    //
                    ParseUnknownStream(pstm, g_ptype);
                }
                else
                {
                    //
                    // parse to dfa-filter
                    //
                    ParseUnknownStream(pstm, g_pfilter);
                }
                psort->Release();
            }
            pchOut->Release();
        }

        pstm->Release();
    }
}

int main(int argc, char* argv[])
{
    InitGlobalTokenTable();
    
    if (CreateDfaFilter(&IID_IDfaFilter, (void**)&g_pfilter))
    {
        g_pszProgName = *argv;
        g_targv = argv + 1;
        g_arglim = argv + argc;
    
        try
        {
            g_pvRoot = g_pfilter->RegisterQuery(NULL, "/", true);
#if defined(XMLTK_OUTPUT)	
	cerr << "root Var = " << g_pvRoot << endl;
#endif
            //
            // mylex drives everything via callbacks _UseContext etc
            //
            mylex();
           
            _SortFile();
        }
        catch (_Error &err)
        {
            err.perror();
        }
        
        ATOMICRELEASE(g_pfilter);
        ATOMICRELEASE(g_ptype);

        if (g_pszFileName)
            free(g_pszFileName);
    }

    CleanupGlobalTokenTable();
}

