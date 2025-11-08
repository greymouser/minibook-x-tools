# Hyprland Tablet Mode Integration Makefile
# Copyright (c) 2025 Armando DiCianno <armando@noonshy.com>
# Licensed under GPL-2.0

PROGRAM_NAME = tablet-mode-daemon
VERSION = 1.0

# Installation paths
PREFIX ?= /usr
BINDIR := $(PREFIX)/bin
MANDIR := $(PREFIX)/share/man
SYSTEMD_USER_DIR := $(PREFIX)/lib/systemd/user
DOCDIR := $(PREFIX)/share/doc/$(PROGRAM_NAME)
EXAMPLEDIR := $(DOCDIR)/examples

# Build configuration
CC ?= gcc
CFLAGS ?= -O2 -Wall -Wextra
CPPFLAGS += -DVERSION=\"$(VERSION)\" -DPROGRAM_NAME=\"$(PROGRAM_NAME)\"
LDFLAGS ?=

# libcmx configuration (prefer installed version, fallback to local)
LIBCMX_INCLUDE_INSTALLED := /usr/include/libcmx
LIBCMX_LIB_INSTALLED := /usr/lib
LIBCMX_INCLUDE_LOCAL := ../cmxd/src
LIBCMX_LIB_LOCAL := ../cmxd

# Check for installed libcmx, otherwise use local
ifeq ($(wildcard $(LIBCMX_INCLUDE_INSTALLED)/cmxd-protocol.h),)
    # Use local development version
    CPPFLAGS += -I$(LIBCMX_INCLUDE_LOCAL)
    LDFLAGS += -L$(LIBCMX_LIB_LOCAL)
else
    # Use installed version
    CPPFLAGS += -I$(LIBCMX_INCLUDE_INSTALLED)
    LDFLAGS += -L$(LIBCMX_LIB_INSTALLED)
endif

LIBS = -lcmx

# Source files
SOURCES = $(PROGRAM_NAME).c
OBJECTS = $(SOURCES:.c=.o)

# Default target
all: $(PROGRAM_NAME)

# Build the main program
$(PROGRAM_NAME): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

# Compile source files
%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

# Installation
install: $(PROGRAM_NAME)
	install -d $(DESTDIR)$(BINDIR)
	install -d $(DESTDIR)$(MANDIR)/man8
	install -d $(DESTDIR)$(SYSTEMD_USER_DIR)
	install -d $(DESTDIR)$(EXAMPLEDIR)/scripts
	
	# Install main program
	install -m 755 $(PROGRAM_NAME) $(DESTDIR)$(BINDIR)/
	
	# Install example configuration and scripts
	install -m 644 tablet-mode.conf $(DESTDIR)$(EXAMPLEDIR)/daemon.conf.example
	
	# Install example scripts
	install -m 755 scripts/tablet-mode-on.sh $(DESTDIR)$(EXAMPLEDIR)/scripts/
	install -m 755 scripts/tablet-mode-off.sh $(DESTDIR)$(EXAMPLEDIR)/scripts/
	
	# Install systemd user service
	install -m 644 $(PROGRAM_NAME).service $(DESTDIR)$(SYSTEMD_USER_DIR)/
	
	# Install man page
	install -m 644 $(PROGRAM_NAME).8 $(DESTDIR)$(MANDIR)/man8/
	
	# Install documentation
	install -m 644 README.md $(DESTDIR)$(DOCDIR)/

# Uninstallation
uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(PROGRAM_NAME)
	rm -f $(DESTDIR)$(SYSTEMD_USER_DIR)/$(PROGRAM_NAME).service
	rm -f $(DESTDIR)$(MANDIR)/man8/$(PROGRAM_NAME).8
	rm -rf $(DESTDIR)$(DOCDIR)

# User installation (sets up user's tablet-mode directory)
install-user: $(PROGRAM_NAME)
	mkdir -p ~/.config/tablet-mode
	mkdir -p ~/.local/share/systemd/user
	
	# Install example config and scripts to user directory
	install -m 644 tablet-mode.conf ~/.config/tablet-mode/daemon.conf.example
	install -m 755 scripts/tablet-mode-on.sh ~/.config/tablet-mode/tablet-on.sh.example
	install -m 755 scripts/tablet-mode-off.sh ~/.config/tablet-mode/tablet-off.sh.example
	
	# Install user systemd service (uses system-installed binary)
	install -m 644 $(PROGRAM_NAME).service ~/.local/share/systemd/user/
	
	@echo "User setup complete. To configure:"
	@echo "  cp ~/.config/tablet-mode/daemon.conf.example ~/.config/tablet-mode/daemon.conf"
	@echo "  cp ~/.config/tablet-mode/tablet-on.sh.example ~/.config/tablet-mode/tablet-on.sh"
	@echo "  cp ~/.config/tablet-mode/tablet-off.sh.example ~/.config/tablet-mode/tablet-off.sh"
	@echo "  editor ~/.config/tablet-mode/daemon.conf"
	@echo "To enable the service:"
	@echo "  systemctl --user daemon-reload"
	@echo "  systemctl --user enable --now $(PROGRAM_NAME)"

# Uninstall user installation
uninstall-user:
	systemctl --user stop $(PROGRAM_NAME) || true
	systemctl --user disable $(PROGRAM_NAME) || true
	rm -f ~/.local/share/systemd/user/$(PROGRAM_NAME).service
	rm -f ~/.config/tablet-mode/daemon.conf.example
	rm -f ~/.config/tablet-mode/tablet-on.sh.example
	rm -f ~/.config/tablet-mode/tablet-off.sh.example
	rmdir ~/.config/tablet-mode 2>/dev/null || true
	systemctl --user daemon-reload

# Quick test build and run
test: $(PROGRAM_NAME)
	@echo "Testing $(PROGRAM_NAME) in foreground mode..."
	@echo "Press Ctrl+C to stop"
	./$(PROGRAM_NAME) -f -v

# Development targets
clean:
	rm -f $(OBJECTS) $(PROGRAM_NAME)

debug: CFLAGS += -g -DDEBUG
debug: $(PROGRAM_NAME)

# Check for common issues
check:
	@echo "Checking system requirements..."
	@command -v hyprctl >/dev/null 2>&1 || echo "WARNING: hyprctl not found - Hyprland integration may not work"
	@test -e /dev/input/event20 || echo "WARNING: Default tablet device /dev/input/event20 not found"
	@command -v onboard >/dev/null 2>&1 || echo "INFO: onboard virtual keyboard not found"
	@echo "System check complete"

# Show runtime information
info:
	@echo "Program: $(PROGRAM_NAME) $(VERSION)"
	@echo "Install prefix: $(PREFIX)"
	@echo "Binary: $(BINDIR)/$(PROGRAM_NAME)"
	@echo "User Config: ~/.config/tablet-mode/daemon.conf"
	@echo "User Service: $(SYSTEMD_USER_DIR)/$(PROGRAM_NAME).service"
	@echo "Man page: $(MANDIR)/man8/$(PROGRAM_NAME).8"
	@echo "Examples: $(EXAMPLEDIR)/"

# Package for distribution
dist: clean
	tar -czf $(PROGRAM_NAME)-$(VERSION).tar.gz \
		--transform 's,^,$(PROGRAM_NAME)-$(VERSION)/,' \
		*.c *.conf *.service *.8 *.md Makefile scripts/

.PHONY: all install uninstall install-user uninstall-user test clean debug check info dist