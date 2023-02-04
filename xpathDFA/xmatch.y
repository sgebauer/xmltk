%{
//	This is a yacc program for xmatch processor.
//      copyright (C) 2001 Makoto Onizuka, University of Washington
//      $Id: xmatch.y,v 1.3 2002/08/13 12:59:46 monizuka Exp $
// Restriction: 
//  1. It is not implemented to use attribute type name as an attribute name.
//     (CDATA, ID, IDREF, and so forth)
//  2. It is not implemented to use entity declaration, notation declaration,
//     and PI (processing instruction).        
//  3. Comment declaration is implemented using lex.

#include <stdio.h>
#include "Root.h"
#include "Query.h"
#ifdef WIN32
#include "xmatch.h"
#endif
extern int yylineno;		// for lex error.
#ifndef WIN32
extern char yytext[];		// for lex error.
#endif
#ifndef WIN32
#if defined(__cplusplus)
extern "C" {
#endif    
#endif
extern int yylex(void);

#ifdef WIN32
#include <malloc.h>
#define alloca _alloca
#endif

int yyerror(char * mes){
  fprintf(stderr, "yacc(lazyDFA) error (line %d): %s at \"%s\" in \"%s\"\n",
          yylineno, mes, yytext, __gQuery->query);
  return 0;
}

int my_yyinput(char * buf, int max_size){
    int n;

    n = min(max_size, strlen(__gQuery->top));

    if (n > 0){
        memcpy(buf, __gQuery->top, n);
	__gQuery->top += n;
    }
    return n;
}

#ifndef WIN32
#if defined(__cplusplus)
};
#endif
#endif

static Variable * var;		// Variable Definition
static XPath * xpath, * pxpath;	// XPath expression
				// LocationStep
static LocationStep  * lstep, * plstep;
static Predicate     * pred;
static AtomPredicate * ap;
static Expression    * exp;	// Predicate's right or left side expression
static Root	     * gRoot;
%}

%union {
  char* strval;			/* for many purpose to store string */
  float fltval;			/* for float literal value */
  int   intval;			/* for integer literal value */
  int   subtok;			/* for basic operator type */
}

%token <strval> ENAME		/* output element */
%token <strval> ANAME		/* output attribute */
%token <strval> EREF		/* element reference */
%token <strval> AREF		/* attribute reference */
%token <strval> FCALL		/* function call */
%token <strval> STRING		/* string literal value */
%token <intval> INTEGER		/* integer literal value */
%token <fltval> FLOAT		/* float literal value */

%left OR
%left AND
%left <subtok> OPERATOR
%left <subtok> FUNCTION
%left DIV
%left MOD
%nonassoc UMINUS

%token MAP VARDEF IN ARROW SLASH DSLASH	TEXT

%%

varExpr:
	    { gRoot = __gQuery->getRoot();
	      var = new Variable();
	      gRoot->insertVariable(var);
	      xpath = new XPath();
	    } locationPath {
	      var->setXPath(xpath);
	    }
      |    error { return -1;}
      ;

locationPath:			/* path, /, /path, or //path */
            relativeLocationPath {
	      plstep = xpath->getLastLStep();
				/* in case expression like "/bib/book/" */
	      if (plstep!=0 && plstep->getType()==LocationStep::DoubleSlash){
		cerr << "A tag must be specified just after //." << endl;
		throw _Error(gRoot, "yacc::locationPath", __LINE__);
	      }
	    }
      |     SLASH		/* DocumentRoot: do nothing */
      |     pathConnect relativeLocationPath {
	      plstep = xpath->getLastLStep();
				/* in case expression like "/bib/book/" */
	      if (plstep!=0 && plstep->getType()==LocationStep::DoubleSlash){
		cerr << "A tag must be specified just after //." << endl;
		throw _Error(gRoot, "yacc::locationPath", __LINE__);
	      }
            }
      ;
	    
relativeLocationPath:	/* constant XPath expression (without variable) */
           lStepExpr 
      |    relativeLocationPath pathConnect lStepExpr
      ;

