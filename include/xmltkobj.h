#ifndef _XMLTKOBJ_H
#define _XMLTKOBJ_H

////////////////////////////////////////////////////////////////////////////////
//
// home-grown data types
//
////////////////////////////////////////////////////////////////////////////////

typedef unsigned int XTOKEN;
typedef unsigned int UINT;

#define XST_ELEMENT     0x00
#define XST_ATTRIBUTE   0x01
#define XST_EXTENDEDINT 0x02
#define XST_VALID       (XST_ELEMENT | XST_ATTRIBUTE | XST_EXTENDEDINT)

#define XT_END          0x00
#define XT_TABLE        0x01
#define XT_STRING       0x02
#define XT_CDATA        0x03
#define XT_UNKNOWN      0x04

#define XT_FIRST        0x08    // start of range for dynamic (vs hardcoded) tokens

typedef struct
{
    unsigned int uVersion;
}
XBINHEADER, *PXBINHEADER;

#define XBIN_VERSION    1


////////////////////////////////////////////////////////////////////////////////
//
// interfaces use "COM-Lite"
//
// lightweight, portable interface model inspired by Win32 COM
//
////////////////////////////////////////////////////////////////////////////////


typedef struct
{
    unsigned long Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char Data4[8];
}
CLIID, *PCLIID;

typedef const CLIID*    RCLIID;

//
// misc required types
//
typedef unsigned long ULONG;
typedef unsigned long DWORD;
typedef int BOOL;


#ifdef __cplusplus
    #define EXTERN_C    extern "C"
#else
    #define EXTERN_C    extern
#endif  // __cplusplus


#define CL_STDMETHODIMP            bool 
#define CL_STDMETHODIMP_(type)     type 


#define interface               struct



#if defined(__cplusplus) && !defined(CINTERFACE)

#define CL_STDMETHOD(method)       virtual bool  method
#define CL_STDMETHOD_(type,method) virtual type  method
#define PURE                    = 0
#define THIS_
#define THIS                    void
#define CL_DECLARE_INTERFACE(iface)    interface iface
#define CL_DECLARE_INTERFACE_(iface, baseiface)    interface iface : public baseiface


#else

#define CL_STDMETHOD(method)       bool ( * method)
#define CL_STDMETHOD_(type,method) type ( * method)
#define PURE
#define THIS_                   CL_INTERFACE *This,
#define THIS                    CL_INTERFACE *This

#ifdef CONST_VTABLE
#define CL_DECLARE_INTERFACE(iface)    typedef interface iface { \
                                    const struct iface##Vtbl * lpVtbl; \
                                } iface; \
                                typedef const struct iface##Vtbl iface##Vtbl; \
                                const struct iface##Vtbl
#else
#define CL_DECLARE_INTERFACE(iface)    typedef interface iface { \
                                    struct iface##Vtbl * lpVtbl; \
                                } iface; \
                                typedef struct iface##Vtbl iface##Vtbl; \
                                struct iface##Vtbl
#endif  // CONST_VTABLE

#define CL_DECLARE_INTERFACE_(iface, baseiface)    CL_DECLARE_INTERFACE(iface)


#endif  // defined(__cplusplus) && !defined(CINTERFACE)


////////////////////////////////////////////////////////////////
//
// Some standard interface definitions (including IUnknownCL)
//
////////////////////////////////////////////////////////////////


#undef  CL_INTERFACE
#define CL_INTERFACE   IUnknownCL

CL_DECLARE_INTERFACE(IUnknownCL)
{
    // *** IUnknownCL methods ***
    CL_STDMETHOD(QueryInterface) (THIS_ RCLIID rcliid, void **ppvObj) PURE;
    CL_STDMETHOD_(ULONG,AddRef) (THIS) PURE;
    CL_STDMETHOD_(ULONG,Release) (THIS) PURE;
};

// {A1D3A5FF-A5BC-46e7-9116-22CF75BBBDE4}
static const CLIID IID_IUnknownCL = 
{ 0xa1d3a5ff, 0xa5bc, 0x46e7, { 0x91, 0x16, 0x22, 0xcf, 0x75, 0xbb, 0xbd, 0xe4 } };

#undef  CL_INTERFACE
#define CL_INTERFACE   IClassFactoryCL

