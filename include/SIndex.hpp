// -*- mode: c++ -*-
//  copyright (C) 2001 Makoto Onizuka, University of Washington

#if ! defined(__INDEXCREATE_H__)
#define __INDEXCREATE_H__

#include <stdio.h>

struct indexHeader {
  unsigned int rowSize;
  unsigned int depth;
};
struct indexTable {
  unsigned int startOffset;
  unsigned int endOffsetinOrg;	// relative
  //  unsigned int endOffsetinSix;
};

#define ITABLE_INITIAL_SIZE 1000
#define ITABLE_EXPAND_SIZE  1000
#define IPATH_INITIAL_SIZE 50
#define IPATH_EXPAND_SIZE  50

class SIndex {
protected:
  struct indexTable * iTable;
  unsigned int		  rowBufSize;	// current row buffer size
  unsigned int		  currentRowSize;
  unsigned int		  expandRowSize;
  struct indexTable ** iPath;
  unsigned int		  depthBufSize;	// current depth buffer size
  unsigned int		  maxDepth;		// max depth
  unsigned int		  currentDepth;
  unsigned int		  expandDepth;
  unsigned int		  threshHold;
  char *			  fileName;

public:
  SIndex(char * xmlFile, unsigned int th):
	iTable(0),rowBufSize(ITABLE_INITIAL_SIZE),currentRowSize(0),expandRowSize(ITABLE_EXPAND_SIZE),
	iPath(0),depthBufSize(IPATH_INITIAL_SIZE),maxDepth(0),currentDepth(0),expandDepth(IPATH_EXPAND_SIZE),threshHold(th) {
	char * filename;
	if (xmlFile == 0){
	  fileName = filename = (char *)malloc(strlen("stdout.six")+1);
	  strcpy(filename, "stdout.six");
	}
	else {
	  if (!strcmp(xmlFile + strlen(xmlFile) - strlen(".xml"), ".xml")){
		fileName = filename = (char *)malloc(strlen(xmlFile)+1);
		strcpy(filename, xmlFile);
		strcpy(filename + strlen(xmlFile) - strlen(".six"), ".six");
	  }
	  else {
		fileName = filename = (char *)malloc(strlen(xmlFile)+5);
		strcpy(filename, xmlFile);
		strcpy(filename + strlen(xmlFile), ".six");
	  }
	}
  }
  virtual ~SIndex() {
	if (iTable != 0) free(iTable);
	if (iPath != 0) free(iPath);
	if (fileName != 0) free(fileName);
  }
  void setiTable(unsigned int init, unsigned int expand){
	rowBufSize = init + 1;
	expandRowSize = expand;
  }
  void setiPath(unsigned int init, unsigned int expand){
	depthBufSize = init + 1;
	expandDepth = expand;
  }
  void init(void){
	iTable =(struct indexTable *)calloc(rowBufSize,sizeof(struct indexTable));
	iPath =(struct indexTable **)malloc(depthBufSize*sizeof(struct indexTable*));
  }

  void writeFile(void){
	FILE * indexfp = fopen(fileName, "w");
	struct indexHeader ih;
	ih.rowSize = currentRowSize;
	ih.depth = maxDepth;
	fwrite(&ih, sizeof(struct indexHeader), 1, indexfp);
	fwrite(iTable, sizeof(struct indexTable), currentRowSize + 1, indexfp);
	cout << "element count=" << currentRowSize << endl;
	fclose(indexfp);
  }

  void startElement(unsigned int offset){
	if (rowBufSize==currentRowSize) expandiTable();
	iTable[currentRowSize++].startOffset = offset;

	if (depthBufSize==currentDepth) expandiPath();
	iPath[currentDepth++] = &(iTable[currentRowSize-1]);
	if (currentDepth>maxDepth) maxDepth = currentDepth;
  }

  void endElement(unsigned int offset){
	unsigned int relativeOffset = offset - iPath[currentDepth-1]->startOffset;
	currentDepth--;
	if (relativeOffset >= threshHold){
	  iPath[currentDepth]->endOffsetinOrg = relativeOffset;
	}
	else {
	  //	  cout << "delete row (elementID="<< iPath[currentDepth]->startOffset << ")" << endl;
	  currentRowSize--;
	}
  }
private:
  void expandiTable(void){
	struct indexTable * it;
	it =(struct indexTable *)calloc(rowBufSize+expandRowSize,
									sizeof(struct indexTable));
	if (it == 0) cerr << "Memory can not allocat any more." << endl;
	memcpy(it, iTable, rowBufSize * sizeof(struct indexTable));
	free(iTable);
	iTable = it;
	rowBufSize+=expandRowSize;
  }
  void expandiPath(void){
	struct indexTable ** it;
	it =(struct indexTable **)malloc(depthBufSize+expandDepth *
									sizeof(struct indexTable *));
	if (it == 0) cerr << "Memory can not allocated." << endl;
	memcpy(it, iPath, depthBufSize * sizeof(struct indexTable *));
	free(iPath);
	iPath = it;
	depthBufSize+=expandDepth;
  }
};

#endif