relativeLocationPathNoAlt:	/* relativeLocationPath without alternation */
           lStepExprNoAlt	/* to remove anbiguity (a|b) and (a and b) */
      |    relativeLocationPathNoAlt pathConnect lStepExprNoAlt
      ;

pathConnect:
	   SLASH		/* do nothing */
      |    DSLASH {
	     try {
	       lstep = new LocationStep(xpath, LocationStep::DoubleSlash, 0);
	     }
	     catch (_Error & err){
	       err.addItem(new ErrItem(gRoot, "yacc::lStepExpr",__LINE__));
	       throw err;
	     }
	   }
      ;

lStepExpr:			/* FUTURE: needs to support function call */
	   '.' {		/* period*/
	     try {
	       lstep = new LocationStep(xpath, LocationStep::Period, 0);
	     }
	     catch (_Error & err){
	       err.addItem(new ErrItem(gRoot, "yacc::lStepExpr",__LINE__));
	       throw err;
	     }
	   }
           optPredicateListExpr
      |	   '.' '.' {		/* period period */
	     cerr << "\"..\" is not supported." << endl;
	     throw _Error(gRoot, "yacc::xPathExpr", __LINE__);
	     try {
	       lstep = new LocationStep(xpath, LocationStep::DoublePeriod, 0);
	     }
	     catch (_Error & err){
	       err.addItem(new ErrItem(gRoot, "yacc::lStepExpr",__LINE__));
	       throw err;
	     }
	   }
           optPredicateListExpr
      |	   '*' {		/* element simple path */
	     try {
	       lstep = new LocationStep(xpath, LocationStep::AnyElement, 0);
	     }
	     catch (_Error & err){
	       err.addItem(new ErrItem(gRoot, "yacc::lStepExpr",__LINE__));
	       throw err;
	     }
	   }
           optPredicateListExpr
      |	   '@' '*' {		/* element simple path */
	     try {
	       lstep = new LocationStep(xpath, LocationStep::AnyAttribute, 0);
	     }
	     catch (_Error & err){
	       err.addItem(new ErrItem(gRoot, "yacc::lStepExpr",__LINE__));
	       throw err;
	     }
	   }
           optPredicateListExpr
      |	   EREF {		/* element simple path */
	     try {
	       lstep = new LocationStep(xpath, LocationStep::Element, $1);
	     }
	     catch (_Error & err){
	       err.addItem(new ErrItem(gRoot, "yacc::lStepExpr",__LINE__));
	       throw err;
	     }
	   }
           optPredicateListExpr
      |    AREF {		/* attribute simple path */
	     try {
	       lstep = new LocationStep(xpath, LocationStep::Attribute, $1);
	     }
	     catch (_Error & err){
	       err.addItem(new ErrItem(gRoot, "yacc::lStepExpr",__LINE__));
	       throw err;
	     }
	   }
           optPredicateListExpr
      |    TEXT ')' {		/* text() */
	     try {
	       lstep = new LocationStep(xpath, LocationStep::Text, 0);
	     }
	     catch (_Error & err){
	       err.addItem(new ErrItem(gRoot, "yacc::lStepExpr",__LINE__));
	       throw err;
	     }
	   }
           optPredicateListExpr
      |    '(' {		/* nested alternation (choice) */
	     try {
	       lstep = new LocationStep(xpath, LocationStep::AltElement, 0);
	       pxpath = xpath;
	       xpath = new XPath();          /* for alternative path */
	       lstep->insertAltPath(xpath);
	     }
	     catch (_Error & err){
	       err.addItem(new ErrItem(gRoot, "yacc::lStepExpr",__LINE__));
	       throw err;
	     }
	   }
           altPathListExpr ')' {
	     try {
	       xpath  = pxpath;
	       pxpath = xpath->getParent();
	     }
	     catch (_Error & err){
	       err.addItem(new ErrItem(gRoot, "yacc::lStepExpr",__LINE__));
	       throw err;
	     }
	   }
      ;

