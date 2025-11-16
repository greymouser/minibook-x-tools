# CHUWI Minibook X Session Daemon (cmxsd) Makefile
# Copyright (c) 2025 Armando DiCianno <armando@noonshy.com>
# Licensed under GPL-2.0

PROGRAM_NAME = cmxsd
VERSION := $(shell cat ../VERSION 2>/dev/null || echo "5.0.2")

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

LIBS = -lcmx

# Directory structure
SRC_DIR = src
SUPPORT_DIR = support

# Source files
SOURCES = $(SRC_DIR)/$(PROGRAM_NAME).c
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
	install -m 644 $(SUPPORT_DIR)/cmxsd.conf $(DESTDIR)$(EXAMPLEDIR)/daemon.conf.example
	
	# Install example scripts
	install -m 755 scripts/tablet-mode-on.sh $(DESTDIR)$(EXAMPLEDIR)/scripts/
	install -m 755 scripts/tablet-mode-off.sh $(DESTDIR)$(EXAMPLEDIR)/scripts/
	
	# Install systemd user service
	install -m 644 $(SUPPORT_DIR)/$(PROGRAM_NAME).service $(DESTDIR)$(SYSTEMD_USER_DIR)/
	
	# Install man page
	install -m 644 $(SUPPORT_DIR)/$(PROGRAM_NAME).8 $(DESTDIR)$(MANDIR)/man8/
	
	# Install documentation
	install -m 644 README.md $(DESTDIR)$(DOCDIR)/

# Uninstallation
uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(PROGRAM_NAME)
	rm -f $(DESTDIR)$(SYSTEMD_USER_DIR)/$(PROGRAM_NAME).service
	rm -f $(DESTDIR)$(MANDIR)/man8/$(PROGRAM_NAME).8
	rm -rf $(DESTDIR)$(DOCDIR)

# User installation (sets up user's cmxsd directory)
install-user: $(PROGRAM_NAME)
	mkdir -p ~/.config/cmxsd
	mkdir -p ~/.local/share/systemd/user
	
	# Install example config and scripts to user directory
	install -m 644 $(SUPPORT_DIR)/cmxsd.conf ~/.config/cmxsd/daemon.conf.example
	install -m 755 scripts/tablet-on.sh ~/.config/cmxsd/tablet-mode-on.sh.example
	install -m 755 scripts/tablet-off.sh ~/.config/cmxsd/tablet-mode-off.sh.example
	
	# Install user systemd service (uses system-installed binary)
	install -m 644 $(SUPPORT_DIR)/$(PROGRAM_NAME).service ~/.local/share/systemd/user/
	
	@echo "User setup complete. To configure:"
	@echo "  cp ~/.config/cmxsd/daemon.conf.example ~/.config/cmxsd/daemon.conf"
	@echo "  cp ~/.config/cmxsd/tablet-mode-on.sh.example ~/.config/cmxsd/tablet-mode-on.sh"
	@echo "  cp ~/.config/cmxsd/tablet-mode-off.sh.example ~/.config/cmxsd/tablet-mode-off.sh"
	@echo "  editor ~/.config/cmxsd/daemon.conf"
	@echo "To enable the service:"
	@echo "  systemctl --user daemon-reload"
	@echo "  systemctl --user enable --now $(PROGRAM_NAME)"

# Uninstall user installation
uninstall-user:
	systemctl --user stop $(PROGRAM_NAME) || true
	systemctl --user disable $(PROGRAM_NAME) || true
	rm -f ~/.local/share/systemd/user/$(PROGRAM_NAME).service
	rm -f ~/.config/cmxsd/daemon.conf.example
	rm -f ~/.config/cmxsd/tablet-mode-on.sh.example
	rm -f ~/.config/cmxsd/tablet-mode-off.sh.example
	rmdir ~/.config/cmxsd 2>/dev/null || true
	systemctl --user daemon-reload

# Quick test build and run
test: $(PROGRAM_NAME)
	@echo "Testing $(PROGRAM_NAME) in foreground mode..."
	@echo "Press Ctrl+C to stop"
	./$(PROGRAM_NAME) -f -v

# Development targets
clean:
	rm -f $(OBJECTS) $(PROGRAM_NAME)
	rm -f $(SRC_DIR)/*.o

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
	@echo "User Config: ~/.config/cmxsd/daemon.conf"
	@echo "User Service: $(SYSTEMD_USER_DIR)/$(PROGRAM_NAME).service"
	@echo "Man page: $(MANDIR)/man8/$(PROGRAM_NAME).8"
	@echo "Examples: $(EXAMPLEDIR)/"

# Package for distribution
dist: clean
	tar -czf $(PROGRAM_NAME)-$(VERSION).tar.gz \
		--transform 's,^,$(PROGRAM_NAME)-$(VERSION)/,' \
		$(SRC_DIR)/ $(SUPPORT_DIR)/ scripts/ *.md Makefile

.PHONY: all install uninstall install-user uninstall-user test clean debug check info dist