CL_DECLARE_INTERFACE_(IClassFactoryCL, IUnknownCL)
{
    // *** IUnknownCL methods ***
    CL_STDMETHOD(QueryInterface) (THIS_ RCLIID rcliid, void **ppvObj) PURE;
    CL_STDMETHOD_(ULONG,AddRef) (THIS) PURE;
    CL_STDMETHOD_(ULONG,Release) (THIS) PURE;
    
    // *** IClassFactoryCL methods ***
    CL_STDMETHOD(CreateInstance) (THIS_ IUnknownCL **pUnkOuter, CLIID* pid, void **ppvObj) PURE;
};

// {95488EE9-1269-4cd1-A9A2-6CFDCCBE56FE}
static const CLIID IID_IClassFactoryCL = 
{ 0x95488ee9, 0x1269, 0x4cd1, { 0xa9, 0xa2, 0x6c, 0xfd, 0xcc, 0xbe, 0x56, 0xfe } };



#undef  CL_INTERFACE
#define CL_INTERFACE   IObjectWithSiteCL

CL_DECLARE_INTERFACE_(IObjectWithSiteCL, IUnknownCL)
{
    // *** IUnknownCL methods ***
    CL_STDMETHOD(QueryInterface) (THIS_ RCLIID rcliid, void **ppvObj) PURE;
    CL_STDMETHOD_(ULONG,AddRef) (THIS) PURE;
    CL_STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IObjectWithSiteCL methods ***
    CL_STDMETHOD(SetSite)(THIS_ IUnknownCL *punkSite) PURE;
    CL_STDMETHOD(GetSite)(THIS_ CLIID* pid, void **ppvSite) PURE;
};

// {7D5DF326-2E1E-46fc-A975-3FF31EAAFE6F}
static const CLIID IID_IObjectWithSiteCL = 
{ 0x7d5df326, 0x2e1e, 0x46fc, { 0xa9, 0x75, 0x3f, 0xf3, 0x1e, 0xaa, 0xfe, 0x6f } };



#undef  CL_INTERFACE
#define CL_INTERFACE   ITokenTable

typedef pair<char*, char*>  STRPAIR;

CL_DECLARE_INTERFACE_(ITokenTable, IUnknownCL)
{
    // *** IUnknownCL methods ***
    CL_STDMETHOD(QueryInterface) (THIS_ RCLIID riid, void **ppvObj) PURE;
    CL_STDMETHOD_(ULONG,AddRef) (THIS) PURE;
    CL_STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** ITokenTable methods ***
    CL_STDMETHOD_(char*, StrFromXTOKEN) (THIS_ XTOKEN xt) PURE;
    CL_STDMETHOD_(DWORD, TypeFromXTOKEN) (THIS_ XTOKEN xt) PURE;
    CL_STDMETHOD_(STRPAIR*, PairFromXTOKEN) (THIS_ XTOKEN xt) PURE;
    CL_STDMETHOD_(XTOKEN, XTOKENFromStr)  (THIS_ char* psz, DWORD dwType) PURE;
    CL_STDMETHOD_(XTOKEN, XTOKENFromPair) (THIS_ char* pszFirst, char *pszSecond, DWORD dwType) PURE;
};

void InitGlobalTokenTable();
void CleanupGlobalTokenTable();
bool GetGlobalTokenTable(RCLIID riid, void** ppv);  // return a pointer to the global token table


// {E71FCED6-E2F5-49a2-9191-630155B94CF4}
static const CLIID IID_ITokenTable = 
{ 0xe71fced6, 0xe2f5, 0x49a2, { 0x91, 0x91, 0x63, 0x1, 0x55, 0xb9, 0x4c, 0xf4 } };



////////////////////////////////////////////////////////////////////////////////
//
// ITSAXContentHandler
//
///////////////////////////////////////////////////////////////////////////////


#undef  CL_INTERFACE
#define CL_INTERFACE   ITSAXContentHandler

