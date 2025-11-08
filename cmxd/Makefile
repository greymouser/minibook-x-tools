# Makefile for Chuwi Minibook X Daemon (cmxd) - New Generation
#
# Copyright (c) 2025 Armando DiCianno <armando@noonshy.com>
# Licensed under GPL-2.0

# Program information
PROGRAM_NAME := cmxd
VERSION := $(shell cat VERSION 2>/dev/null || echo "3.0")

# Build configuration
CC ?= gcc
CFLAGS := -std=gnu11 -Wall -Wextra -O2 -g
CPPFLAGS := -D_GNU_SOURCE -DVERSION=\"$(VERSION)\"
LDFLAGS := 
LIBS := -lm -lpthread

# Installation paths
PREFIX ?= /usr
DESTDIR ?=
BINDIR := $(PREFIX)/sbin
SYSCONFDIR := /etc
SYSTEMDDIR := $(PREFIX)/lib/systemd/system
MANDIR := $(PREFIX)/share/man

# Source files
SRCDIR := src
SOURCES := $(SRCDIR)/$(PROGRAM_NAME).c $(SRCDIR)/cmxd-calculations.c $(SRCDIR)/cmxd-orientation.c $(SRCDIR)/cmxd-modes.c $(SRCDIR)/cmxd-data.c $(SRCDIR)/cmxd-events.c
OBJECTS := $(SOURCES:.c=.o)
TARGET := $(PROGRAM_NAME)
MANPAGES := $(PROGRAM_NAME).8
DOCFILES := README.md
CONFIGFILES := support/$(PROGRAM_NAME).conf
SERVICEFILES := support/$(PROGRAM_NAME).service

# Default target
all: $(TARGET)

# Build the main program
$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

# Compile object files
%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

# Compile object files in src directory  
$(SRCDIR)/%.o: $(SRCDIR)/%.c
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
	rm -f $(MANPAGES)
	rm -rf logs/*.log

# Install the program and supporting files
install: $(TARGET)
	install -d $(DESTDIR)$(BINDIR)
	install -m 755 $(TARGET) $(DESTDIR)$(BINDIR)/
	install -d $(DESTDIR)$(SYSCONFDIR)/default
	install -m 644 $(CONFIGFILES) $(DESTDIR)$(SYSCONFDIR)/default/$(PROGRAM_NAME)
	install -d $(DESTDIR)$(SYSTEMDDIR)
	install -m 644 $(SERVICEFILES) $(DESTDIR)$(SYSTEMDDIR)/
	# Create runtime directory for Unix domain sockets
	install -d $(DESTDIR)/run/cmxd -m 755

# Uninstall the program and supporting files
uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(TARGET)
	rm -f $(DESTDIR)$(SYSCONFDIR)/default/$(PROGRAM_NAME)
	rm -f $(DESTDIR)$(SYSTEMDDIR)/$(PROGRAM_NAME).service

# Test build (syntax check only)
test: $(SOURCES)
	$(CC) $(CPPFLAGS) $(CFLAGS) -fsyntax-only $(SOURCES)

# Test executables
test-all: $(TARGET)
	@echo "Building test executables..."
	@for test in tests/test-*.c; do \
		if [ -f "$$test" ]; then \
			testname=$$(basename "$$test" .c); \
			echo "Building $$testname..."; \
			$(CC) $(CPPFLAGS) $(CFLAGS) -I$(SRCDIR) -o "tests/$$testname" "$$test" $(filter-out $(SRCDIR)/$(PROGRAM_NAME).o,$(OBJECTS)) $(LIBS); \
		fi \
	done
	@if [ -f "tests/analyze-logs.c" ]; then \
		echo "Building analyze-logs..."; \
		$(CC) $(CPPFLAGS) $(CFLAGS) -I$(SRCDIR) -o "tests/analyze-logs" "tests/analyze-logs.c" $(SRCDIR)/cmxd-calculations.o $(LIBS); \
	fi
	@echo "Test executables built in tests/ directory"

# Clean test executables
test-clean:
	rm -f tests/test-* tests/analyze-logs

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
	@echo "  test-all      - Build all test executables"
	@echo "  test-clean    - Remove test executables"
	@echo "  help          - Show this help"

# Declare phony targets
.PHONY: all debug analyze clean distclean install uninstall install-systemd uninstall-systemd test test-all test-clean help