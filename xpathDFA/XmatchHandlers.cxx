// -*- mode: c++ -*-
//  This is a DfaHandler module for DFA processor.
//  copyright (C) 2001 Makoto Onizuka, University of Washington
//  $Id: XmatchHandlers.cxx,v 1.1.1.1 2002/05/04 12:54:02 tjgreen Exp $

#include <util/XMLUniDefs.hpp>
#include <sax2/Attributes.hpp>
#include <sax2/SAX2XMLReader.hpp>
#include <sax2/XMLReaderFactory.hpp>
#include "Xmatch.hpp"
#include "Depth2offset.hpp"
extern Query 			* gQuery;
extern SAX2XMLReader	* parser;
extern Depth2offset		* d2o;
unsigned int outputLevel; // for checking output state and skip state
VarPtrArray * vpl;		  // for startContext, endContext

unsigned int contextCount;
unsigned int elementCount;
unsigned int skipCount;
unsigned int skipMax;
unsigned int skipSum;

// ---------------------------------------------------------------------------
//  XmatchHandlers: Constructors and Destructor
// ---------------------------------------------------------------------------
XmatchHandlers::XmatchHandlers( const char* const encodingName, 
				const XMLFormatter::UnRepFlags unRepFlags,
				const bool expandNamespaces) :

  fFormatter(encodingName, this, XMLFormatter::NoEscapes, unRepFlags),
  fExpandNS ( expandNamespaces ){
  //
  //  Go ahead and output an XML Decl with our known encoding. This
  //  is not the best answer, but its the best we can do until we
  //  have SAX2 support.
  //
}

XmatchHandlers::~XmatchHandlers(){
}


// ---------------------------------------------------------------------------
//  XmatchHandlers: Overrides of the output formatter target interface
// ---------------------------------------------------------------------------
void XmatchHandlers::writeChars(const XMLByte* const toWrite){
}

void XmatchHandlers::writeChars(const XMLByte* const toWrite,
				const unsigned int count,
				XMLFormatter* const formatter){
  // For this one, just dump them to the standard output
  // Surprisingly, Solaris was the only platform on which
  // required the char* cast to print out the string correctly.
  // Without the cast, it was printing the pointer value in hex.
  // Quite annoying, considering every other platform printed
  // the string with the explicit cast to char* below.

  //  cout.write((char *) toWrite, (int) count);
  //  cout.flush();
}


// ---------------------------------------------------------------------------
//  XmatchHandlers: Overrides of the SAX ErrorHandler interface
// ---------------------------------------------------------------------------
void XmatchHandlers::error(const SAXParseException& e){
  cerr << "\nError at file " << StrX(e.getSystemId())
       << ", line " << e.getLineNumber()
       << ", char " << e.getColumnNumber()
       << "\n  Message: " << StrX(e.getMessage()) << endl;
}

void XmatchHandlers::fatalError(const SAXParseException& e){
  cerr << "\nFatal Error at file " << StrX(e.getSystemId())
       << ", line " << e.getLineNumber()
       << ", char " << e.getColumnNumber()
       << "\n  Message: " << StrX(e.getMessage()) << endl;
}

void XmatchHandlers::warning(const SAXParseException& e){
  cerr << "\nWarning at file " << StrX(e.getSystemId())
       << ", line " << e.getLineNumber()
       << ", char " << e.getColumnNumber()
       << "\n  Message: " << StrX(e.getMessage()) << endl;
}


// ---------------------------------------------------------------------------
//  XmatchHandlers: Overrides of the SAX DTDHandler interface
// ---------------------------------------------------------------------------
void XmatchHandlers::unparsedEntityDecl(const     XMLCh* const name
					, const   XMLCh* const publicId
					, const   XMLCh* const systemId
					, const   XMLCh* const notationName){
  // Not used at this time
}


void XmatchHandlers::notationDecl(const   XMLCh* const name
				  , const XMLCh* const publicId
				  , const XMLCh* const systemId){
  // Not used at this time
}


// ---------------------------------------------------------------------------
//  XmatchHandlers: Overrides of the SAX DocumentHandler interface
// ---------------------------------------------------------------------------
void XmatchHandlers::characters(const     XMLCh* const    chars,
				const   unsigned int    length){
#if defined(XMLTK_OUTPUT)
  if (outputLevel>0){
	PString * ps = new PString(chars);
	gQuery->characters(ps);
	cout << XMLString::transcode(chars);
	delete ps;
  }
#endif
}

void XmatchHandlers::endDocument(){
  gQuery->endDocument();
#if defined(XMLTK_OUTPUT)
  cout << "</root>" << endl;
  cout << "startElement count= " << elementCount << ",";
  cout << "startContext count= " << contextCount << ",";
  cout << "skip count= " << skipCount << ",";
  cout << "max skip= " << skipMax  << ",";
  cout << "avg skip= " << (float)skipSum/skipCount << endl;
#endif
}


