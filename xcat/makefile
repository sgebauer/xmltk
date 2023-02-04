#
#  copyright (C) 2001 TJ Green, Makoto Onizuka, Dan Suciu, 
#                     University of Washington
#  $Id: makefile,v 1.2 2002/05/22 21:45:52 tjgreen Exp $

#
# commands
#

#
# flags
#
LIB_DIR = -L${XMLTKROOT}/lib -L${XMLTKROOT}/xpathDFA
LIBNAME = -ltsax -lxmltkdfa -lfl
LIBRARY_SEARCH_PATHS= -L. ${LIB_DIR}

#
# generic makefile
#
include $(XMLTKROOT)/general.mk

INCLUDES= -I. -I${XMLTKROOT}/include

BINDIR  = ${XMLTKROOT}/bin

#
# application sources
# 
SRC	= main.cpp

#
# binary
#

TARGET = xcat

.SUFFIXES: .o .cpp

.cpp.o:
	$(CXX) $(INCLUDES) -c $(CFLAGS) $*.cpp

OBJ	= $(SRC:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) -o $@ $(OBJ) $(LDFLAGS)

depend: $(SRC:.cpp=.d) ${XMLTK_DIR}/lib/libtsax.a

install: $(TARGET)
	cp $(TARGET) $(BINDIR)

clean:
	rm -f $(TARGET) $(OBJ) *.d core
