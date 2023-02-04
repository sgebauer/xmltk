# Description: A simple makefile for the xtraverse tool
# Author:      Iliana Avila-Campillo
# Date:        July 24, 2001

#
# commands
#
CC	= gcc
CPP	= g++ 
BINDIR  = ${XMLTKROOT}/bin

TARGET = file2xml


all: $(TARGET)

install: $(TARGET)
	cp $(TARGET) $(BINDIR)

file2xml : xtraverse.o pathstack.o depthstack.o SysTimer.o
	g++ xtraverse.o pathstack.o depthstack.o SysTimer.o -o file2xml
pathstack.o : pathstack.cxx pathstack.h
	g++ -c pathstack.cxx
depthstack.o : depthstack.cxx depthstack.h
	g++ -c depthstack.cxx
xtraverse.o : xtraverse.cxx xtraverse.h
	g++ -c xtraverse.cxx
SysTimer.o : SysTimer.cxx SysTimer.h
	g++ -c SysTimer.cxx
clean :
	rm -f file2xml *.o core
