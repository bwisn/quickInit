PROJECT_NAME=quickinit

GIT_VERSION := "$(shell git describe --abbrev=4 --dirty --always --tags)"

SRCDIR := src
INCDIR := inc
BUILDDIR := obj
TARGETDIR := bin
SRCEXT := c
OBJEXT := o

SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.$(OBJEXT)))

#Compiler
CC=gcc

# Flags for compiler
CFLAGS := -O2 -Wall 
CFLAGS += -I $(INCDIR) -lpthread -DQUICKINIT_VERSION=\"$(GIT_VERSION)\"

.PHONY: proj

all: proj

proj: $(BUILDDIR)/init

$(BUILDDIR)/init: $(SOURCES)
	@mkdir -p $(TARGETDIR)
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) $^ -o $@
	@cp $(BUILDDIR)/init $(TARGETDIR)/init
	@echo Done!

clean:
	rm -f $(BUILDDIR)/*.o $(TARGETDIR)/init $(BUILDDIR)/init
