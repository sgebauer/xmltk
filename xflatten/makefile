#
#  copyright (C) 2002 Ashish Kumar Gupta,
#                     University of Washington

#
# commands
#
CC	= gcc
CPP	= g++ 
LEX	= flex -l -Pmy
BINDIR  = ${XMLTKROOT}/bin

#
# application sources
# 
SRC		= xflatten.cpp lex.my.c

#
# flags
#
DFADIR	= ../xpathDFA
STREAM_DIR = ../streamIndex # for xpath

LIB_DIR = -L../lib -L${DFADIR}
INC_DIR = -I../include -I${DFADIR}
LIBNAME = -ltsax -lxmltkdfa
LIBRARY_SEARCH_PATHS= -L. ${LIB_DIR}

CFLAGS  = -g -Wall -Wno-ctor-dtor-privacy -Wno-sign-compare -DXMLTK_PREDICATE # -DXMLTK_DEBUG  -DXMLTK_OUTPUT
LDFLAGS = -g ${LIBRARY_SEARCH_PATHS} ${LIBNAME} -lfl #-pg
INCLUDES= -I. ${INC_DIR} -I${STREAM_DIR}

LEXSRC	= cmdline.l

#
# binary
#

TARGET = xflatten

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

lex.my.c: $(LEXSRC)
	$(LEX) $(LEXSRC)

lex.my.o: lex.my.c
