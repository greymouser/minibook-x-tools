# Makefile for Chuwi Minibook X Daemon
#
# Copyright (c) 2025 Armando DiCianno <armando@noonshy.com>
# Licensed under GPL-2.0

# Program information
PROGRAM_NAME := chuwi-minibook-x-daemon
VERSION := 1.0

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
SOURCES := $(PROGRAM_NAME).c
OBJECTS := $(SOURCES:.c=.o)
TARGET := $(PROGRAM_NAME)
MANPAGES := $(PROGRAM_NAME).8
DOCFILES := BINARY_DOCUMENTATION.md
CONFIGFILES := $(PROGRAM_NAME).default

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
	rm -f $(TARGET) $(OBJECTS) *.core

# Install the program and supporting files
install: $(TARGET) install-config install-systemd install-man install-doc
	@echo "Installing $(PROGRAM_NAME)..."
	install -d $(DESTDIR)$(BINDIR)
	install -m 755 $(TARGET) $(DESTDIR)$(BINDIR)/$(PROGRAM_NAME)
	@echo "Installation completed successfully"

# Install configuration files
install-config: $(PROGRAM_NAME).conf
	@echo "Installing configuration files..."
	install -d $(DESTDIR)$(SYSCONFDIR)/default
	@if [[ ! -f $(DESTDIR)$(SYSCONFDIR)/default/$(PROGRAM_NAME) ]]; then \
		install -m 644 $(PROGRAM_NAME).conf $(DESTDIR)$(SYSCONFDIR)/default/$(PROGRAM_NAME); \
		echo "Configuration file installed: $(DESTDIR)$(SYSCONFDIR)/default/$(PROGRAM_NAME)"; \
	else \
		echo "Configuration file already exists, not overwriting"; \
	fi

# Install systemd service files
install-systemd: $(PROGRAM_NAME).service
	@echo "Installing systemd service..."
	install -d $(DESTDIR)$(SYSTEMDDIR)
	install -m 644 $(PROGRAM_NAME).service $(DESTDIR)$(SYSTEMDDIR)/

# Install man pages
install-man: $(MANPAGES)
	@echo "Installing man pages..."
	install -d $(DESTDIR)$(MANDIR)/man8
	install -m 644 $(PROGRAM_NAME).8 $(DESTDIR)$(MANDIR)/man8/

# Install documentation
install-doc: $(DOCFILES)
	@echo "Installing documentation..."
	install -d $(DESTDIR)$(PREFIX)/share/doc/$(PROGRAM_NAME)
	install -m 644 $(DOCFILES) $(DESTDIR)$(PREFIX)/share/doc/$(PROGRAM_NAME)/

# Uninstall everything
uninstall:
	@echo "Uninstalling $(PROGRAM_NAME)..."
	rm -f $(DESTDIR)$(BINDIR)/$(PROGRAM_NAME)
	rm -f $(DESTDIR)$(SYSTEMDDIR)/$(PROGRAM_NAME).service
	rm -f $(DESTDIR)$(MANDIR)/man8/$(PROGRAM_NAME).8
	rm -rf $(DESTDIR)$(PREFIX)/share/doc/$(PROGRAM_NAME)
	@echo "Note: Configuration file $(DESTDIR)$(SYSCONFDIR)/default/$(PROGRAM_NAME) preserved"

# Test the build
test: $(TARGET)
	@echo "Running basic tests..."
	./$(TARGET) --help
	./$(TARGET) --version
	@echo "Basic tests passed"

# Check for common issues
check: $(TARGET)
	@echo "Running checks..."
	@if command -v valgrind >/dev/null 2>&1; then \
		echo "Running memory check..."; \
		timeout 5s valgrind --leak-check=brief --error-exitcode=1 \
			./$(TARGET) --help >/dev/null || echo "Memory check failed"; \
	fi
	@if command -v cppcheck >/dev/null 2>&1; then \
		echo "Running static analysis..."; \
		cppcheck --enable=warning,style --error-exitcode=1 $(SOURCES); \
	fi

# Show program information
info:
	@echo "Program: $(PROGRAM_NAME) v$(VERSION)"
	@echo "Target: $(TARGET)"
	@echo "Sources: $(SOURCES)"
	@echo "Man pages: $(MANPAGES)"
	@echo "Install prefix: $(PREFIX)"
	@if [ -f $(TARGET) ]; then \
		echo "Binary size: $$(stat -c%s $(TARGET)) bytes"; \
		echo "Binary info: $$(file $(TARGET))"; \
	fi

# Development helpers
run: $(TARGET)
	@echo "Running $(TARGET) with default settings..."
	./$(TARGET) --verbose

gdb: debug
	gdb ./$(TARGET)

# View man pages
man: $(MANPAGES)
	@echo "Viewing man page for $(PROGRAM_NAME)..."
	man -l $(PROGRAM_NAME).8

# Package for distribution
dist: clean
	@echo "Creating distribution package..."
	tar -czf $(PROGRAM_NAME)-$(VERSION).tar.gz \
		*.c Makefile README.md *.service *.8 *.conf validate-config.sh

# Help target
help:
	@echo "Available targets:"
	@echo "  all (default) - Build the program"
	@echo "  debug         - Build with debug symbols"
	@echo "  analyze       - Build with static analysis"
	@echo "  clean         - Clean build artifacts"
	@echo "  install       - Install program and supporting files"
	@echo "  install-config - Install configuration files only"
	@echo "  install-systemd - Install systemd service only"
	@echo "  install-man   - Install man pages only"
	@echo "  uninstall     - Remove installed files"
	@echo "  test          - Run basic functionality tests"
	@echo "  check         - Run static analysis and memory checks"
	@echo "  info          - Show program information"
	@echo "  run           - Build and run with default settings"
	@echo "  gdb           - Build debug version and run in gdb"
	@echo "  man           - View main program man page"
	@echo "  dist          - Create distribution package"
	@echo "  help          - Show this help"
	@echo ""
	@echo "Variables:"
	@echo "  CC            - Compiler (default: gcc)"
	@echo "  CFLAGS        - Compiler flags"
	@echo "  PREFIX        - Install prefix (default: /usr)"
	@echo "  DESTDIR       - Staging directory for packaging"

# Dependencies
$(PROGRAM_NAME).o: $(PROGRAM_NAME).c

.PHONY: all debug analyze clean install install-config install-systemd install-man uninstall test check info run gdb man dist help