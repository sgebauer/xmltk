%{
/**************************************************************
*
* command-line parser for xsort
*
**************************************************************/

extern char *g_pszProgName;
extern bool g_bBinary;
extern int g_cbMaxBuf;
extern int g_iIndent;
int myinput(char *pszBuf, int cchMax);

void _UseContext(char *psz);
void _UseElement(char *psz);
void _UseKey(char *psz);
void _UseFile(char *psz);
void _UseType(char *psz);
void usage();

#undef YY_INPUT
#define YY_INPUT(pszBuf, result, cchBuf) (result = myinput(pszBuf, cchBuf))
#undef mywrap
#define mywrap()    1

%}

%s CONTEXT
%s ELEMENT
%s KEY
%s MEMORY
%s TYPE
%s INDENT
%x DONE

%%

-h      |
"-?"    |
-help   { usage(); exit(0); }

-b      |
-binary { g_bBinary = true; BEGIN 0; }

-m      |
-memory { BEGIN MEMORY; }
<MEMORY>[^ ]+ { g_cbMaxBuf = (atoi(yytext) * 1024 * 1024); BEGIN 0; }

-i      |
-indent { BEGIN INDENT; }
<INDENT>[^ ]+ { g_iIndent = atoi(yytext); BEGIN 0; }

-c      |
-context    { BEGIN CONTEXT; }
<CONTEXT>[^ ]+ { _UseContext(yytext); BEGIN 0; }

-e          |
-element    { BEGIN ELEMENT; }
<ELEMENT>[^ ]+ { _UseElement(yytext); BEGIN 0; }

-k      |
-key    { BEGIN KEY; }
<KEY>[^ ]+  { _UseKey(yytext); BEGIN 0; }

-t      |
-type   { BEGIN TYPE; }
<TYPE>[^ ]+ { _UseType(yytext); BEGIN 0; }

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
        "[(-memory | -m) mem-size (MB)] "
        "[(-indent | -i) indent-size]"
        "[(-type | -t) type-expr] "
        "[(-context | -c) xpath-expr] "
        "[(-element | -e) xpath-expr] "
        "[(-key | -k) xpath-expr]]] "
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

