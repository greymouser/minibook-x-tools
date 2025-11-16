# Makefile for Chuwi Minibook X Daemon (cmxd) - New Generation
#
# Copyright (c) 2025 Armando DiCianno <armando@noonshy.com>
# Licensed under GPL-2.0

# Program information
PROGRAM_NAME := cmxd
VERSION := $(shell cat ../VERSION 2>/dev/null || echo "5.0.2")

# Optional features (set to 1 to enable, 0 to disable)
ENABLE_DBUS ?= 1

# Build configuration
CC ?= gcc
CFLAGS := -std=gnu11 -Wall -Wextra -O2 -g
CPPFLAGS := -D_GNU_SOURCE -DVERSION=\"$(VERSION)\"
LDFLAGS := 
LIBS := -lm -lpthread

# DBus support configuration
ifeq ($(ENABLE_DBUS),1)
    CPPFLAGS += -DENABLE_DBUS=1
    LIBS += $(shell pkg-config --libs dbus-1 2>/dev/null || echo "-ldbus-1")
    CFLAGS += $(shell pkg-config --cflags dbus-1 2>/dev/null)
endif

# Installation paths
PREFIX ?= /usr
DESTDIR ?=
BINDIR := $(PREFIX)/sbin
LIBDIR := $(PREFIX)/lib
INCLUDEDIR := $(PREFIX)/include/libcmx
SYSCONFDIR := /etc
SYSTEMDDIR := $(PREFIX)/lib/systemd/system
MANDIR := $(PREFIX)/share/man

# Source files
SRCDIR := src
DAEMON_SOURCES := $(SRCDIR)/$(PROGRAM_NAME).c $(SRCDIR)/cmxd-calculations.c $(SRCDIR)/cmxd-orientation.c $(SRCDIR)/cmxd-modes.c $(SRCDIR)/cmxd-data.c $(SRCDIR)/cmxd-events.c

# Add DBus module if enabled
ifeq ($(ENABLE_DBUS),1)
    DAEMON_SOURCES += $(SRCDIR)/cmxd-dbus.c
endif

LIB_SOURCES := $(SRCDIR)/cmxd-protocol.c
DAEMON_OBJECTS := $(DAEMON_SOURCES:.c=.o)
LIB_OBJECTS := $(LIB_SOURCES:.c=.o)
ALL_OBJECTS := $(DAEMON_OBJECTS) $(LIB_OBJECTS)

# Targets
TARGET := $(PROGRAM_NAME)
LIBRARY := libcmx.so
LIBRARY_SONAME := $(LIBRARY).1
LIBRARY_FULLNAME := $(LIBRARY).1.0.0
STATIC_LIBRARY := libcmx.a
HEADER_FILES := $(SRCDIR)/cmxd-protocol.h

# Documentation and config
MANPAGES := $(PROGRAM_NAME).8
DOCFILES := README.md
CONFIGFILES := support/$(PROGRAM_NAME).conf
SERVICEFILES := support/$(PROGRAM_NAME).service

# Default target
all: $(TARGET) $(LIBRARY)

# Build the main program (link with the library)
$(TARGET): $(DAEMON_OBJECTS) $(LIBRARY)
	$(CC) $(LDFLAGS) -o $@ $(DAEMON_OBJECTS) -L. -lcmx $(LIBS)

# Build shared library
$(LIBRARY): $(LIBRARY_FULLNAME)
	ln -sf $(LIBRARY_FULLNAME) $(LIBRARY)
	ln -sf $(LIBRARY_FULLNAME) $(LIBRARY_SONAME)

$(LIBRARY_FULLNAME): $(LIB_OBJECTS)
	$(CC) -shared -fPIC -Wl,-soname,$(LIBRARY_SONAME) $(LDFLAGS) -o $@ $^

# Build static library (optional)
$(STATIC_LIBRARY): $(LIB_OBJECTS)
	ar rcs $@ $^

# Compile object files for daemon
%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