CL_DECLARE_INTERFACE_(ITSAXContentHandler, IUnknownCL)
{
    // *** IUnknownCL methods ***
    CL_STDMETHOD(QueryInterface) (THIS_ RCLIID riid, void **ppvObj) PURE;
    CL_STDMETHOD_(ULONG,AddRef) (THIS) PURE;
    CL_STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** ITSAXContentHandler methods ***
    CL_STDMETHOD(startDocument) (THIS) PURE;
    CL_STDMETHOD(endDocument) (THIS) PURE;
    CL_STDMETHOD(startElement) (THIS_ XTOKEN xtName) PURE;
    CL_STDMETHOD(endElement) (THIS_ XTOKEN xtName) PURE;
    CL_STDMETHOD(attribute) (THIS_ XTOKEN xtName, char *pszChars, int cchChars) PURE;
    CL_STDMETHOD(characters) (THIS_ char *pszChars, int cchChars) PURE;
    CL_STDMETHOD(cdata) (THIS_ char *pszChars, int cchChars) PURE;
    CL_STDMETHOD(extendedint) (THIS_ XTOKEN xt, int iInt) PURE;
};


// {6C25AE7A-1E37-46ea-A2FC-6872E5CC45B8}
static const CLIID IID_ITSAXContentHandler = 
{ 0x6c25ae7a, 0x1e37, 0x46ea, { 0xa2, 0xfc, 0x68, 0x72, 0xe5, 0xcc, 0x45, 0xb8 } };




#undef  CL_INTERFACE
#define CL_INTERFACE    IFilteredTSAXHandler

class Variable;

CL_DECLARE_INTERFACE_(IFilteredTSAXHandler, ITSAXContentHandler)
{
    // *** IUnknownCL methods ***
    CL_STDMETHOD(QueryInterface) (THIS_ RCLIID riid, void **ppvObj) PURE;
    CL_STDMETHOD_(ULONG,AddRef) (THIS) PURE;
    CL_STDMETHOD_(ULONG,Release) (THIS) PURE;
    
    // *** ITSAXContentHandler methods ***
    CL_STDMETHOD(startDocument) (THIS) PURE;
    CL_STDMETHOD(endDocument) (THIS) PURE;
    CL_STDMETHOD(startElement) (THIS_ XTOKEN xtName) PURE;
    CL_STDMETHOD(endElement) (THIS_ XTOKEN xtName) PURE;
    CL_STDMETHOD(attribute) (THIS_ XTOKEN xtName, char *pszChars, int cchChars) PURE;
    CL_STDMETHOD(characters) (THIS_ char *pszChars, int cchChars) PURE;
    CL_STDMETHOD(cdata) (THIS_ char *pszChars, int cchChars) PURE;
    CL_STDMETHOD(extendedint) (THIS_ XTOKEN xt, int iInt) PURE;

    // *** IFilteredTSAXHandler methods ***
    CL_STDMETHOD(startContext) (THIS_ Variable *pv) PURE;
    CL_STDMETHOD(endContext) (THIS_ Variable *pv) PURE;
};

// {08FE5554-A526-4835-A5D8-4ECEEB21D50D}
static const CLIID IID_IFilteredTSAXHandler = 
{ 0x8fe5554, 0xa526, 0x4835, { 0xa5, 0xd8, 0x4e, 0xce, 0xeb, 0x21, 0xd5, 0xd } };

#undef  CL_INTERFACE
#define CL_INTERFACE    IDfaFilter

CL_DECLARE_INTERFACE_(IDfaFilter, ITSAXContentHandler)
{
    // *** IUnknownCL methods ***
    CL_STDMETHOD(QueryInterface) (THIS_ RCLIID riid, void **ppvObj) PURE;
    CL_STDMETHOD_(ULONG,AddRef) (THIS) PURE;
    CL_STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** ITSAXContentHandler methods ***
    CL_STDMETHOD(startDocument) (THIS) PURE;
    CL_STDMETHOD(endDocument) (THIS) PURE;
    CL_STDMETHOD(startElement) (THIS_ XTOKEN xtName) PURE;
    CL_STDMETHOD(endElement) (THIS_ XTOKEN xtName) PURE;
    CL_STDMETHOD(attribute) (THIS_ XTOKEN xtName, char *pszChars, int cchChars) PURE;
    CL_STDMETHOD(characters) (THIS_ char *pszChars, int cchChars) PURE;
    CL_STDMETHOD(cdata) (THIS_ char *pszChars, int cchChars) PURE;
    CL_STDMETHOD(extendedint) (THIS_ XTOKEN xt, int iInt) PURE;

    // *** IDfaFilter methods ***
    CL_STDMETHOD_(Variable*,RegisterQuery) (THIS_ Variable *pv, const char *psz, bool bEvents) PURE;
    CL_STDMETHOD_(Variable*,RegisterQuery) (THIS_ Variable *pv, const char *psz, bool bEvents, float fPrec) PURE;
    CL_STDMETHOD(RegisterQueryFile) (THIS_ const char *psz) PURE;
};

