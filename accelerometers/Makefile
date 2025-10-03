# Makefile for MDA6655 Split Tool
#
# Copyright (c) 2025 Armando DiCianno <armando@noonshy.com>
# Licensed under GPL-2.0

# Tool information
TOOL_NAME := mda6655-split
VERSION := 1.0

# Installation paths
PREFIX ?= /usr
DESTDIR ?=
SBINDIR := $(PREFIX)/sbin
UDEVDIR := /etc/udev/rules.d
CONFDIR := /etc/default

# Source files
SCRIPT := $(TOOL_NAME)
UDEV_RULES := 99-mxc4005-second.rules
CONFIG_FILE := $(TOOL_NAME).conf

# Default target
all: $(SCRIPT)-processed $(UDEV_RULES)-processed

# Process script and udev rules with PREFIX substitution
%-processed: %
	@echo "Processing $< for PREFIX=$(PREFIX)..."
	sed 's|/usr/local/sbin|$(PREFIX)/sbin|g' $< > $@
	@if [ "$<" = "$(SCRIPT)" ]; then chmod +x $@; fi

# Install the tool and udev rules
install: $(SCRIPT)-processed $(UDEV_RULES)-processed
	@echo "Installing $(TOOL_NAME)..."
	install -d $(DESTDIR)$(SBINDIR)
	install -m 755 $(SCRIPT)-processed $(DESTDIR)$(SBINDIR)/$(TOOL_NAME)
	install -d $(DESTDIR)$(UDEVDIR)
	install -m 644 $(UDEV_RULES)-processed $(DESTDIR)$(UDEVDIR)/$(UDEV_RULES)
	install -d $(DESTDIR)$(CONFDIR)
	install -m 644 $(CONFIG_FILE) $(DESTDIR)$(CONFDIR)/$(CONFIG_FILE)
	@echo "Installation completed successfully"
	@echo "Note: Run 'sudo udevadm control --reload-rules' to activate udev rules"
	@echo "Note: Configuration file installed at $(CONFDIR)/$(CONFIG_FILE)"

# Uninstall the tool and udev rules
uninstall:
	@echo "Uninstalling $(TOOL_NAME)..."
	rm -f $(DESTDIR)$(SBINDIR)/$(TOOL_NAME)
	rm -f $(DESTDIR)$(UDEVDIR)/$(UDEV_RULES)
	rm -f $(DESTDIR)$(CONFDIR)/$(CONFIG_FILE)
	@echo "Note: Run 'sudo udevadm control --reload-rules' to deactivate udev rules"

# Clean build artifacts
clean:
	rm -f $(SCRIPT)-processed $(UDEV_RULES)-processed

# Test the installation
test: install
	@echo "Testing $(TOOL_NAME) installation..."
	@if [ -x "$(DESTDIR)$(SBINDIR)/$(TOOL_NAME)" ]; then \
		echo "✓ Script installed and executable"; \
	else \
		echo "✗ Script not found or not executable"; \
		exit 1; \
	fi
	@if [ -f "$(DESTDIR)$(UDEVDIR)/$(UDEV_RULES)" ]; then \
		echo "✓ Udev rules installed"; \
	else \
		echo "✗ Udev rules not found"; \
		exit 1; \
	fi

# Show program information
info:
	@echo "Tool: $(TOOL_NAME) v$(VERSION)"
	@echo "Script: $(SCRIPT)"
	@echo "Udev rules: $(UDEV_RULES)"
	@echo "Install prefix: $(PREFIX)"

# Reload udev rules (requires root)
reload-udev:
	@echo "Reloading udev rules..."
	udevadm control --reload-rules
	udevadm settle

# Show help
help:
	@echo "Available targets:"
	@echo "  all (default) - Process script and udev rules"
	@echo "  install       - Install script and udev rules"
	@echo "  uninstall     - Remove installed files"
	@echo "  clean         - Clean build artifacts"
	@echo "  test          - Test installation"
	@echo "  info          - Show tool information"
	@echo "  reload-udev   - Reload udev rules (requires root)"
	@echo "  help          - Show this help"
	@echo ""
	@echo "Variables:"
	@echo "  PREFIX        - Install prefix (default: /usr)"
	@echo "  DESTDIR       - Staging directory for packaging"

# Package for distribution
dist: clean
	@echo "Creating distribution package..."
	tar -czf $(TOOL_NAME)-$(VERSION).tar.gz \
		$(SCRIPT) $(UDEV_RULES) Makefile README.md

.PHONY: all install uninstall clean test info reload-udev help dist