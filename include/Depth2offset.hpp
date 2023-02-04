// -*- mode: c++ -*-
//  copyright (C) 2001 Makoto Onizuka, University of Washington

#if ! defined(__DEPTH2OFFSET_H__)
#define __DEPTH2OFFSET_H__

#include <stdio.h>
#include <iostream.h>
#include <FileManager.h>
#include "SIndex.hpp"		// to read indexHeader and indexTable

class Depth2offset {
protected:
  enum states {
	S_CONSTRUCT = 0,
	S_ACTIVE,		// SIX is created
	S_END_OF_INDEX
  };

  struct indexTable * iPath;
  unsigned int	maxDepth;	// static: max depth
  unsigned int	maxRowSize;	// static: max row size
  unsigned int	currentDepth;	// dynamic
  unsigned int	skipDepth; 	// dynamic: 
				// this is a flag that specifies current 
				// row is match with the input element.
  FileManager	* buf;
  states	state;

public:
  Depth2offset(const char * xmlFile):iPath(0),maxDepth(0),maxRowSize(0),currentDepth(0),skipDepth(0),state(S_CONSTRUCT){
	if (xmlFile == 0) {
	  buf = new FileManager("stdout.six");
	  return;
	}
	else {
	  if (!strcmp(xmlFile + strlen(xmlFile) - strlen(".xml"), ".xml")){
		char * filename = (char *)malloc(strlen(xmlFile)+1);
		strcpy(filename, xmlFile);
		strcpy(filename + strlen(xmlFile) - strlen(".six"), ".six");
		buf = new FileManager(filename);
		free(filename);
	  }
	  else {
		char * filename = (char *)malloc(strlen(xmlFile)+5);
		strcpy(filename, xmlFile);
		strcpy(filename + strlen(xmlFile), ".six");
		buf = new FileManager(filename);
		free(filename);
	  }
	}
  }
  virtual ~Depth2offset() {
	if (iPath != 0) delete iPath;
	delete buf;
  }

  Depth2offset * init(void){
	if (buf->open() == false) return 0;	// IndexManager dosen't work
	else state = S_ACTIVE;
	struct indexHeader ih;
	buf->read(&ih, sizeof(struct indexHeader));
	maxDepth = ih.depth;
	maxRowSize = ih.rowSize;
	iPath =(struct indexTable *)calloc(maxDepth,sizeof(struct indexTable));
	buf->read(&iPath[currentDepth], sizeof(struct indexTable));
	return this;
  }

  void startElement(unsigned int elementID) {
	if (state != S_ACTIVE) return;
#if defined(XMLTK_DEBUG)
	cout << "Depth2offset::startElement(" << elementID << ")";
#endif
	if (skipDepth > 0){
#if defined(XMLTK_DEBUG)
	  cout << " unmatch(skipDepth>0)" << endl;
#endif
	  skipDepth++;
	  return;
	}
	if (iPath[currentDepth].startOffset == elementID){
#if defined(XMLTK_DEBUG)
	  cout << " match(" << currentDepth << ")" << endl;
#endif
	  currentDepth++;
	  return;
	}
				// there is no entry
	if (iPath[currentDepth].startOffset > elementID){
#if defined(XMLTK_DEBUG)
	  cout << " unmatch(no entry)" << endl;
#endif
	  skipDepth++;
	  return;
	  // else continue to the following while loop
	}
	while (buf->read(&iPath[currentDepth], sizeof(struct indexTable))){
	  if (iPath[currentDepth].startOffset < elementID) continue;
				// there is an entry
	  else if (iPath[currentDepth].startOffset == elementID){
#if defined(XMLTK_DEBUG)
		cout << " match(" << currentDepth << ")" << endl;
#endif
		currentDepth++;
		return;
	  }
	  else {		// no entry for the element,
				// but already read a following entry.
#if defined(XMLTK_DEBUG)
		cout << " unmatch(no entry)" << endl;
#endif
		skipDepth++;
		return;		// there is no entry
	  }
	}
	cerr << " unmatch(the end of index)" << endl;
	state = S_END_OF_INDEX;
  }

  void endElement(unsigned int elementID) {
	if (state != S_ACTIVE) return;
#if defined(XMLTK_DEBUG)
	cout << "Depth2offset::endElement(" << elementID << ")";
#endif
	if (skipDepth > 0){
	  skipDepth--;
	  return;
	}
	if (currentDepth == 0){
	  cerr << "Stack is empty." << endl;
	  return;
	}
				// there should be an entry
	currentDepth--;
#if defined(XMLTK_DEBUG)
	if (iPath[currentDepth].startOffset + iPath[currentDepth].endOffsetinOrg != elementID){
	  cout << " unmatch(" << currentDepth << ")" << endl;
	}
	else cout << " match(" << currentDepth << ")" << endl;
#endif
				// iPath[currentDepth+1] can store a following
				// entry for an element coming future
	iPath[currentDepth]=iPath[currentDepth+1];
  }

				// we have to consider both two cases:
				// [fail skip] /bib/book <- bib/article
				// [success skip] /bib/book[1] 
  bool isAppliable(void) {	// to check there is a row for the element
	if (state != S_ACTIVE) return false;
	if (skipDepth > 0) return false;
	return true;
  }

  unsigned int failSkip(unsigned int depth){ // for failSkip
	if (state != S_ACTIVE) return 0;
	if (depth == 0) return 0;
	if (depth >currentDepth){
	  cerr << "The specified skip depth (" << depth << ") exceeds the number of stacked elements (" << currentDepth << ")." << endl;
	  return 0;
	}
	currentDepth-=depth;
			// iPath[currentDepth+depth] can store a following
			// entry for an element coming future
	iPath[currentDepth+1]=iPath[currentDepth+depth];
	return iPath[currentDepth].endOffsetinOrg;
  }

				// for both failSkip and succeedSkip
  unsigned int skip(unsigned int depth, unsigned int currentAddress){
	if (state != S_ACTIVE) return 0;
	if (depth == 0) return 0;
	if (depth >currentDepth){
	  cerr << "The specified skip depth (" << depth << ") exceeds the number of stacked elements (" << currentDepth << ")." << endl;
	  return 0;
	}
	currentDepth-=depth;
			// iPath[currentDepth+depth] can store a following
			// entry for an element coming future
	iPath[currentDepth+1]=iPath[currentDepth+depth];
	return iPath[currentDepth].startOffset + iPath[currentDepth].endOffsetinOrg - currentAddress;
  }

  unsigned int getRowCount(void) {return maxRowSize;}

  void print(void){
	buf->seek(0, FileManager::T_SEEK_SET);
	struct indexHeader ih;
	buf->read(&ih, sizeof(struct indexHeader));
	fprintf(stdout, "rowSize=%d, depth=%d\n", ih.rowSize, ih.depth);
	struct indexTable it;
	for (unsigned int i = 0; i<maxRowSize; i++){
	  buf->read(&it, sizeof(struct indexTable));
	  fprintf(stdout, "startElement=%d, endElement=%d\n",
			  it.startOffset, it.endOffsetinOrg);
	}
  }
};

#endif
