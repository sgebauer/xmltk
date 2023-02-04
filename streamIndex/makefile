#
#  copyright (C) 2001 TJ Green, Makoto Onizuka, Dan Suciu, 
#                     University of Washington
#  $Id: makefile,v 1.1.1.1 2002/05/04 12:54:07 tjgreen Exp $

#
# commands
#
CC	= gcc
CPP	= g++ 
BINDIR  = ${XMLTKROOT}/bin

#
# application sources
# 
SRC		= createSindex.cpp

#
# flags
#
DFADIR	= ../xpathDFA
STREAM_DIR = ../streamIndex # for xpath

LIB_DIR = -L../lib -L${DFADIR}
INC_DIR = -I../include -I${DFADIR}
LIBNAME = -ltsax -lxmltkdfa
LIBRARY_SEARCH_PATHS= -L. ${LIB_DIR}

CFLAGS  = -g -Wno-ctor-dtor-privacy -Wno-sign-compare -DXMLTK_PREDICATE # -DXMLTK_OUTPUT 
LDFLAGS = -g ${LIBRARY_SEARCH_PATHS} ${LIBNAME} -lfl #-pg
INCLUDES= -I. ${INC_DIR} -I${STREAM_DIR}

#
# binary
#

TARGET = createSindex

.SUFFIXES: .o .c .cpp

.cpp.o:
	$(CPP) $(INCLUDES) -c $(CFLAGS) $*.cpp

.c.o:
	$(CC) -c $(CFLAGS) $*.c

OBJ	= $(SRC:.cpp=.o)

all: $(TARGET)

$(TARGET): $(LIB) $(OBJ) 
	$(CPP) -o $@ $(OBJ) $(LDFLAGS)

depend: $(SRC:.cpp=.d)

install: $(TARGET)
	cp $(TARGET) $(BINDIR)

clean:
	rm -f $(TARGET) $(LIB) $(LIBOBJ) $(OBJ) *.d core
