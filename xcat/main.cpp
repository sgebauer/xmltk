#include "xmltk.h"


bool g_bBinary = false;
ITypeFilter *g_ptype = NULL;
int g_iIndent = 0;

int _ParseCommandLine(int argc, char *argv[])
{
    int i;

	//    for (i = 1; i < argc && (*(argv[i]) == '-' || *(argv[i]) == '/'); i++)
    for (i = 1; i < argc && (*(argv[i]) == '-'); i++)
    {
        switch (*(argv[i]+1))
        {
        case 'b':
            g_bBinary = true;
            break;
        
        case 't':
            {
                //
                // skip to type-expr
                //
                i++;

                if (g_ptype == NULL)
                {
                    CreateTypeFilter(&IID_ITypeFilter, (void**)&g_ptype);
                }
                if (i >= argc || !g_ptype->RegisterType(argv[i]))
                {
                    return 0;   // error
                }
            }
            break;

        case 'i':
            if (++i >= argc)
            {
                return 0;
            }
            else
            {
                g_iIndent = atoi(argv[i]);
            }
            break;

        default:
            return 0;       // error
        }
    }

    return i;
}

bool IUnknownCL_Init(IUnknownCL *punk, IStream* pws, bool bForStdout)
{
    ITSAX2Bin *p2bin;
    bool bRet = punk->QueryInterface(&IID_ITSAX2Bin, (void**)&p2bin);
    if (bRet)
    {
        bRet = p2bin->Init(pws, bForStdout);
        p2bin->Release();
    }
    return bRet;
}

int main(int argc, char *argv[])
{
    InitGlobalTokenTable();

    int iFileArg = _ParseCommandLine(argc, argv);

    if (iFileArg > 0)
    {
        ITSAXContentHandler *pch = NULL;
    
        if (g_bBinary)
        {
            IFileStream* pfs;
            CreateFileStream(&IID_IFileStream, (void**)&pfs);
            if (pfs)
            {
                pfs->SetFile(stdout);
                CreateTSAX2Bin(&IID_ITSAXContentHandler, (void**)&pch);
                if (pch)
                {
                    IUnknownCL_Init(pch, pfs, true);
                }
                pfs->Release();
            }
        }
        else
        {
            CreateTSAX2XML(&IID_ITSAXContentHandler, (void**)&pch, stdout, g_iIndent);
        }

        if (g_ptype)
        {
            IUnknownCL_SetHandler(g_ptype, pch);
            pch->Release();
            g_ptype->QueryInterface(&IID_ITSAXContentHandler, (void**)&pch);
        }

        if (pch)
        {
            IParse2TSAX *pparseXML, *pparseBin;
            CreateXML2TSAX(&IID_IParse2TSAX, (void**)&pparseXML);
            CreateBin2TSAX(&IID_IParse2TSAX, (void**)&pparseBin);

            IFileStream *pfs;
            CreateFileStream(&IID_IFileStream, (void**)&pfs);

            if (pparseXML && pparseBin)
            {
                if (iFileArg < argc)
                {
                    //
                    // use specified files as input
                    //
                    for ( ; iFileArg < argc; iFileArg++)
                    {
                        if (pfs->OpenFile(argv[iFileArg], "r"))
                        {
                            if (!pparseBin->Parse(pfs, pch))
                            {
                                pparseXML->Parse(pfs, pch);
                            }
                            pfs->CloseFile();
                        }
                        else
                        {
                            fprintf(stderr, "error opening %s for read\n",
                                    argv[iFileArg]);
                        }
                    }
                }
                else
                {
                    pfs->SetFile(stdin);
                    if (!pparseBin->Parse(pfs, pch))
                    {
                        pparseXML->Parse(pfs, pch);
                    }
                }
            }

            if (pparseXML)
                pparseXML->Release();
            if (pparseBin)
                pparseBin->Release();

            pch->Release();
        }
    }
    else
    {
        //
        // error, echo usage
        //
        fprintf(stderr, "usage: xcat [-b] [-i indent] [-t type-expr] file ...\n");
    }

    ATOMICRELEASE(g_ptype);
    
    CleanupGlobalTokenTable();

    return 0;
}