// {08FE5554-A526-4835-A5D8-4ECEEB21D50C}
static const CLIID IID_IDfaFilter = 
{ 0x8fe5554, 0xa526, 0x4835, { 0xa5, 0xd8, 0x4e, 0xce, 0xeb, 0x21, 0xd5, 0xc } };



#undef  CL_INTERFACE
#define CL_INTERFACE   ITypeFilter


CL_DECLARE_INTERFACE_(ITypeFilter, ITSAXContentHandler)
{
    // *** IUnknownCL methods ***
    CL_STDMETHOD(QueryInterface) (THIS_ RCLIID riid, void **ppvObj) PURE;
    CL_STDMETHOD_(ULONG,AddRef) (THIS) PURE;
    CL_STDMETHOD_(ULONG,Release) (THIS) PURE;
    
    // *** ITSAXContentHandler methods ***
    CL_STDMETHOD(startDocument) (THIS) PURE;
    CL_STDMETHOD(endDocument) (THIS) PURE;
    CL_STDMETHOD(startElement) (THIS_ XTOKEN xtName) PURE;
    CL_STDMETHOD(endElement) (THIS_ XTOKEN xtName) PURE;
    CL_STDMETHOD(attribute) (THIS_ XTOKEN xtName, char *pszChars, int cchChars) PURE;
    CL_STDMETHOD(characters) (THIS_ char *pszChars, int cchChars) PURE;
    CL_STDMETHOD(cdata) (THIS_ char *pszChars, int cchChars) PURE;
    CL_STDMETHOD(extendedint) (THIS_ XTOKEN xt, int iInt) PURE;

    // *** ITypeFilter methods ***
    CL_STDMETHOD(RegisterType) (THIS_ char *pszType) PURE;
};

// {808EA8E8-4DB3-4729-9DDD-11BCBCE9A046}
static const CLIID IID_ITypeFilter = 
{ 0x808ea8e8, 0x4db3, 0x4729, { 0x9d, 0xdd, 0x11, 0xbc, 0xbc, 0xe9, 0xa0, 0x46 } };

#undef  CL_INTERFACE
#define CL_INTERFACE    ISetHandler

CL_DECLARE_INTERFACE_(ISetHandler, IUnknownCL)
{
    // *** IUnknownCL methods ***
    CL_STDMETHOD(QueryInterface) (THIS_ RCLIID riid, void **ppvObj) PURE;
    CL_STDMETHOD_(ULONG,AddRef) (THIS) PURE;
    CL_STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** ISetHandler methods ***
    CL_STDMETHOD(SetHandler) (THIS_ IUnknownCL *punk) PURE;
};

// {08FE4534-A526-4835-A5D8-4ECEEB21D50F}
static const CLIID IID_ISetHandler = 
{ 0x8fe4534, 0xa526, 0x4835, { 0xa5, 0xd8, 0x4e, 0xce, 0xeb, 0x21, 0xd5, 0xf } };

#undef  CL_INTERFACE
#define CL_INTERFACE    IStream

CL_DECLARE_INTERFACE_(IStream, IUnknownCL)
{
    // *** IUnknownCL methods ***
    CL_STDMETHOD(QueryInterface) (THIS_ RCLIID riid, void **ppvObj) PURE;
    CL_STDMETHOD_(ULONG,AddRef) (THIS) PURE;
    CL_STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IStream methods ***
    CL_STDMETHOD_(long, CopyTo) (THIS_ IStream *pstm, long cb) PURE;
    CL_STDMETHOD_(long, Peek) (THIS_ void *pv, long cb) PURE;
    CL_STDMETHOD_(long, Read) (THIS_ void *pv, long cb) PURE;
    CL_STDMETHOD_(int, ReadChar) (THIS) PURE;
    CL_STDMETHOD_(int, Seek) (THIS_ long offset, int origin) PURE;
    CL_STDMETHOD_(long, SetSize) (THIS_ long lsize) PURE;
    CL_STDMETHOD_(long, GetSize) (THIS) PURE;
    CL_STDMETHOD_(long, Tell) (THIS) PURE;
    CL_STDMETHOD_(long, Write) (THIS_ void *pv, long cb) PURE;
    CL_STDMETHOD(WriteChar) (THIS_ int ch) PURE;
};

