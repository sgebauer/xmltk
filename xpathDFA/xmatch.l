%{
//	This is a lex program for xmatch processor.
//      copyright (C) 2001 Makoto Onizuka, University of Washington
//      $Id: xmatch.l,v 1.3 2002/08/13 12:59:46 monizuka Exp $
//

#include <string.h>
#include <stdlib.h>
#include "xmatch.tab.h"
#include "operator.h"
#ifdef WIN32
#include "xmatch.h"
#endif
extern int my_yyinput(char *, int);
extern int yyerror(char * mes);

#undef YY_INPUT
#define YY_INPUT(buf, result, max_size)(result = my_yyinput(buf, max_size))

int lineCount(char * text, const char * del){
  int num = 0;
  char * top;
  char * target = top = strdup(text);
  while ((target = strstr(target, del))!=NULL){
    target += strlen(del);
    num++;
  }
  free(top);
  return num;
}

%}

%option noyywrap

%e 1200
%a 12000
%o 25000

Digit	        [0-9]
NameChar	[0-9a-zA-Z.\-_:]
TopNameChar	[a-zA-Z_:]

%%

 /* for INTEGER and FLOAT value */
<INITIAL>[0-9]+          {yylval.intval = atoi(yytext);
                         return INTEGER;}
<INITIAL>{Digit}*"."{Digit}+ {yylval.fltval = atof(yytext);
                         return FLOAT;}

 /* for comparison operator */
<INITIAL>"="               {yylval.subtok = MATH_OPR_EQ;  return OPERATOR;}
<INITIAL>"!="              {yylval.subtok = MATH_OPR_NEQ; return OPERATOR;}
<INITIAL>"<"               {yylval.subtok = MATH_OPR_LES; return OPERATOR;}
<INITIAL>">"               {yylval.subtok = MATH_OPR_GRE; return OPERATOR;}
<INITIAL>"<="              {yylval.subtok = MATH_OPR_LEQ; return OPERATOR;}
<INITIAL>">="              {yylval.subtok = MATH_OPR_GEQ; return OPERATOR;}
<INITIAL>"starts-with("	   {yylval.subtok = FUNC_STARTS;  return FUNCTION;}
<INITIAL>"contains("       {yylval.subtok = FUNC_CONTAINS;return FUNCTION;}

 /* for mathematical calculation */
<INITIAL>"div"             {return DIV;}
<INITIAL>"mod"             {return MOD;}

 /* for logical AND OR */
<INITIAL>[aA][nN][dD]      {return AND;}
<INITIAL>[oO][rR]          {return OR;}

 /* functions */
<INITIAL>"text("	   {return TEXT;}

 /* for XPath expression (including mathematical operation) */
<INITIAL>"//"              {return DSLASH;}
<INITIAL>"/"               {return SLASH;}
<INITIAL>[\[\].,()|*+\-@]   {return yytext[0];}

<INITIAL>{TopNameChar}{NameChar}*   {yylval.strval = yytext;
                         yylineno += lineCount(yytext, "\n");
                         return EREF;} /* element simple path */
<INITIAL>@{TopNameChar}{NameChar}*   {yylval.strval = yytext;
                         yylineno += lineCount(yytext, "\n");
                         return AREF;} /* attribute simple path */
 /* lex expression for a registerd function call should come here. */

 /* literal expression */
<INITIAL>'[^'\n]*'          {
#ifndef WIN32
     int c = input();
#else
     int c = yyinput();
#endif WIN32
     unput(c);
     if (c != '\''){
         yylval.strval = yytext;
         yylval.strval[strlen(yylval.strval) - 1] = '\0';
         yylval.strval += 1;
         return STRING;
     }
     else
         yymore();
}

<INITIAL>\n		{yylineno++;} /* whitespace */
<INITIAL>[ \t\r]+	        ;	    /* whitespace */
<INITIAL>.		{if (strlen(yytext) != 0)
			   return yyerror("lex: An invalid character in XPATH expression.");
		        }
%%
