// -*- mode: c++ -*-
//  copyright (C) 2001 Makoto Onizuka, University of Washington
//  $Id: Predicate.cxx,v 1.3 2002/08/13 12:59:46 monizuka Exp $

#define PREDICATE_EMBODY
#include "Variable.h"
#include "Automata.h"

//
// issue: case 1: "//book[1]/title"
//        case 2: "//star[1]/title"
// <bib>
//   <book><title/></book>
//   <book><title/></book>
// </bib>
// Case 2, the predicate is inactivated at startElement(title),
// because, the "//star" binds the title.
//

bool
Predicate::apply(Automata * a, PString * ps){
  switch(type){
  case T_DOUBLESLASH_INDEX:
    if (accessCount == 0) automataStackCount = a->getStackDepth();
				// continue to T_INDEX
  case T_INDEX:
    if (atom->left->value.integer == ++accessCount){
      ctype = T_COMPLETE;
      return var->setEnableFlag(this, true);
    }
    return var->setEnableFlag(this, false);
  case T_SIMPLE_PREDICATE:
    if (!var->getEnableFlag()) return false; // if var is already set as "false"
    switch(atom->oprType){
    case AtomPredicate::T_UNARY:
      return true;
    case AtomPredicate::T_BINARY:{
      switch(atom->opr){
      case MATH_OPR_EQ:
	switch(atom->right->valType){
	case Expression::T_VAL_STRING:
	  if (!strcmp(ps->getString(), atom->right->value.string)){
	    return true;
	  }
	  break;
	case Expression::T_VAL_INTEGER:
	  if (atoi(ps->getString()) == atom->right->value.integer){
	    return true;
	  }
	  break;
	case Expression::T_VAL_FLOAT:
	  if (atof(ps->getString()) == atom->right->value.real){
	    return true;
	  }
	  break;
	} // end switch
	break;

      case MATH_OPR_NEQ:
	switch(atom->right->valType){
	case Expression::T_VAL_STRING:
	  if (strcmp(ps->getString(), atom->right->value.string)){
	    return true;
	  }
	  break;
	case Expression::T_VAL_INTEGER:
	  if (atoi(ps->getString()) != atom->right->value.integer){
	    return true;
	  }
	  break;
	case Expression::T_VAL_FLOAT:
	  if (atof(ps->getString()) != atom->right->value.real){
	    return true;
	  }
	  break;
	} // end switch
	break;

      case MATH_OPR_LES:
	switch(atom->right->valType){
	case Expression::T_VAL_STRING:
	  if (strcmp(ps->getString(), atom->right->value.string)<0){
	    return true;
	  }
	  break;
	case Expression::T_VAL_INTEGER:
	  if (atoi(ps->getString()) < atom->right->value.integer){
	    return true;
	  }
	  break;
	case Expression::T_VAL_FLOAT:
	  if (atof(ps->getString()) < atom->right->value.real){
	    return true;
	  }
	  break;
	} // end switch
	break;

      case MATH_OPR_GRE:
	switch(atom->right->valType){
	case Expression::T_VAL_STRING:
	  if (strcmp(ps->getString(), atom->right->value.string)>0){
	    return true;
	  }
	  break;
	case Expression::T_VAL_INTEGER:
	  if (atoi(ps->getString()) > atom->right->value.integer){
	    return true;
	  }
	  break;
	case Expression::T_VAL_FLOAT:
	  if (atof(ps->getString()) > atom->right->value.real){
	    return true;
	  }
	  break;
	} // end switch
	break;

      case MATH_OPR_LEQ:
	switch(atom->right->valType){
	case Expression::T_VAL_STRING:
	  if (strcmp(ps->getString(), atom->right->value.string)<=0){
	    return true;
	  }
	  break;
	case Expression::T_VAL_INTEGER:
	  if (atoi(ps->getString()) <= atom->right->value.integer){
	    return true;
	  }
	  break;
	case Expression::T_VAL_FLOAT:
	  if (atof(ps->getString()) <= atom->right->value.real){
	    return true;
	  }
	  break;
	} // end switch
	break;

      case MATH_OPR_GEQ:
	switch(atom->right->valType){
	case Expression::T_VAL_STRING:
	  if (strcmp(ps->getString(), atom->right->value.string)>=0){
	    return true;
	  }
	  break;
	case Expression::T_VAL_INTEGER:
	  if (atoi(ps->getString()) >= atom->right->value.integer){
	    return true;
	  }
	  break;
	case Expression::T_VAL_FLOAT:
	  if (atof(ps->getString()) >= atom->right->value.real){
	    return true;
	  }
	  break;
	} // end switch(atom->right->valType)
	break;
      }	// end switch(atom->opr){
      break;
    } // end case AtomPredicate::T_BINARY:
    case AtomPredicate::T_FUNCTION:{
      switch(atom->opr){
      case FUNC_CONTAINS:
	switch(atom->right->valType){
	case Expression::T_VAL_STRING:
	  if (strstr(ps->getString(), atom->right->value.string)){
	    return true;
	  }
	  break;
	default:
	  cerr << errmes[E_TYPE_MISMATCH] << endl;
	  throw _Error(this, "Predicate::apply()", __LINE__);
	} // end switch(atom->right->valType)
	break;

      case FUNC_STARTS:
	switch(atom->right->valType){
	case Expression::T_VAL_STRING:
	  if (!strncmp(ps->getString(), atom->right->value.string, strlen(atom->right->value.string))){
	    return true;
	  }
	  break;
	default:
	  cerr << errmes[E_TYPE_MISMATCH] << endl;
	  throw _Error(this, "Predicate::apply()", __LINE__);
	} // end switch(atom->right->valType)
	break;
      }	// end switch(atom->opr)
      break;
    }// end case AtomPredicate::T_FUNCTION
    default:			// T_INDEX
      cerr << errmes[E_INTERNAL] << endl;
      throw _Error(this, "Predicate::apply()", __LINE__);
    } // end switch(atom->predType)
    return var->setEnableFlag(this, false);
      // end case T_SIMPLE_PREDICATE
  default:			// T_PREDICATE
    cerr << errmes[E_NOT_SUPPORT] << endl;
    throw _Error(this, "Predicate::apply()", __LINE__);
  } // end switch(type)
}