// {08FE5554-A526-4835-A5D8-4ECEEB21D50D}
static const CLIID IID_IStream = 
{ 0x8fe5554, 0xa526, 0x4835, { 0xa5, 0xd8, 0x4e, 0xce, 0xeb, 0x21, 0xd5, 0xd } };

#undef  CL_INTERFACE
#define CL_INTERFACE    IFileStream

CL_DECLARE_INTERFACE_(IFileStream, IStream)
{
    // *** IUnknownCL methods ***
    CL_STDMETHOD(QueryInterface) (THIS_ RCLIID riid, void **ppvObj) PURE;
    CL_STDMETHOD_(ULONG,AddRef) (THIS) PURE;
    CL_STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IStream methods ***
    CL_STDMETHOD_(long, CopyTo) (THIS_ IStream *pstm, long cb) PURE;
    CL_STDMETHOD_(long, Peek) (THIS_ void *pv, long cb) PURE;
    CL_STDMETHOD_(long, Read) (THIS_ void *pv, long cb) PURE;
    CL_STDMETHOD_(int, ReadChar) (THIS) PURE;
    CL_STDMETHOD_(int, Seek) (THIS_ long offset, int origin) PURE;
    CL_STDMETHOD_(long, SetSize) (THIS_ long lsize) PURE;
    CL_STDMETHOD_(long, GetSize) (THIS) PURE;
    CL_STDMETHOD_(long, Tell) (THIS) PURE;
    CL_STDMETHOD_(long, Write) (THIS_ void *pv, long cb) PURE;
    CL_STDMETHOD(WriteChar) (THIS_ int ch) PURE;

    // *** IFileStream methods ***
    CL_STDMETHOD_(int, CloseFile) (THIS) PURE;
    CL_STDMETHOD_(FILE*, GetFile) (THIS) PURE;
    CL_STDMETHOD(SetFile) (THIS_ FILE *pfile) PURE;
    CL_STDMETHOD_(char*, GetFileName) (THIS) PURE;
    CL_STDMETHOD(OpenFile) (char *pszFile, char *pszMode) PURE;
};

// {08FE5554-A526-4835-A5D8-4ECEEB21D510}
static const CLIID IID_IFileStream = 
{ 0x8fe5554, 0xa526, 0x4835, { 0xa5, 0xd8, 0x4e, 0xce, 0xeb, 0x21, 0xd5, 0x10 } };

#undef  CL_INTERFACE
#define CL_INTERFACE    IMemoryStream

CL_DECLARE_INTERFACE_(IMemoryStream, IStream)
{
    // *** IUnknownCL methods ***
    CL_STDMETHOD(QueryInterface) (THIS_ RCLIID riid, void **ppvObj) PURE;
    CL_STDMETHOD_(ULONG,AddRef) (THIS) PURE;
    CL_STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IStream methods ***
    CL_STDMETHOD_(long, CopyTo) (THIS_ IStream *pstm, long cb) PURE;
    CL_STDMETHOD_(long, Peek) (THIS_ void *pv, long cb) PURE;
    CL_STDMETHOD_(long, Read) (THIS_ void *pv, long cb) PURE;
    CL_STDMETHOD_(int, ReadChar) (THIS) PURE;
    CL_STDMETHOD_(int, Seek) (THIS_ long offset, int origin) PURE;
    CL_STDMETHOD_(long, SetSize) (THIS_ long lsize) PURE;
    CL_STDMETHOD_(long, GetSize) (THIS) PURE;
    CL_STDMETHOD_(long, Tell) (THIS) PURE;
    CL_STDMETHOD_(long, Write) (THIS_ void *pv, long cb) PURE;
    CL_STDMETHOD(WriteChar) (THIS_ int ch) PURE;

    // *** IMemoryStream methods ***
    CL_STDMETHOD_(int, getSize) (THIS) PURE;
    CL_STDMETHOD_(char*, getBuffer) (THIS) PURE;
    CL_STDMETHOD(setBuffer) (THIS_ void *pv, int iSize) PURE;
    CL_STDMETHOD_(int, getGrowSize) (THIS) PURE;
    CL_STDMETHOD(setGrowSize) (THIS_ int cchGrow) PURE;
};