# Compile object files in src directory for daemon
$(SRCDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -fPIC -c -o $@ $<

# Development build with debug symbols and no optimization
debug: CFLAGS := -std=gnu11 -Wall -Wextra -Werror -O0 -g3 -DDEBUG
debug: $(TARGET)

# Static analysis build
analyze: CFLAGS += -fanalyzer
analyze: $(TARGET)

# Clean build artifacts
clean:
	rm -f $(TARGET) $(ALL_OBJECTS) $(LIBRARY) $(LIBRARY_SONAME) $(LIBRARY_FULLNAME) $(STATIC_LIBRARY)

# Distclean - remove all generated files
distclean: clean
	rm -f $(MANPAGES)
	rm -rf logs/*.log

# Install the program, library and supporting files
install: $(TARGET) $(LIBRARY)
	# Install daemon
	install -d $(DESTDIR)$(BINDIR)
	install -m 755 $(TARGET) $(DESTDIR)$(BINDIR)/
	# Install shared library
	install -d $(DESTDIR)$(LIBDIR)
	install -m 755 $(LIBRARY_FULLNAME) $(DESTDIR)$(LIBDIR)/
	ln -sf $(LIBRARY_FULLNAME) $(DESTDIR)$(LIBDIR)/$(LIBRARY_SONAME)
	ln -sf $(LIBRARY_FULLNAME) $(DESTDIR)$(LIBDIR)/$(LIBRARY)
	# Install headers
	install -d $(DESTDIR)$(INCLUDEDIR)
	install -m 644 $(HEADER_FILES) $(DESTDIR)$(INCLUDEDIR)/
	# Install configuration
	install -d $(DESTDIR)$(SYSCONFDIR)/default
	install -m 644 $(CONFIGFILES) $(DESTDIR)$(SYSCONFDIR)/default/$(PROGRAM_NAME)
	# Install systemd service
	install -d $(DESTDIR)$(SYSTEMDDIR)
	install -m 644 $(SERVICEFILES) $(DESTDIR)$(SYSTEMDDIR)/
	# Install DBus policy (if DBus is enabled)
ifeq ($(ENABLE_DBUS),1)
	install -d $(DESTDIR)$(SYSCONFDIR)/dbus-1/system.d
	install -m 644 support/cmxd-dbus.conf $(DESTDIR)$(SYSCONFDIR)/dbus-1/system.d/
endif
	# Update ldconfig
	ldconfig
	# Create runtime directory for Unix domain sockets
	install -d $(DESTDIR)/run/cmxd -m 755

# Uninstall the program, library and supporting files
uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(TARGET)
	rm -f $(DESTDIR)$(LIBDIR)/$(LIBRARY) $(DESTDIR)$(LIBDIR)/$(LIBRARY_SONAME) $(DESTDIR)$(LIBDIR)/$(LIBRARY_FULLNAME)
	rm -rf $(DESTDIR)$(INCLUDEDIR)
	rm -f $(DESTDIR)$(SYSCONFDIR)/default/$(PROGRAM_NAME)
	rm -f $(DESTDIR)$(SYSTEMDDIR)/$(PROGRAM_NAME).service
ifeq ($(ENABLE_DBUS),1)
	rm -f $(DESTDIR)$(SYSCONFDIR)/dbus-1/system.d/cmxd-dbus.conf
endif
	ldconfig

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
	@echo "  all           - Build the program and library (default)"
	@echo "  $(TARGET)     - Build the daemon program"
	@echo "  $(LIBRARY)    - Build the shared library"
	@echo "  debug         - Build with debug symbols and no optimization"
	@echo "  analyze       - Build with static analysis enabled"
	@echo "  clean         - Remove build artifacts"
	@echo "  distclean     - Remove all generated files"
	@echo "  install       - Install program, library and config files"
	@echo "  uninstall     - Remove installed files"
	@echo "  install-systemd - Install with systemd service"
	@echo "  uninstall-systemd - Remove systemd service"
	@echo "  test          - Syntax check only"
	@echo "  test-all      - Build all test executables"
	@echo "  test-clean    - Remove test executables"
	@echo "  help          - Show this help"
	@echo ""
	@echo "Build options:"
	@echo "  ENABLE_DBUS=$(ENABLE_DBUS) - Enable DBus support (1=on, 0=off)"
	@echo "  test-all      - Build all test executables"
	@echo "  test-clean    - Remove test executables"
	@echo "  help          - Show this help"

# Declare phony targets
.PHONY: all debug analyze clean distclean install uninstall install-systemd uninstall-systemd test test-all test-clean help