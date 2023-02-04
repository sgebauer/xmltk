#
#  copyright (C) 2001 Makoto Onizuka, University of Washington
#  $Id: Makefile,v 1.4 2002/05/29 08:50:31 tjgreen Exp $

# for linux, gcc 2.96
SUBDIR	= lib xpathDFA xagg xcat xsort xtail xrun xstat streamIndex file2xml \
          xdelete xflatten xparse

all:
	for dir in $(SUBDIR) ; do \
	  cd $${dir} ; $(MAKE); cd ..; \
	done

install: 
	for dir in $(SUBDIR) ; do \
	  cd $${dir} ; $(MAKE) install; cd ..; \
	done

clean:
	for dir in $(SUBDIR) ; do \
	  cd $${dir} ; $(MAKE) clean; cd ..;\
	done
