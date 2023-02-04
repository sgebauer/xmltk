#include "xmltk.h"

#ifndef WIN32
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#endif

BOOL IsEqualCLIID(const CLIID *riid1, const CLIID *riid2)
{
    return (memcmp(riid1, riid2, sizeof(*riid1)) == 0);
}


static const unsigned char c_chMask = 0x7F;     // 0x7F = 0111 1111
static const unsigned char c_chContinue = 0x80; // 0x80 = 1000 0000

void fwriteMultiByteUINT(IStream *pstm, unsigned int uInt)
{
    unsigned char rgch[5];                          // 5 = ceil(32/7)
    int i;
    
    for (i = 0; i < ARRAYSIZE(rgch); i++)
    {
        rgch[i] = (uInt & c_chMask) | c_chContinue;
        uInt >>= 7;
        if (uInt <= 0)
        {
            break;
        }
    }

    rgch[0] &= ~c_chContinue;

    for ( ; i >= 0; i--)
    {
        pstm->WriteChar(rgch[i]);
    }
}

bool freadMultiByteUINT(IStream *pstm, unsigned int *puInt)
{
    *puInt = 0;
    int i;

    //
    // use a loop to guard against overflow
    //
    for (i = 0; i < 5; i++)     // 5 = ceil(32/7)
    {
        int ch = pstm->ReadChar();
        if (ch == EOF)
        {
            return false;
        }
        else
        {
            *puInt <<= 7;
            *puInt |= (ch & c_chMask);
            if (!(ch & c_chContinue))
            {
                return true;
            }
        }
    }

    return false;
}

void funread(void *buffer, size_t size, size_t count, FILE *stream)
{
    for (int i = count - 1; i >= 0; i--)
    {
        for (int j = (size/sizeof(char)) - 1; j >= 0; j--)
        {
            void *bufferT = (void*)((size_t)buffer + (size*i));
            int ch = ((char*)bufferT)[j];
            ungetc(ch, stream);
        }
    }
}

#ifdef DEBUG
void myassert(bool bCondition)
{
#ifdef WIN32
    if (!bCondition)
    {
        __asm
        {
            int 3;
        }
    }
#else
    assert(bCondition);
#endif
}
#endif

int mystrcmp(char *psz1, char *psz2)
{
    if (psz1 == NULL)
    {
        return psz2 == NULL ? 0 : 1;
    }
    else if (psz2 == NULL)
    {
        return -1;
    }
    else
    {
        return strcmp(psz1, psz2);
    }
}

char* mystrdup(char *psz)
{
    if (psz == NULL)
    {
        return NULL;
    }
    else
    {
        return strdup(psz);
    }
}

size_t mystrlen(char *psz)
{
    if (psz)
    {
        return strlen(psz);
    }
    else
    {
        return 0;
    }
}

int GetVariableDepth(Variable *pv)
{
    int iDepth = 0;
    while ((pv = pv->getParent()) != NULL)
    {
        ++iDepth;
    }
    return iDepth;
}

//
// magic bytetok calculation taken from the source 
// code for top
//
inline unsigned long bytetok(unsigned long l)
{
    return (l + 512) >> 10;
}

inline unsigned long bytetobyte(unsigned long l)
{
    return l + 512;
}

int _QueryProcFileMemUsage()
{
    int cbUsed = 0;

    char szBuf[4096];
    sprintf(szBuf, "/proc/%d/stat", getpid());
    szBuf[ARRAYSIZE(szBuf)-1] = 0;

    int fd = open(szBuf, O_RDONLY);
    if (fd != -1)
    {
        //
        // my psychic powers tell me that the memory usage stat
        // is the 23rd token
        //
        read(fd, szBuf, sizeof(szBuf)-1);

        szBuf[ARRAYSIZE(szBuf)-1] = 0;

        char *psz = szBuf;
        int i;
        for (i = 0; i < 22; i++)
        {
            psz = strchr(psz, ' ') + 1;
        }
                  
        cbUsed = bytetobyte(strtoul(psz, NULL, 10));

	close(fd);
    }
    
    return cbUsed;
}

//
// QueryProcessMemoryUsage is based on top's read_one_proc_stat
//
int QueryProcessMemoryUsage()
{
#ifdef USE_SBRK
    static unsigned long s_baseproc = _QueryProcFileMemUsage();
    static unsigned long s_baseheap = (unsigned long) sbrk(0);

    return (unsigned long) s_baseproc + (unsigned long) sbrk(0) - s_baseheap;
#else // USE_SBRK

    return _QueryProcFileMemUsage();

#endif // USE_SBRK
}

void ParseUnknownStream(IStream *pstm, ITSAXContentHandler *pch)
{
    //
    // helper to parse a stream that contains either xml or binary data
    //

    //
    // try binary first
    //
    IParse2TSAX *pbin;
    if (CreateBin2TSAX(&IID_IParse2TSAX, (void**)&pbin))
    {
        if (!pbin->Parse(pstm, pch))
        {
            //
            // that didn't work, try xml
            //
            IParse2TSAX *pxml;
            if (CreateXML2TSAX(&IID_IParse2TSAX, (void**)&pxml))
            {
			    g_xmlparse = pxml;
                pxml->Parse(pstm, pch);
                pxml->Release();
			    g_xmlparse = 0;
            }
        }
        pbin->Release();
    }
}

/*
IParse2TSAX * CreateIParseTSAX()
{
    IParse2TSAX *pxml;
	CreateXML2TSAX(&IID_IParse2TSAX, (void**)&pxml);
	return pxml;
}
*/

void _PointToStdout(ITSAXContentHandler *pch)
{
    //
    // QI for ITSAX2Bin, which we use to set the output stream
    //
    ITSAX2Bin *pbin;
    if (pch->QueryInterface(&IID_ITSAX2Bin, (void**)&pbin))
    {
        //
        // create the stdout stream
        //
        IFileStream *pstm;
        if (CreateFileStream(&IID_IFileStream, (void**)&pstm))
        {
            //
            // point the stream to stdout, point pch to stream
            //
            pstm->SetFile(stdout);
            pbin->Init(pstm, true);
            pstm->Release();
        }
        pbin->Release();
    }
}

ITSAXContentHandler *CreateStdoutStream(bool bBinary, int iIndent)
{
    //
    // create an xml or binary output stream, depending on cmd line
    //
    ITSAXContentHandler *pchOut;
    if (bBinary)
    {
        CreateTSAX2Bin(&IID_ITSAXContentHandler, (void**)&pchOut);
        _PointToStdout(pchOut);
    }
    else
    {
        CreateTSAX2XML(&IID_ITSAXContentHandler, (void**)&pchOut, stdout, iIndent);
    }
    return pchOut;
}

IFileStream *_CreateFileStream(char *psz)
{
    //
    // create a file stream object for this file
    //
    IFileStream *pstm = NULL;
    if (CreateFileStream(&IID_IFileStream, (void**)&pstm))
    {
        if (psz)
        {
            if (pstm->OpenFile(psz, "r") == false)
            {
                fprintf(stderr, "error opening %s for read\n", psz);
                pstm->Release();
                pstm = NULL;
            }
        }
        else
        {
            pstm->SetFile(stdin);
        }
    }
    return pstm;
}

bool IUnknownCL_SetHandler(IUnknownCL *punk, IUnknownCL *punkHandler)
{
    ISetHandler* psh;
    bool bRet = punk->QueryInterface(&IID_ISetHandler, (void**)&psh);
    if (bRet)
    {
        bRet = psh->SetHandler(punkHandler);
        psh->Release();
    }
    return bRet;
}

