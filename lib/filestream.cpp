#include "xmltk.h"

class CFileStream : public IFileStream
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

    // *** IFileStream methods ***
    CL_STDMETHOD_(int, CloseFile) ();
    CL_STDMETHOD_(FILE*, GetFile) ();
    CL_STDMETHOD(SetFile) (FILE* pfile);
    CL_STDMETHOD_(char*, GetFileName) ();
    CL_STDMETHOD(OpenFile) (char *pszFile, char *pszMode);
    
    CFileStream() : m_cRef(1), m_pfile(NULL), m_pszFile(NULL) {}
    
private:

    virtual ~CFileStream()
    {
        if (m_pszFile)
        {
            //
            // in this case we own the file handle too, so close it
            //
            if (m_pfile)
                fclose(m_pfile);

            free(m_pszFile);
        }
    }
    
    ULONG m_cRef;
    FILE *m_pfile;
    char *m_pszFile;
};

// *** IUnknown methods ***
// 
bool CFileStream::QueryInterface(RCLIID riid, void **ppvObj)
{
    if (IsEqualCLIID(riid, &IID_IUnknownCL) ||
        IsEqualCLIID(riid, &IID_IStream) ||
        IsEqualCLIID(riid, &IID_IFileStream))
    {
        *ppvObj = static_cast<IFileStream*>(this);
    }
    else
    {
        *ppvObj = NULL;
        return false;
    }
    AddRef();
    return true;
}

ULONG CFileStream::AddRef()
{
    return ++m_cRef;
}

ULONG CFileStream::Release()
{
    if (--m_cRef > 0)
        return m_cRef;

    delete this;
    return 0;
}

// *** IStream methods ***

long CFileStream::CopyTo(IStream *pstm, long cb)
{
    myassert(0);    // not implemnentd
    return 0;    
}

long CFileStream::Peek(void *pv, long cb)
{
    long lRet = fread(pv, cb, 1, m_pfile);
    funread(pv, cb, 1, m_pfile);
    return lRet;
}

long CFileStream::Read(void *pv, long cb)
{
    return fread(pv, cb, 1, m_pfile);
}

int CFileStream::ReadChar()
{
    return fgetc(m_pfile);
}

int CFileStream::Seek(long offset, int origin)
{
    return fseek(m_pfile, offset, origin);
}

long CFileStream::SetSize(long lSize)
{
    //
    // we don't actually resize the file, but we at least
    // update the current position
    //
    long lSizeCur = ftell(m_pfile);
    if (lSize < lSizeCur)
    {
        fseek(m_pfile, SEEK_CUR, lSizeCur - lSize);
    }
    return true;
}

long CFileStream::GetSize()
{
    return Tell();
}

long CFileStream::Tell()
{
    return ftell(m_pfile);
}

long CFileStream::Write(void *pv, long cb)
{
    return fwrite(pv, cb, 1, m_pfile);
}

bool CFileStream::WriteChar(int ch)
{
    return fputc(ch, m_pfile) != EOF;
}

// *** IFileStream methods ***
int CFileStream::CloseFile()
{
    int iRet = EOF;
    if (m_pfile)
    {
        iRet = fclose(m_pfile);
        m_pfile = NULL;
    }
    return iRet;
}

FILE *CFileStream::GetFile()
{
    return m_pfile;
}

bool CFileStream::SetFile(FILE *pfile)
{
    m_pfile = pfile;
    return true;
}

char *CFileStream::GetFileName()
{
    return m_pszFile;
}

bool CFileStream::OpenFile(char *pszName, char *pszMode)
{
    if (m_pszFile)
    {
        free(m_pszFile);
        if (m_pfile)
            fclose(m_pfile);
    }

    m_pszFile = mystrdup(pszName);
    m_pfile = fopen(m_pszFile, pszMode);

    return m_pfile != NULL;
}

bool CreateFileStream(RCLIID riid, void **ppvObj)
{
    bool bRet = false;
    
    CFileStream* pfs = new CFileStream();
    if (pfs)
    {
        bRet = pfs->QueryInterface(riid, ppvObj);
        pfs->Release();
    }
    
    return bRet;
}

