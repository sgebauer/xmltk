#ifndef _XMLTK_H
#define _XMLTK_H

#ifdef WIN32
#pragma warning(disable:4786)
#endif

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <assert.h>
#include "stl.h"
#include "xmltkobj.h"

#if !defined(__XPATH_H__) && !defined(__AUTOMATA_H__)
#include "../xpathDFA/Variable.h"
#endif  // __NODE_H__ && __AUTOMATA_H__

#define ARRAYSIZE(A)    (sizeof(A)/sizeof((A)[0]))
#define min(a, b)  (((a) < (b)) ? (a) : (b)) 
#define max(a, b)  (((a) > (b)) ? (a) : (b)) 
#define ATOMICRELEASE(punk) { if (punk) { (punk)->Release(); (punk) = NULL; } }

bool CreateTSAX2Bin(RCLIID riid, void **ppvObj);
bool CreateTSAX2XML(RCLIID riid, void **ppvObj, FILE *pfOut, int iIndent = 0);
bool CreateXML2TSAX(RCLIID riid, void **ppvObj);
bool CreateBin2TSAX(RCLIID riid, void **ppvObj);
bool CreateDfaFilter(RCLIID rcliid, void **ppvObj);
bool CreateDfaFilter(RCLIID rcliid, void **ppvObj, char * fname);
bool CreateFileStream(RCLIID rcliid, void **ppvObj);
bool CreateMemoryStream(RCLIID rcliid, void **ppvObj);
bool CreateTypeFilter(RCLIID rcliid, void **ppvObj);
bool CreateTSAX2Nil(RCLIID rcliid, void **ppvObj);

extern ITokenTable *g_ptt;

//
// NOTE: CreateXPathSMFactory takes only XPath expressions
// without qualifiers or XPath expressions with qualifiers on
// attributes only.  e.g., /bib/book or /bib/book[@id] but not
// /bib/book[/author].  For the latter expression, you must
// first parse it into two fragments, then create a factory
// for each fragment.
//
IClassFactoryCL *CreateXPathSMFactory(char *pszXPath);

BOOL IsEqualCLIID(const CLIID *riid1, const CLIID *riid2);

bool freadMultiByteUINT(IStream *pstm, unsigned int *puInt);
void fwriteMultiByteUINT(IStream *pstm, unsigned int uInt);

void funread(void *buffer, size_t size, size_t count, FILE *stream);

#ifdef DEBUG
void myassert(bool bCondition);
#else
#define myassert(b) {}
#endif  // DEBUG

int mystrcmp(char *psz1, char *psz2);
char* mystrdup(char *psz);
size_t mystrlen(char *psz);

int GetVariableDepth(Variable *pv);
int QueryProcessMemoryUsage();

void ParseUnknownStream(IStream *pstm, ITSAXContentHandler *pch);
// IParse2TSAX * CreateIParseTSAX();
void ParseStream(IStream *pstm, ITSAXContentHandler *pch);
ITSAXContentHandler *CreateStdoutStream(bool bBinary, int iIndent = 0);
IFileStream *_CreateFileStream(char *psz);

bool IUnknownCL_SetHandler(IUnknownCL *punk, IUnknownCL *punkHandler);

#endif  // _XMLTK_H
