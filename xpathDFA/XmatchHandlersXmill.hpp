// -*- mode: c++ -*-
//  This is a DfaHandler module for DFA processor.
//  copyright (C) 2001 Makoto Onizuka, University of Washington
//  $Id: XmatchHandlersXmill.hpp,v 1.1.1.1 2002/05/04 12:54:03 tjgreen Exp $

#include    <saxclient.hpp>

class XmatchHandlers : public SAXClient
{
public:
    // -----------------------------------------------------------------------
    //  Constructors
    // -----------------------------------------------------------------------
  XmatchHandlers (void);
  ~XmatchHandlers(void);

    // -----------------------------------------------------------------------
    //  Implementations of the Xmill's SAXClient interface
    // -----------------------------------------------------------------------
  void startDocument() {}
  void endDocument() {}

  void HandleAttribName(char *str,int len,char iscont);
  // Handles a given attribute name
  // 'iscont' is one, if the string is not complete and more is to follow

  void HandleAttribValue(char *str,int len,char iscont);
  // Handles an attribute value
  // 'iscont' is one, if the string is not complete and more is to follow

  void HandleAttribWhiteSpaces(char *str,int len,char iscont);
  // Handles white spaces between attribute-value pairs
  // 'iscont' is one, if the string is not complete and more is to follow

  void HandleStartLabel(char *str,int len,char iscont);
  // Handles a start tag
  // 'iscont' is one, if the string is not complete and more is to follow

  void HandleEndLabel(char *str,int len,char iscont);
  // Handles an end tag
  // 'iscont' is one, if the string is not complete and more is to follow

  void HandleText(char *str,int len,char iscont,int leftwslen,int rightwslen);
  // Handles a piece of text.
  // 'leftwslen' and 'rightwslen' specify how many white spaces (' ', '\n', ...)
  // are on the left and right end of the text.
  // If 'iscont=1', then 'rightwslen' must be zero and for the text piece
  // coming afterwards, 'leftwslen' must be zero.
  // It is also possible that 'len=leftwslen=rightwslen'

  void HandleComment(char *str,int len,char iscont);
// Handles a piece of comment data

void HandlePI(char *str,int len,char iscont);
// Handles a piece of processing instruction data

void HandleDOCTYPE(char *str,int len,char iscont);
// Handles a piece of DOCTYPE data

void HandleCDATA(char *str,int len,char iscont);
// Handles a piece of CDATA data
};
