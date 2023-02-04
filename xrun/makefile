#
#  copyright (C) 2001 TJ Green, Makoto Onizuka, Dan Suciu, 
#                     University of Washington
#  $Id: makefile,v 1.2 2002/05/30 16:49:02 tjgreen Exp $

#
# commands
#
CC	= gcc
CPP	= g++ 
BINDIR  = ${XMLTKROOT}/bin

#
# application sources
# 
SRC		= xrun.cpp

#
# flags
#
DFADIR	= ../xpathDFA
STREAM_DIR = ../streamIndex # for xpath

LIB_DIR = -L../lib -L${DFADIR}
INC_DIR = -I../include -I${DFADIR}
LIBNAME = -ltsax -lxmltkdfa
LIBRARY_SEARCH_PATHS= -L. ${LIB_DIR}

#
# generic makefile
#
include ${XMLTKROOT}/general.mk

INCLUDES= -I. ${INC_DIR} -I${STREAM_DIR}

#
# binary
#

TARGET = xrun

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