void XmatchHandlers::endElement(const XMLCh* const uri, 
				const XMLCh* const localname, 
				const XMLCh* const qname){
#if defined(XMLTK_DEBUG)
  char * name;
  if ( fExpandNS ) name = XMLString::transcode(localname);
  else name = XMLString::transcode(qname);
  cout << "endElement(" << name << ") outputLevel=" << outputLevel << ", ";
#endif
#if defined(XMLTK_OUTPUT)
  if (outputLevel>0){
	char * name;
	if ( fExpandNS ) name = XMLString::transcode(localname);
	else name = XMLString::transcode(qname);
	cout << "</" << name << ">";
	if (outputLevel == 1) cout << endl;
  }
#endif

  if (vpl != 0 && vpl->getCount() != 0){
#if defined(XMLTK_ITERATION)
	for (unsigned int i = vpl->getCount()-1;; i--){
	  Variable *v = vpl->getItem(i);
#if defined(XMLTK_OUTPUT)
	  cout << "endContext(" << v->varName << ")" << endl;
#endif
	  if (i==0) break;
	}
#endif
	vpl = 0;
  }

  if (outputLevel > 0) outputLevel--;
  if (d2o) d2o->endElement(parser->getSrcOffset());// for streamingIndex
  PString * ps;
  if ( fExpandNS ){
	ps = new PString(localname);
	switch(gQuery->endElement(ps)){
	case Query::S_INACTIVE:
#if defined(XMLTK_DEBUG)
	  cout << "INACTIVE" << endl;
#endif
	  break;
	case Query::S_COMPLETE:
#if defined(XMLTK_DEBUG)
	  cout << "COMPLETE" << endl;
#endif
	  vpl = gQuery->getStateVariables();
	  break;
	case Query::S_SKIP:
#if defined(XMLTK_DEBUG)
	  cout << "SKIP" << endl;
#endif
	  if (outputLevel == 0 && d2o){
#if defined(XMLTK_DEBUG)
		cout << "suceed skip" << endl;
#endif
		if (d2o->isAppliable()==true){
		  unsigned int skipDepth = gQuery->succeedSkip();
		  unsigned int offset = d2o->succeedSkip(skipDepth, parser->getSrcOffset());
		  parser->skipReader(offset, skipDepth);
		  skipCount++;
		  if (skipMax<offset) skipMax = offset;
		  skipSum += offset;
#if defined(XMLTK_DEBUG)
		  cout << "  SUCEED SKIP(level=" << skipDepth << ", offset=" << offset << ") start:" << endl;
#endif
		}
	  }
	  break;
	}
  }
  else {
	ps = new PString(qname);
	switch(gQuery->endElement(ps)){
	case Query::S_INACTIVE:
#if defined(XMLTK_DEBUG)
	  cout << "INACTIVE" << endl;
#endif
	  break;
	case Query::S_COMPLETE:
#if defined(XMLTK_DEBUG)
	  cout << "COMPLETE" << endl;
#endif
	  vpl = gQuery->getStateVariables();
	  break;
	case Query::S_SKIP:
#if defined(XMLTK_DEBUG)
	  cout << "SKIP" << endl;
#endif
	  if (outputLevel == 0 && d2o){
#if defined(XMLTK_DEBUG)
		cout << "suceed skip" << endl;
#endif
		if (d2o->isAppliable()==true){
		  unsigned int skipDepth = gQuery->succeedSkip();
		  unsigned int offset = d2o->succeedSkip(skipDepth, parser->getSrcOffset());
		  parser->skipReader(offset, skipDepth);
		  skipCount++;
		  if (skipMax<offset) skipMax = offset;
		  skipSum += offset;
#if defined(XMLTK_DEBUG)
		  cout << "  SUCEED SKIP(level=" << skipDepth << ", offset=" << offset << ") start:" << endl;
#endif
		}
	  }
	  break;
	}
  }
  delete ps;
}


void XmatchHandlers::ignorableWhitespace( const   XMLCh* const chars
					  ,const  unsigned int length){
}


void XmatchHandlers::processingInstruction(const  XMLCh* const target
					   , const XMLCh* const data){
}


void XmatchHandlers::startDocument(){
  gQuery->startDocument();
  outputLevel = 0;
  vpl = 0;

  elementCount = 0;
  contextCount = 0;
  skipCount = 0;
  skipMax = 0;
  skipSum = 0;
#if defined(XMLTK_OUTPUT)
  cout << "<root>" << endl;
#endif
}

