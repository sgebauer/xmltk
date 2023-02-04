#
#  copyright (C) 2001 TJ Green, Makoto Onizuka, Dan Suciu, 
#                     University of Washington
#  $Id: makefile,v 1.3 2002/05/22 21:45:52 tjgreen Exp $

#
# commands
#
LEX	= flex -l -Pmy
BINDIR  = ${XMLTKROOT}/bin

#
# application sources
# 
SRC		= xagg.cpp 

#
# flags
#
DFADIR	= $(XMLTKROOT)/xpathDFA
STREAM_DIR = $(XMLTKROOT)/streamIndex # for xpath

LIB_DIR = -L$(XMLTKROOT)/lib -L${DFADIR}
INC_DIR = -I$(XMLTKROOT)/include -I${DFADIR}
LIBNAME = -ltsax -lxmltkdfa
LIBRARY_SEARCH_PATHS= -L. ${LIB_DIR}

MYLDFLAGS = -lfl

#
# generic makefile
#
include $(XMLTKROOT)/general.mk

INCLUDES= -I. ${INC_DIR} -I${STREAM_DIR}

LEXSRC	= cmdline.l

#
# binary
#

TARGET = xagg

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
