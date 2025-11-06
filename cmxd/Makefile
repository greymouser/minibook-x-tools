# Makefile for Chuwi Minibook X Daemon (cmxd) - New Generation
#
# Copyright (c) 2025 Armando DiCianno <armando@noonshy.com>
# Licensed under GPL-2.0

# Program information
PROGRAM_NAME := cmxd
VERSION := 2.0

# Build configuration
CC ?= gcc
CFLAGS := -std=gnu11 -Wall -Wextra -O2 -g
CPPFLAGS := -D_GNU_SOURCE -DVERSION=\"$(VERSION)\"
LDFLAGS := 
LIBS := -lm

# Installation paths
PREFIX ?= /usr
DESTDIR ?=
BINDIR := $(PREFIX)/sbin
SYSCONFDIR := /etc
SYSTEMDDIR := $(PREFIX)/lib/systemd/system
MANDIR := $(PREFIX)/share/man

# Source files
SOURCES := $(PROGRAM_NAME).c cmxd-calculations.c cmxd-orientation.c cmxd-modes.c cmxd-data.c
OBJECTS := $(SOURCES:.c=.o)
TARGET := $(PROGRAM_NAME)
MANPAGES := $(PROGRAM_NAME).8
DOCFILES := README.md
CONFIGFILES := $(PROGRAM_NAME).conf

# Default target
all: $(TARGET)

# Build the main program
$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

# Compile object files
%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

# Development build with debug symbols and no optimization
debug: CFLAGS := -std=gnu11 -Wall -Wextra -Werror -O0 -g3 -DDEBUG
debug: $(TARGET)

# Static analysis build
analyze: CFLAGS += -fanalyzer
analyze: $(TARGET)

# Clean build artifacts
clean:
	rm -f $(TARGET) $(OBJECTS)

# Distclean - remove all generated files
distclean: clean
	rm -f $(MANPAGES) $(CONFIGFILES)

# Install the program and supporting files
install: $(TARGET)
	install -d $(DESTDIR)$(BINDIR)
	install -m 755 $(TARGET) $(DESTDIR)$(BINDIR)/
	install -d $(DESTDIR)$(SYSCONFDIR)/default
	install -m 644 $(PROGRAM_NAME).conf $(DESTDIR)$(SYSCONFDIR)/default/$(PROGRAM_NAME)

# Uninstall the program and supporting files
uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(TARGET)
	rm -f $(DESTDIR)$(SYSCONFDIR)/default/$(PROGRAM_NAME)

# Install systemd service (optional)
install-systemd: install
	install -d $(DESTDIR)$(SYSTEMDDIR)
	install -m 644 $(PROGRAM_NAME).service $(DESTDIR)$(SYSTEMDDIR)/

# Uninstall systemd service
uninstall-systemd:
	rm -f $(DESTDIR)$(SYSTEMDDIR)/$(PROGRAM_NAME).service

# Test build (syntax check only)
test: $(SOURCES)
	$(CC) $(CPPFLAGS) $(CFLAGS) -fsyntax-only $(SOURCES)

# Help target
help:
	@echo "Available targets:"
	@echo "  all           - Build the program (default)"
	@echo "  debug         - Build with debug symbols and no optimization"
	@echo "  analyze       - Build with static analysis enabled"
	@echo "  clean         - Remove build artifacts"
	@echo "  distclean     - Remove all generated files"
	@echo "  install       - Install program and config files"
	@echo "  uninstall     - Remove installed files"
	@echo "  install-systemd - Install with systemd service"
	@echo "  uninstall-systemd - Remove systemd service"
	@echo "  test          - Syntax check only"
	@echo "  help          - Show this help"

# Declare phony targets
.PHONY: all debug analyze clean distclean install uninstall install-systemd uninstall-systemd test help