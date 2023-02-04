#
#  copyright (C) 2001 TJ Green, Makoto Onizuka, Dan Suciu, 
#                     University of Washington
#  $Id: makefile,v 1.4 2002/08/13 12:59:46 monizuka Exp $

#
# commands
#
LEX	= flex -l
# YACC	= yacc -b xmatch # -v
YACC	= bison # -v
BINDIR  = ${XMLTKROOT}/bin

#
# application sources
# 
# SRC	= XmatchHandlers.cxx Xmatch.cxx # for xerces
SRC	= XmatchHandlersXmill.cxx XmatchXmill.cxx # for xmill XML parser

#
# flags
#
STREAM_DIR = $(XMLTKROOT)/streamIndex # for xpath

LIB_DIR = -L$(XMLTKROOT)/lib
INC_DIR = -I$(XMLTKROOT)/include
LIBNAME = -ltsax

LIBRARY_SEARCH_PATHS= -L. ${LIB_DIR}

MYCFLAGS = -DTOKEN_PARSER -DXMLTK_CONTEXT
MYLDFLAGS = -ll

#
# generic makefile
#
include $(XMLTKROOT)/general.mk


INCLUDES= -I. ${INC_DIR} -I${STREAM_DIR}

#
# xpath FA sources
# 
LEXSRC	= xmatch.l
YACCSRC = xmatch.y
LIBSRC	= Error.cxx List.cxx Root.cxx Node.cxx XPath.cxx Variable.cxx Query.cxx Automata.cxx Predicate.cxx dfafilter.cxx
LIBCSRC	= xmatch.tab.c lex.yy.c addressComp.c
LIB	= libxmltkdfa.a

#
# binary
#

.SUFFIXES: .o .c .cxx

.cxx.o:
	$(CXX) $(INCLUDES) -c $(CFLAGS) $*.cxx

.c.o:
	$(CC) -c $(CFLAGS) $*.c

LIBOBJ	= $(LIBSRC:.cxx=.o) $(LIBCSRC:.c=.o)
OBJ	= $(SRC:.cxx=.o)

all: $(LIB)

$(LIB): $(LIBOBJ)
	rm -f $(LIB)
	$(AR) -rs $(LIB) $(LIBOBJ)
	ranlib $(LIB)

depend: $(SRC:.cxx=.d)

install:$(LIB)

clean:
	rm -f $(LIB) $(LIBOBJ) $(OBJ) *.d $(CSRC) xmatch.tab.h xmatch.tab.cxx xmatch.tab.c lex.yy.c core

lex.yy.c: $(LEXSRC)
	$(LEX) $(LEXSRC)

xmatch.tab.cxx xmatch.tab.h:$(YACCSRC)
	$(YACC) -d $(YACCSRC); mv xmatch.tab.c xmatch.tab.cxx

lex.yy.o: lex.yy.c xmatch.tab.h
