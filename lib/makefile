#
#  copyright (C) 2001 TJ Green, Makoto Onizuka, Dan Suciu, 
#                     University of Washington
#  $Id: makefile,v 1.3 2002/06/18 01:59:53 monizuka Exp $


#
# flags
#

LIBRARY_SEARCH_PATHS= -L.

#
# generic makefile
#
include $(XMLTKROOT)/general.mk


INCLUDES= -I. -I${XMLTKROOT}/include

#
# application sources
# 
SRC	= tokenmap.cpp tokentable.cpp tsax2xml.cpp tsax2bin.cpp bin2tsax.cpp util.cpp xml2tsax.cpp error.cpp memman.cpp memstreamer.cpp filestream.cpp memstream.cpp typefilter.cpp tsax2nil.cpp
LIB	= libtsax.a

#
# binary
#

.SUFFIXES: .o .cpp

.cpp.o:
	$(CXX) $(INCLUDES) -c $(CFLAGS) $*.cpp

OBJ	= $(SRC:.cpp=.o)

$(LIB): $(OBJ)
	rm -f $(LIB)
	$(AR) -rs $(LIB) $(OBJ)
	ranlib $(LIB)

depend: $(SRC:.cpp=.d)

install:$(LIB)

clean:
	rm -f $(LIB) $(OBJ) *.d core