void XmatchHandlers::startElement(const   XMLCh* const  uri,
				  const   XMLCh* const  localname,
				  const   XMLCh* const  qname,
				  const   Attributes&	attributes){
  elementCount++;
  vpl = 0;						// to store the current completed Variables
  if (d2o) d2o->startElement(parser->getSrcOffset()); // for streamIndex
  PString * ps;
#if defined(XMLTK_DEBUG)
  char * name;
  if ( fExpandNS ) name = XMLString::transcode(localname);
  else name = XMLString::transcode(qname);
  cout << "startElement(" << name << ") outputLevel=" << outputLevel << ", ";
#endif
  if ( fExpandNS ){
	ps = new PString(localname);
	switch(gQuery->startElement(ps)){
	case Query::S_INACTIVE:
	  if (outputLevel > 0) outputLevel++;
#if defined(XMLTK_DEBUG)
	  cout << "INACTIVE" << endl;
#endif
	  break;
	case Query::S_COMPLETE:{
	  outputLevel++;
#if defined(XMLTK_DEBUG)
	  cout << "COMPLETE" << endl;
#endif
	  vpl = gQuery->getStateVariables();
#if defined(XMLTK_ITERATION)
	  unsigned int c = vpl->getCount();
	  for (unsigned int i = 0; i<c; i++){
		Variable *v = vpl->getItem(i);
		contextCount++;
#if defined(XMLTK_OUTPUT)
		cout << "startContext(" << v->varName << ")" << endl;
#endif
	  }
#endif
	  break;
	}
	case Query::S_SKIP:
  //
  // skip XML stream (both parser, Depth2offset, automata)
  // 1. just skip one level if there is no schema information.
  //    This is just for index skip (author[2] for example).
  // 2. skip some level using schema information.
  //
#if defined(XMLTK_DEBUG)
	  cout << "SKIP" << endl;
#endif
	  if (outputLevel == 0 && d2o){
		if (d2o->isAppliable()==true){
		  unsigned int skipDepth = gQuery->failSkip();
		  unsigned int offset = d2o->failSkip(skipDepth);
		  parser->skipReader(offset, skipDepth);
		  skipCount++;
		  if (skipMax<offset) skipMax = offset;
		  skipSum += offset;
#if defined(XMLTK_DEBUG)
		  cout << "  FAIL SKIP(level=" << skipDepth << ", offset=" << offset << ") start:" << XMLString::transcode(localname) << endl;
#endif
		}
	  }
	  else if (outputLevel > 0) outputLevel++;
	  break;
	}
  }
  else {
	ps = new PString(qname);
	switch(gQuery->startElement(ps)){
	case Query::S_INACTIVE:
	  if (outputLevel > 0) outputLevel++;
#if defined(XMLTK_DEBUG)
	  cout << "INACTIVE" << endl;
#endif
	  break;
	case Query::S_COMPLETE:{
	  outputLevel++;
#if defined(XMLTK_DEBUG)
	  cout << "COMPLETE" << endl;
#endif
	  vpl = gQuery->getStateVariables();
#if defined(XMLTK_ITERATION)
	  unsigned int c = vpl->getCount();
	  for (unsigned int i = 0; i<c; i++){
		Variable *v = vpl->getItem(i);
		contextCount++;
#if defined(XMLTK_OUTPUT)
		cout << "startContext(" << v->varName << ")" << endl;
#endif
	  }
#endif
	  break;
	}
	case Query::S_SKIP:			// skip both parser, Depth2offset, automata
#if defined(XMLTK_DEBUG)
	  cout << "SKIP" << endl;
#endif
	  if (outputLevel == 0 && d2o){
		if (d2o->isAppliable()==true){
		  unsigned int skipDepth = gQuery->failSkip();
		  unsigned int offset = d2o->failSkip(skipDepth);
		  parser->skipReader(offset, skipDepth);
		  skipCount++;
		  if (skipMax<offset) skipMax = offset;
		  skipSum += offset;
#if defined(XMLTK_DEBUG)
		  cout << "  FAIL SKIP(level=" << skipDepth << ", offset=" << offset << ") start:" << XMLString::transcode(qname) << endl;
#endif
		}
	  }
	  else if (outputLevel > 0) outputLevel++;
	  break;
	}
  }
#if defined(XMLTK_OUTPUT)
  if (outputLevel>0) cout << "<" << ps->getString() << ">";
#endif  
  delete ps;

  unsigned int len = attributes.getLength();
  for (unsigned int index = 0; index < len; index++){
    //
    //  Again the name has to be completely representable. But the
    //  attribute can have refs and requires the attribute style
    //  escaping.
    //
    //    if ( fExpandNS )
    //      {
    //	if (XMLString::compareIString(attributes.getURI(index),XMLUni::fgEmptyString) != 0)
    //	  fFormatter  << attributes.getURI(index) << chColon;
    //	fFormatter  << attributes.getLocalName(index) ;
    //      }
    //    else
    //      fFormatter  << attributes.getQName(index) ;
    
    //    fFormatter  << chEqual << chDoubleQuote
    //		<< XMLFormatter::AttrEscapes
    //		<< attributes.getValue(index)
    //		<< XMLFormatter::NoEscapes
    //		<< chDoubleQuote;
  }
}
