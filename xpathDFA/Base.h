/************************************************************************
// -*- mode: c++ -*-
//  copyright (C) 2001-2002 Makoto Onizuka, University of Washington
//  $Id: Base.h,v 1.1.1.1 2002/05/04 12:53:53 tjgreen Exp $
//
*************************************************************************/
#if ! defined(__XMLTK_BASE_H__)
#define __XMLTK_BASE_H__

static unsigned long _Base_gid = 0;

class Base {			// MetaClass
public:
  unsigned long id;		// ID
  
public:
  Base(): id(_Base_gid++)	{};
  virtual ~Base(){};
};

#endif /* __XMLTK_BASE_H__ */
