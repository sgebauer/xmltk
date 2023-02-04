#define YY_ALWAYS_INTERACTIVE 1

extern int lineno;
extern char *yytext;
extern int my_yyinput(char *, int);
extern int yylex(void);
extern int yyerror(char * mes);
extern FILE * yyin;
extern int yyparse(void);