void 
Predicate::reset(Automata * a){
  if (type == T_SIMPLE_PREDICATE ){
    automataStackCount = 0;
    var->setEnableFlag(this, true);
  }
}

void 
Predicate::resetAccessCount(Automata * a){
  if (type == T_INDEX ||
      type == T_DOUBLESLASH_INDEX && 
      automataStackCount == a->getStackDepth()
      ){
    automataStackCount = 0;
    accessCount = 0;
    ctype = T_CONTINUE;
    var->setEnableFlag(this, true);
  }
}

// This is invoked from yacc.
// T_DOUBLESLASH_INDEX is set from LocationStep::setPredTypeDoubleSlash()
// This fucntion also sets the a->predType
void  
Predicate::setType(void){
		// if this is an atomic predicate
  if (atom && atom->oprType == AtomPredicate::T_INDEX){
    type = T_INDEX;
    return;
  }
  type = T_SIMPLE_PREDICATE;
  XPath * xp = atom->left->getXPath();
  unsigned int c = xp->getLStepCount();
  for (unsigned int i = 0; i<c;i++){
    LocationStep * ls = xp->getLStep(i);
    switch (ls->getType()){
    case LocationStep::Element:
    case LocationStep::AnyElement:
      type = T_PREDICATE;
      if (i+1==c){		// append "text()" to the tail
	if (atom->oprType==AtomPredicate::T_BINARY||
	    atom->oprType==AtomPredicate::T_FUNCTION)
	  new LocationStep(xp, LocationStep::Text, 0);
      }
      break;
    case LocationStep::Period:
      if (i+1==c){		// The tail is period, we need to concatinate
				// all descendant text(). not support
	cerr << errmes[E_NOT_SUPPORT] << endl;
	throw _Error(this, "Predicate::setType()", __LINE__);
      }
      break;
    case LocationStep::DoubleSlash:
    case LocationStep::DocumentRoot:
      break;			// skip
    case LocationStep::Attribute:
    case LocationStep::AnyAttribute:
    case LocationStep::Text:
      break;			// skip
    default:			// DoublePeriod, AltElement
      cerr << errmes[E_NOT_SUPPORT] << endl;
      throw _Error(this, "Predicate::setType()", __LINE__);
    }
  }
  // every locationstep is SIMPLE_PREDICATE,
  // attribute, or text()
  return;
}

