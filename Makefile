# ------------------------------------------------
# Generic Makefile
#
# Author: yanick.rochon@gmail.com
# Date  : 2011-08-10
#
# Changelog :
#   2010-11-05 - first version
#   2011-08-10 - added structure : sources, objects, binaries
#                thanks to http://stackoverflow.com/users/128940/beta
# ------------------------------------------------

# project name (generate executable with this name)
TARGET   = fwtool

CC       = gcc
# compiling flags here
CFLAGS   = -std=c99 -Wall -DNDEBUG -DIOAPI_NO_64 -DFWT_CONSOLE -D__USE_BSD

LINKER   = gcc -o
# linking flags here
LDFLAGS   = -lz -larchive

# change these to set the proper directories where each files shoould be
SRCDIR   = src
OBJDIR   = obj
BINDIR   = bin

SOURCES  := $(wildcard $(SRCDIR)/*.c)
INCLUDES := $(wildcard $(SRCDIR)/*.h)
OBJECTS  := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
rm       = rm -f


$(BINDIR)/$(TARGET): $(OBJECTS)
	$(LINKER) $@ $(OBJECTS) $(LDFLAGS)

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONEY: clean
clean:
	@$(rm) $(OBJECTS)

.PHONEY: remove
remove: clean
	$(rm) $(BINDIR)/$(TARGET)
