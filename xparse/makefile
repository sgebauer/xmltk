#
#  copyright (C) 2001 TJ Green, Makoto Onizuka, Dan Suciu, 
#                     University of Washington
#  $Id: makefile,v 1.1 2002/05/29 08:49:09 tjgreen Exp $

#
# commands
#
CC	= gcc
CXX	= g++ 

#
# flags
#
LIB_DIR = -L${XMLTKROOT}/lib -L${XMLTKROOT}/xpathDFA
LIBNAME = -ltsax -lxmltkdfa -lfl
LIBRARY_SEARCH_PATHS= -L. ${LIB_DIR}

include ${XMLTKROOT}/general.mk

INCLUDES= -I. -I${XMLTKROOT}/include

BINDIR  = ${XMLTKROOT}/bin

#
# application sources
# 
SRC	= main.cpp

#
# binary
#

TARGET = xparse

.SUFFIXES: .o .cpp

.cpp.o:
	$(CXX) $(INCLUDES) -c $(CFLAGS) $*.cpp

OBJ	= $(SRC:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) -o $@ $(OBJ) $(LDFLAGS)

depend: $(SRC:.cpp=.d) ${XMLTKROOT}/lib/libtsax.a

install: $(TARGET)
	cp $(TARGET) $(BINDIR)

clean:
	rm -f $(TARGET) $(OBJ) *.d core