// {08FE5554-A526-4835-A5D8-4ECEEB21D50E}
static const CLIID IID_IMemoryStream = 
{ 0x8fe5554, 0xa526, 0x4835, { 0xa5, 0xd8, 0x4e, 0xce, 0xeb, 0x21, 0xd5, 0xe } };

#undef  CL_INTERFACE
#define CL_INTERFACE   IParse2TSAX


CL_DECLARE_INTERFACE_(IParse2TSAX, IUnknownCL)
{
    // *** IUnknownCL methods ***
    CL_STDMETHOD(QueryInterface) (THIS_ RCLIID riid, void **ppvObj) PURE;
    CL_STDMETHOD_(ULONG,AddRef) (THIS) PURE;
    CL_STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IParse2TSAX methods ***
    CL_STDMETHOD(Parse) (THIS_ IStream *pstm, ITSAXContentHandler *pch) PURE;
    CL_STDMETHOD_(unsigned int,getSrcOffset) (THIS_) PURE;
    CL_STDMETHOD(skip) (THIS_ unsigned int skipDepth) PURE;
};

// {08FE5554-A526-4835-A5D8-4ECEEB21D50B}
static const CLIID IID_IParse2TSAX = 
{ 0x8fe5554, 0xa526, 0x4835, { 0xa5, 0xd8, 0x4e, 0xce, 0xeb, 0x21, 0xd5, 0xb } };

extern IParse2TSAX * g_xmlparse;			// for SIX operation

#undef  CL_INTERFACE
#define CL_INTERFACE    IBin2TSAX

CL_DECLARE_INTERFACE_(IBin2TSAX, IParse2TSAX)
{
    // *** IUnknownCL methods ***
    CL_STDMETHOD(QueryInterface) (THIS_ RCLIID riid, void **ppvObj) PURE;
    CL_STDMETHOD_(ULONG,AddRef) (THIS) PURE;
    CL_STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IParse2TSAX methods ***
    CL_STDMETHOD(Parse) (THIS_ IStream *pstm, ITSAXContentHandler *pch) PURE;
    CL_STDMETHOD_(unsigned int,getSrcOffset) (THIS_) PURE;
    CL_STDMETHOD(skip) (THIS_ unsigned int skipDepth) PURE;
    
    // *** IBin2TSAX methods ***
    CL_STDMETHOD(Init) (THIS_ bool bExternalStream) PURE;
};

// {08FE5554-A526-4835-A5D8-4ECEEB21D511}
static const CLIID IID_IBin2TSAX = 
{ 0x8fe5554, 0xa526, 0x4835, { 0xa5, 0xd8, 0x4e, 0xce, 0xeb, 0x21, 0xd5, 0x11 } };


#undef  CL_INTERFACE
#define CL_INTERFACE    ITSAX2Bin

CL_DECLARE_INTERFACE_(ITSAX2Bin, ITSAXContentHandler)
{
    // *** IUnknownCL methods ***
    CL_STDMETHOD(QueryInterface) (THIS_ RCLIID riid, void **ppvObj) PURE;
    CL_STDMETHOD_(ULONG,AddRef) (THIS) PURE;
    CL_STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** ITSAXContentHandler methods ***
    CL_STDMETHOD(startDocument) (THIS) PURE;
    CL_STDMETHOD(endDocument) (THIS) PURE;
    CL_STDMETHOD(startElement) (THIS_ XTOKEN xtName) PURE;
    CL_STDMETHOD(endElement) (THIS_ XTOKEN xtName) PURE;
    CL_STDMETHOD(attribute) (THIS_ XTOKEN xtName, char *pszChars, int cchChars) PURE;
    CL_STDMETHOD(characters) (THIS_ char *pszChars, int cchChars) PURE;
    CL_STDMETHOD(cdata) (THIS_ char *pszChars, int cchChars) PURE;
    CL_STDMETHOD(extendedint) (THIS_ XTOKEN xt, int iInt) PURE;

    // *** ITSAX2Bin methods ***
    CL_STDMETHOD(Init) (THIS_ IStream* pws, bool bEmitTableEntries) PURE;
};

// {08FE5554-A526-4835-A5D8-4ECEEB21D50F}
static const CLIID IID_ITSAX2Bin = 
{ 0x8fe5554, 0xa526, 0x4835, { 0xa5, 0xd8, 0x4e, 0xce, 0xeb, 0x21, 0xd5, 0xf } };


#endif  // _XMLTKOBJ_H

