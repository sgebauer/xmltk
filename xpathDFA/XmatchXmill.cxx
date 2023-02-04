// -*- mode: c++ -*-
//  This is a DfaHandler module for DFA processor.
//  copyright (C) 2001 Makoto Onizuka, University of Washington
//  $Id: XmatchXmill.cxx,v 1.1.1.1 2002/05/04 12:54:03 tjgreen Exp $

#include <xmlparse.hpp>
#include <Depth2offset.hpp>
#include "XPath2DFAFactory.h"
#include "XmatchHandlersXmill.hpp"
Query 			* gQuery;
XMLParse		* parser;
Depth2offset	* d2o;

// ---------------------------------------------------------------------------
//  Local data
//
//  encodingName
//      The encoding we are to output in. If not set on the command line,
//      then it is defaulted to LATIN1.
//
//  xmlFile
//      The path to the file to parser. Set via command line.
//
//  valScheme
//      Indicates what validation scheme to use. It defaults to 'auto', but
//      can be set via the -v= command.
//
//	expandNamespaces
//		Indicates if the output should expand the namespaces Alias with
//		their URI's, defaults to false, can be set via the command line -e
// ---------------------------------------------------------------------------
static const char*              encodingName    = "LATIN1";
static char*                    xmlFile         = 0;
static char*                    query           = 0;
static bool						expandNamespaces= false ;


// ---------------------------------------------------------------------------
//  Local helper methods
// ---------------------------------------------------------------------------
static void usage()
{
  cout <<  "\nUsage: xpath [options] xmlfile xpathExpression\n"
    "This program process the specified XPath expression and output a subtree\n"
    "\n"
    "Options:\n"
    "    -u=xxx      Handle unrepresentable chars [fail | rep | ref*]\n"
    "    -v=xxx      Validation scheme [always | never | auto*]\n"
    "    -e          Expand Namespace Alias with URI's.\n"
    "    -x=XXX      Use a particular encoding for output (LATIN1*).\n"
    "    -h          Show this help\n\n"
    "  * = Default if not provided explicitly\n\n"
    "The parser has intrinsic support for the following encodings:\n"
    "    UTF-8, USASCII, ISO8859-1, UTF-16[BL]E, UCS-4[BL]E,\n"
    "    WINDOWS-1252, IBM1140, IBM037\n"
       <<  endl;
}



// ---------------------------------------------------------------------------
//  Program entry point
// ---------------------------------------------------------------------------
int main(int argC, char* argV[])
{
  // Watch for special case help request
  //  sleep(1);
  if ((argC == 2) && !strcmp(argV[1], "-h"))
    {
      usage();
      return 2;
    }

  int parmInd;
  for (parmInd = 1; parmInd < argC; parmInd++)
    {
      // Break out on first parm not starting with a dash
      if (argV[parmInd][0] != '-')
	break;

      if (!strncmp(argV[parmInd], "-v=", 3)
	  ||  !strncmp(argV[parmInd], "-V=", 3))
        {
	  const char* const parm = &argV[parmInd][3];
        }
      else if (!strcmp(argV[parmInd], "-e")
	       ||  !strcmp(argV[parmInd], "-E"))
        {
	  expandNamespaces = true;
        }
      else if (!strncmp(argV[parmInd], "-x=", 3)
	       ||  !strncmp(argV[parmInd], "-X=", 3))
        {
	  // Get out the encoding name
	  encodingName = &argV[parmInd][3];
        }
      else if (!strncmp(argV[parmInd], "-u=", 3)
	       ||  !strncmp(argV[parmInd], "-U=", 3))
        {
	  const char* const parm = &argV[parmInd][3];
        }
      else
        {
	  cerr << "Unknown option '" << argV[parmInd]
	       << "', ignoring it\n" << endl;
        }
    }

  //
  //  And now we have to have more than one parameter left and 
  //  it must be the XPath query expression.
  //  If there are two paremeters, then the first aprameter
  //  indicates that XML file (if not XML is coming form STDOUT).
  //

  if (parmInd + 1 == argC){
	xmlFile = 0;
	query = argV[parmInd];
  }
  else if (parmInd + 2 == argC){
	xmlFile = argV[parmInd];
	query = argV[parmInd+1];
  }
  else {
    usage();
    return 1;
  }
  try {
    gQuery = XPath2DFAFactory::createQuery();
	gQuery->registerQuery(query);
    gQuery->createAutomata();
	// for debugging
	//    ofstream * ofs = new ofstream("xpathQuery.xml", ios::out);
	//    gQuery->printQuery(ofs, argV[2]);
	//    ofs->close();
	//    delete ofs;
  }
  catch (_Error & err){
    cerr << Query::version << endl;
    err.perror();
    cerr << "failed" << endl;
    return -1;
  }
  //
  //  Create a SAX parser object. Then, according to what we were told on
  //  the command line, set it to validate or not.
  //
  parser = new XMLParse();
  //
  //  Create a Depth2offset object.
  //
  Depth2offset * idx = new Depth2offset(xmlFile);
  d2o = idx->init();
  
  //
  //  Create the handler object and install it as the document and error
  //  handler for the parser. Then parse the file and catch any exceptions
  //  that propogate out
  //
  XmatchHandlers * handler = new XmatchHandlers();
  parser->setContentHandler(handler);
  parser->setErrorHandler(handler);
  parser->parse(xmlFile);

  // And call the termination method
  delete parser;			// parser must be destruct before query destruct
  delete gQuery; gQuery = 0;
  delete idx;
  return 0;
}

