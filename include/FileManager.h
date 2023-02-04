// -*- mode: c++ -*-
//  copyright (C) 2001 Makoto Onizuka, University of Washington

#if ! defined(__FILEMANAGER_H__)
#define __FILEMANAGER_H__

#ifdef WIN32
#define getpagesize() (8192)
#endif

#include <stdio.h>
#include <stdlib.h>
#ifndef WIN32
#include <unistd.h>
#include <iostream.h>
#else
#include <iostream>
#endif

class FileManager {
public:
  enum seekType{
	T_SEEK_SET = 0,
	T_SEEK_CUR,
	T_SEEK_END
  };
protected:
  char *		fileName;
  FILE *		fp;
  unsigned int	bufSize;		// current Buffer size
  unsigned int	posInBuf;		// position in the current Buffer
  unsigned int	baseInFile;		// current Buffer position in the file
  void *		buf;			// Buffer
public:
  FileManager(const char * sFile):fp(0),bufSize(0),posInBuf(0),baseInFile(0),buf(0){
	fileName = strdup(sFile);
  }
  virtual ~FileManager() {
	if (fileName != 0) free(fileName);
	if (fp != 0) fclose(fp);
	if (buf != 0) free(buf);
  }
  bool 	open(void){
	fp = fopen(fileName, "r");
	if (fp == 0) return false;
	buf = malloc(getpagesize());
	if (buf == 0) return false;
	return true;
  }
  size_t	read(void * mem, size_t size){
	if (posInBuf + size < bufSize){ // buffer is enough
	  memcpy(mem, (char *)buf + posInBuf, size);
	  posInBuf += size;
	  return size;
	}
	else {						// buffer is not enough
	  unsigned int left = bufSize - posInBuf;
	  memcpy(mem, (char *)buf + posInBuf, left);
	  if (readBuf() == 0) return 0;
	  else return left + read((char *)mem + left, size - left);
	}
  }
  int		seek(long offset, seekType w){
	switch (w){
	case T_SEEK_CUR:
	  if (posInBuf + offset < bufSize){ // buffer is enough
		posInBuf += offset;
		return 0;
	  }
	  else {						// buffer is not enough
		int retval = fseek(fp, offset + posInBuf - bufSize, SEEK_CUR);
		baseInFile += offset + posInBuf;
		bufSize = posInBuf = 0;			// initialize
		return retval;
	  }
	  break;
	case T_SEEK_SET:
	  if (offset >= baseInFile && offset < baseInFile + bufSize){ // buffer is enough
		posInBuf = offset - baseInFile;
		return 0;
	  }
	  else {						// buffer is not enough
		int retval = fseek(fp, offset, SEEK_SET);
		baseInFile = offset;
		bufSize = posInBuf = 0;			// initialize
		return retval;
	  }
	  break;
	default:					// case T_SEEK_END:
	  cerr << "not implemented" << endl;
	  return -1;
	}
  }
private:
  size_t	readBuf(void){
	baseInFile += bufSize;
	posInBuf = 0;
	bufSize = fread(buf, 1, getpagesize(), fp);
	if (bufSize == 0){
	  cerr << "The end of file." << endl;
	}
	return bufSize;
  }
};

#endif
