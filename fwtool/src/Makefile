# -*- makefile -*-
# Copyright 2011 Nicolas Limare <nicolas.limare@cmla.ens-cachan.fr>
#
# Copying and distribution of this file, with or without
# modification, are permitted in any medium without royalty provided
# the copyright notice and this notice are preserved.  This file is
# offered as-is, without any warranty.

# generic makefile for C and C++ code
#
# It should work with any C and C++ compiler, and any code using a set
# of C and C++ files to build a single executable. You need to
# customize, at least,
# - the BIN variable with the name of your executable program file
# - the LDLIBS variable with the libraries used (-lm -lpng...)
#
# You can simplify this makefile by removing the C or C++ parts if
# your code only uses one language.

# C source and object files (autodetected in the current directory)
CSRC	= $(wildcard *.c)
COBJ	= $(CSRC:.c=.o)
# C source and object files (autodetected in the current directory)
CXXSRC	= $(wildcard *.cpp)
CXXOBJ	= $(CXXSRC:.c=.o)

# C optimization and options
COPT	= -O2
CFLAGS	= $(COPT) 
# C++ optimization and options
CXXOPT	= -O2
CXXFLAGS	= $(CXXOPT) 

# preprocessor options (NDEBUG disables assert())
CPPFLAGS	= -I. -DNDEBUG -DIOAPI_NO_64 -DFWT_CONSOLE
# linker options (-Lxxx for the library path)
LDFLAGS	= 
# libraries (use the -lfoo -lbar syntax)
LDLIBS	= -lz -larchive

# binary output
BIN	= a.out

# default target
default	: all
all	: $(BIN)

# dependencies collected with cc -MM
#-include Makefile.dep
#Makefile.dep	: $(CSRC) $(CXXSRC)
#	$(CC) -MM $(CPPFLAGS) $^ > $@

# C object compilation
%.o	: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@
# C++ object compilation
%.o	: %.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

# final link
$(BIN)	:  $(COBJ) $(CXXOBJ)
ifeq ($(CXXOBJ),)
	$(CC) $^ $(LDFLAGS) $(LDLIBS) -o $@
else
	$(CXX) $^ $(LDFLAGS) $(LDLIBS) -o $@
endif

###########
# utilities

clean	: 
	$(RM) $(COBJ) $(CXXOBJ)

distclean	: clean
	$(RM) $(EXEC)

debug	:
	$(MAKE) CPPFLAGS="$(CPPFLAGS) -UNDEBUG" \
		CFLAGS="$(CFLAGS) -g" \
		CXXFLAGS="$(CXXFLAGS) -g"

CSTRICT	= -ansi -pedantic -Wall -Wextra -Werror
CXXSTRICT	= -ansi -pedantic -Wall -Wextra -Werror
lint: 
	$(MAKE) CFLAGS="$(CFLAGS) $(CSTRICT)" \
		CXXFLAGS="$(CXXFLAGS) $(CXXSTRICT)"