lStepExprNoAlt:			/* FUTURE: needs to support function call */
	   '.' {		/* period*/
	     try {
	       lstep = new LocationStep(xpath, LocationStep::Period, 0);
	     }
	     catch (_Error & err){
	       err.addItem(new ErrItem(gRoot, "yacc::lStepExprNoAlt",__LINE__));
	       throw err;
	     }
	   }
           optPredicateListExpr
      |	   '.' '.' {		/* period period */
	     cerr << "\"..\" is not supported." << endl;
	     throw _Error(gRoot, "yacc::lStepExprNoAlt", __LINE__);
	     try {
	       lstep = new LocationStep(xpath, LocationStep::DoublePeriod, 0);
	     }
	     catch (_Error & err){
	       err.addItem(new ErrItem(gRoot, "yacc::lStepExprNoAlt",__LINE__));
	       throw err;
	     }
	   }
           optPredicateListExpr
      |	   '*' {		/* element simple path */
	     try {
	       lstep = new LocationStep(xpath, LocationStep::AnyElement, 0);
	     }
	     catch (_Error & err){
	       err.addItem(new ErrItem(gRoot, "yacc::lStepExprNoAlt",__LINE__));
	       throw err;
	     }
	   }
           optPredicateListExpr
      |	   '@' '*' {		/* element simple path */
	     try {
	       lstep = new LocationStep(xpath, LocationStep::AnyAttribute, 0);
	     }
	     catch (_Error & err){
	       err.addItem(new ErrItem(gRoot, "yacc::lStepExprNoAlt",__LINE__));
	       throw err;
	     }
	   }
           optPredicateListExpr
      |	   EREF {		/* element simple path */
	     try {
	       lstep = new LocationStep(xpath, LocationStep::Element, $1);
	     }
	     catch (_Error & err){
	       err.addItem(new ErrItem(gRoot, "yacc::lStepExprNoAlt",__LINE__));
	       throw err;
	     }
	   }
           optPredicateListExpr
      |    AREF {		/* attribute simple path */
	     try {
	       lstep = new LocationStep(xpath, LocationStep::Attribute, $1);
	     }
	     catch (_Error & err){
	       err.addItem(new ErrItem(gRoot, "yacc::lStepExprNoAlt",__LINE__));
	       throw err;
	     }
	   }
           optPredicateListExpr
      |    TEXT ')' {		/* text() */
	     try {
	       lstep = new LocationStep(xpath, LocationStep::Text, 0);
	     }
	     catch (_Error & err){
	       err.addItem(new ErrItem(gRoot, "yacc::lStepExprNoAlt",__LINE__));
	       throw err;
	     }
	   }
           optPredicateListExpr
      ;

altPathListExpr:		/* constant XPath expression for alternation */
	   relativeLocationPath
      |    altPathListExpr '|' {/* pxpath dose not needed to change here */
	     try {
	       xpath = new XPath();
	       plstep = pxpath->getLastLStep();
	       plstep->insertAltPath(xpath);
	     }
	     catch (_Error & err){
	       err.addItem(new ErrItem(gRoot, "yacc::altPathListExpr",__LINE__));
	       throw err;
	     }
	   } relativeLocationPath
      ;

optPredicateListExpr:

      |    predicateListExpr {
	     lstep->setType();
	   }
      ;

predicateListExpr:
	       '[' {
	         pxpath = xpath;
		 xpath = new XPath();
		 xpath->setParent(pxpath);
		 pred = new Predicate(var,0);
		 lstep->addPredicate(pred);
	       }
               predicateExpr ']' {
		 xpath  = pxpath;
		 pxpath = xpath->getParent();
		 lstep  = xpath->getLastLStep();
	       }
	  |    predicateListExpr '[' {
		 pxpath = xpath;
		 xpath = new XPath();
		 xpath->setParent(pxpath);
		 pred = new Predicate(var,0);
		 lstep->addPredicate(pred);
	       }
               predicateExpr ']' {
	         xpath  = pxpath;
		 pxpath = xpath->getParent();
		 lstep  = xpath->getLastLStep();
	       }
      ;

