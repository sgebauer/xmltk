%{
/**************************************************************
*
* command-line parser for xdelete
*
**************************************************************/

extern char *g_pszProgName;
extern bool g_bBinary;
int myinput(char *pszBuf, int cchMax);

void _UseElement(char *psz);
void _UseFile(char *psz);
void usage();

#undef YY_INPUT
#define YY_INPUT(pszBuf, result, cchBuf) (result = myinput(pszBuf, cchBuf))
#undef mywrap
#define mywrap()    1

%}

%s CONTEXT
%s ELEMENT
%x DONE

%%

-h      |
"-?"    |
-help   { usage(); exit(0); }

-b      |
-binary { g_bBinary = true; BEGIN 0; }

-e          |
-element    { BEGIN ELEMENT; }
<ELEMENT>[^ ]+ { _UseElement(yytext); BEGIN 0; }

-[^ ]+  { fprintf(stderr, "unrecognized option \"%s\"\n", yytext); 
            usage(); exit(1); }

[^ ]+   { _UseFile(yytext); BEGIN 0; }

.       {}
%%

/**************************************************************
*
* C code section
*
**************************************************************/

#include <string.h>

extern char **g_targv;
extern char **g_arglim;

static unsigned int s_uOffset = 0;

void usage()
{
     fprintf(stderr,
        "usage: %s [-help | -h | -? ] [-binary | -b ] "
        "(-element | -e) xpath-expr "
        "[FILE]\n",
        g_pszProgName);
}

int myinput(char *pszBuf, int cchBuf)   /* parse input from command line */
{
    int len, copylen;

    if (g_targv >= g_arglim)
    {
        return (0); /* EOF */
    }
    
    len = strlen(*g_targv) - s_uOffset;
    copylen = (len >= cchBuf) ? cchBuf-1 : len;

    if (len > 0)
    {
        memcpy(pszBuf, g_targv[0]+s_uOffset, copylen);
    }
    
    if (g_targv[0][s_uOffset+copylen] == '\0') /* end of arg */
    {
        pszBuf[copylen] = ' ';
        copylen++;
        s_uOffset = 0;
        g_targv++;
    }

    return copylen;
}

