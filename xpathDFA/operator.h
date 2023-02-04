// -*- mode: c++ -*-
//  This is a Predicate module for xmatch processor.
//  copyright (C) 2001 Makoto Onizuka, University of Washington
//  $Id: operator.h,v 1.2 2002/08/07 06:58:57 monizuka Exp $

#if ! defined(__operator_H__)
#define __operator_H__
				// These are basic operators
#define  MATH_OPR_NOT_DEFINED   0
#define  MATH_OPR_EQ   1		// "="
#define  MATH_OPR_NEQ  2		// "!="
#define  MATH_OPR_LES  3		// "<"
#define  MATH_OPR_GRE  4		// ">"
#define  MATH_OPR_LEQ  5		// "<="
#define  MATH_OPR_GEQ  6		// ">="

#define	 FUNC_CONTAINS	1		// "contains"
#define	 FUNC_STARTS	2		// "starts-with"
#endif