predicateExpr:
	  {
	    pred->atom = new AtomPredicate();
	  } atomPredExpr {
	    pred->setType();	/* after the ap->oprType set */
	  }
      |   predicateExpr AND predicateExpr {
	     cerr << "AND operator isn't supported yet. Please modify it to [cond1][cond2]." << endl;
	     throw _Error(gRoot, "yacc::predicateExpr", __LINE__);
          }
      |   predicateExpr OR predicateExpr {
	     cerr << "OR operator isn't supported yet." << endl;
	     throw _Error(gRoot, "yacc::predicateExpr", __LINE__);
          }
      |	  '(' predicateExpr ')'
      ;

atomPredExpr:
	  relativeLocationPathNoAlt {	/* unary: existence check */
	    plstep = pxpath->getLastLStep();
	    pred = plstep->getLastPred();
	    ap = pred->getTopAtom();
	    ap->oprType = AtomPredicate::T_UNARY;
	    ap->left = new Expression(xpath);
	  }
      |	  INTEGER {			/* index number */
	    if ($1 == 0){
	      cerr << "index number\""<< $1 << "\" needs larger than 1." << endl;
	      throw _Error(gRoot, "yacc::atomPredExpr", __LINE__);
	    }
	    plstep = pxpath->getLastLStep();
	    pred = plstep->getLastPred();
	    ap = pred->getTopAtom();
	    ap->oprType = AtomPredicate::T_INDEX;
	    ap->left = new Expression($1);
          }
      |	  FUNCTION expr { 		/* function */
	    plstep = pxpath->getLastLStep();
	    pred = plstep->getLastPred();
	    ap = pred->getTopAtom();
	    ap->opr = $1;
	    ap->oprType = AtomPredicate::T_FUNCTION;
	    ap->left = exp;
	    if (!exp->getXPath()){
	      cerr << "Conditions needs to be [function(xpath,literal)] form."<< endl;
	      throw _Error(gRoot, "yacc::atomPredExpr", __LINE__);
	    }
          } ',' expr {
	    plstep = pxpath->getLastLStep();
	    pred = plstep->getLastPred();
	    ap = pred->getTopAtom();
	    ap->right = exp;
	    if (exp->getXPath()){
	      cerr << "Conditions needs to be [function(xpath,literal)] form."<< endl;
	      throw _Error(gRoot, "yacc::atomPredExpr", __LINE__);
	    }
          } ')' 
      |	  expr {			/* binary: condition */
	    plstep = pxpath->getLastLStep();
	    pred = plstep->getLastPred();
	    ap = pred->getTopAtom();
	    ap->oprType = AtomPredicate::T_BINARY;
	    ap->left = exp;
	    if (!exp->getXPath()){
	      cerr << "Conditions needs to be [xpath operator literal] form." << endl;
	      throw _Error(gRoot, "yacc::atomPredExpr", __LINE__);
	    }
	  }
	  OPERATOR expr {
	    plstep = pxpath->getLastLStep();
	    pred = plstep->getLastPred();
	    ap = pred->getTopAtom();
	    ap->opr = $3;
	    ap->right = exp;
	    if (exp->getXPath()){
	      cerr << "Conditions needs to be [xpath operator literal] form." << endl;
	      throw _Error(gRoot, "yacc::atomPredExpr", __LINE__);
	    }
	  } 
      ;

expr:
	  relativeLocationPathNoAlt { /* path expression */
	    exp = new Expression(xpath);
	  }
      |	  literalExpr		/* literal value expresion */
      ;

literalExpr:
           STRING {
			 exp = new Expression(strdup($1));
		   }
      |    INTEGER {
			 exp = new Expression($1);
	       }
      |    FLOAT {
			 exp = new Expression($1);
	       }
      ;

%%
