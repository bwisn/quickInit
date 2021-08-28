PROJECT_NAME=quickinit

GIT_VERSION := "$(shell git describe --abbrev=4 --dirty --always --tags)"

SRCDIR := src
INCDIR := inc
BUILDDIR := obj
TARGETDIR := bin
SRCEXT := c

SOURCES := $(shell find $(SRCDIR) -maxdepth 1 -type f -name *.$(SRCEXT))
SOURCES_TELINIT := $(shell find $(SRCDIR)/telinit -maxdepth 1 -type f -name *.$(SRCEXT))

#Compiler
CC=$(CROSS_COMPILE)gcc

# Flags for compiler
CFLAGS := -O2 -Wall 
CFLAGS += -I $(INCDIR) -pthread -DQUICKINIT_VERSION=\"$(GIT_VERSION)\"

.PHONY: proj

all: proj

proj: $(BUILDDIR)/init $(BUILDDIR)/telinit


$(BUILDDIR)/init: $(SOURCES)
	@mkdir -p $(TARGETDIR)
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) $^ -o $@
	@cp $(BUILDDIR)/init $(TARGETDIR)/init
	@echo Done!

$(BUILDDIR)/telinit: $(SOURCES_TELINIT)
	@mkdir -p $(TARGETDIR)
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) $^ -o $@
	@cp $(BUILDDIR)/telinit $(TARGETDIR)/telinit
	@echo Done!

clean:
	rm -f $(BUILDDIR)/*.o $(BUILDDIR)/telinit/*.o $(TARGETDIR)/init \
	$(BUILDDIR)/init $(TARGETDIR)/telinit $(BUILDDIR)/telinit
