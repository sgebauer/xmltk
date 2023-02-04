
#
# commands
#
AR	= ar
CC	= gcc
CPP	= g++
CXX	= g++ 
BINDIR  = ${XMLTKROOT}/bin

#
# flags
#

CFLAGS	= -O -Wall -Wno-ctor-dtor-privacy -Wno-sign-compare -DXMLTK_PREDICATE $(MYCFLAGS)
#CFLAGS	= -g -Wall -Wno-ctor-dtor-privacy -Wno-sign-compare -DXMLTK_PREDICATE $(MYCFLAGS)
LDFLAGS = ${LIBRARY_SEARCH_PATHS} ${LIBNAME} $(MYLDFLAGS)
#LDFLAGS = -g ${LIBRARY_SEARCH_PATHS} ${LIBNAME} $(MYLDFLAGS)
