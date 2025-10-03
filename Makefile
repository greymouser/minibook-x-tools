# Master Makefile for Chuwi Minibook X Tools
#
# This Makefile coordinates builds across all components:
# - accelerometers/     - MDA6655 Split Tool for missing accelerometer devices
# - module-v1/module/   - Kernel module for tablet mode detection
# - module-v1/module-userspace/ - Userspace daemon for tablet mode
#
# Copyright (c) 2025 Armando DiCianno <armando@noonshy.com>
# Licensed under GPL-2.0

# Project information
PROJECT_NAME := minibook-x-tools
VERSION := 1.0

# Build configuration
DEBUGFS ?= false

# Component directories
ACCELEROMETERS_DIR := accelerometers
MODULE_DIR := module-v1/module
USERSPACE_DIR := module-v1/module-userspace
TABLET_INTEGRATION_DIR := tablet-mode-integration

# Components that support each target
ALL_COMPONENTS := $(ACCELEROMETERS_DIR) $(MODULE_DIR) $(USERSPACE_DIR) $(TABLET_INTEGRATION_DIR)
INSTALL_COMPONENTS := $(ACCELEROMETERS_DIR) $(MODULE_DIR) $(USERSPACE_DIR) $(TABLET_INTEGRATION_DIR)
CLEAN_COMPONENTS := $(ACCELEROMETERS_DIR) $(MODULE_DIR) $(USERSPACE_DIR) $(TABLET_INTEGRATION_DIR)

# Default target - build all components
all: build-accelerometers build-module build-userspace build-tablet-integration
	@echo "=== All components built successfully ==="

# Individual component build targets
build-accelerometers:
	@echo "=== Building accelerometers tools ==="
	@$(MAKE) -C $(ACCELEROMETERS_DIR) all || { echo "ERROR: Accelerometers build failed"; exit 1; }

build-module:
	@echo "=== Building kernel module ==="
ifeq ($(DEBUGFS),true)
	@$(MAKE) -C $(MODULE_DIR) debugfs || { echo "ERROR: Kernel module build failed"; exit 1; }
else
	@$(MAKE) -C $(MODULE_DIR) all || { echo "ERROR: Kernel module build failed"; exit 1; }
endif

build-userspace:
	@echo "=== Building userspace daemon ==="
	@$(MAKE) -C $(USERSPACE_DIR) all || { echo "ERROR: Userspace daemon build failed"; exit 1; }

build-tablet-integration:
	@echo "=== Building tablet mode integration ==="
	@$(MAKE) -C $(TABLET_INTEGRATION_DIR) all || { echo "ERROR: Tablet integration build failed"; exit 1; }

# Install all components
install: install-accelerometers install-module install-userspace install-tablet-integration
	@echo "=== All components installed successfully ==="

install-accelerometers:
	@echo "=== Installing accelerometers tools ==="
	@$(MAKE) -C $(ACCELEROMETERS_DIR) install || { echo "ERROR: Accelerometers installation failed"; exit 1; }

install-module:
	@echo "=== Installing kernel module ==="
	@$(MAKE) -C $(MODULE_DIR) install || { echo "ERROR: Kernel module installation failed"; exit 1; }

install-userspace:
	@echo "=== Installing userspace daemon ==="
	@$(MAKE) -C $(USERSPACE_DIR) install || { echo "ERROR: Userspace daemon installation failed"; exit 1; }

install-tablet-integration:
	@echo "=== Installing tablet mode integration ==="
	@$(MAKE) -C $(TABLET_INTEGRATION_DIR) install || { echo "ERROR: Tablet integration installation failed"; exit 1; }

# Uninstall all components
uninstall: uninstall-accelerometers uninstall-module uninstall-userspace uninstall-tablet-integration
	@echo "=== All components uninstalled successfully ==="

uninstall-accelerometers:
	@echo "=== Uninstalling accelerometers tools ==="
	@$(MAKE) -C $(ACCELEROMETERS_DIR) uninstall || { echo "ERROR: Accelerometers uninstallation failed"; exit 1; }

uninstall-module:
	@echo "=== Uninstalling kernel module ==="
	@$(MAKE) -C $(MODULE_DIR) uninstall || { echo "ERROR: Kernel module uninstallation failed"; exit 1; }

uninstall-userspace:
	@echo "=== Uninstalling userspace daemon ==="
	@$(MAKE) -C $(USERSPACE_DIR) uninstall || { echo "ERROR: Userspace daemon uninstallation failed"; exit 1; }

uninstall-tablet-integration:
	@echo "=== Uninstalling tablet mode integration ==="
	@$(MAKE) -C $(TABLET_INTEGRATION_DIR) uninstall || { echo "ERROR: Tablet integration uninstallation failed"; exit 1; }

# Clean all components
clean: clean-accelerometers clean-module clean-userspace clean-tablet-integration
	@echo "=== All components cleaned successfully ==="

clean-accelerometers:
	@echo "=== Cleaning accelerometers tools ==="
	@$(MAKE) -C $(ACCELEROMETERS_DIR) clean || { echo "ERROR: Accelerometers clean failed"; exit 1; }

clean-module:
	@echo "=== Cleaning kernel module ==="
	@$(MAKE) -C $(MODULE_DIR) clean || { echo "ERROR: Kernel module clean failed"; exit 1; }

clean-userspace:
	@echo "=== Cleaning userspace daemon ==="
	@$(MAKE) -C $(USERSPACE_DIR) clean || { echo "ERROR: Userspace daemon clean failed"; exit 1; }

clean-tablet-integration:
	@echo "=== Cleaning tablet mode integration ==="
	@$(MAKE) -C $(TABLET_INTEGRATION_DIR) clean || { echo "ERROR: Tablet integration clean failed"; exit 1; }

# Debug builds (only affects userspace component)
debug: build-accelerometers build-module debug-userspace
	@echo "=== All components built successfully (debug mode) ==="

debug-userspace:
	@echo "=== Building userspace daemon (debug) ==="
	@$(MAKE) -C $(USERSPACE_DIR) debug || { echo "ERROR: Userspace daemon debug build failed"; exit 1; }

# Test targets
test: test-accelerometers test-userspace
	@echo "=== All component tests completed successfully ==="

test-accelerometers:
	@echo "=== Testing accelerometers tools ==="
	@$(MAKE) -C $(ACCELEROMETERS_DIR) test || { echo "ERROR: Accelerometers tests failed"; exit 1; }

test-userspace:
	@echo "=== Testing userspace daemon ==="
	@$(MAKE) -C $(USERSPACE_DIR) test || { echo "ERROR: Userspace daemon tests failed"; exit 1; }

# Information and help
info:
	@echo "=== Project Information ==="
	@echo "Project: $(PROJECT_NAME) v$(VERSION)"
	@echo "Components:"
	@echo "  - accelerometers/          MDA6655 Split Tool"
	@echo "  - module-v1/module/        Kernel module"
	@echo "  - module-v1/module-userspace/  Userspace daemon"
	@echo ""
	@echo "Configuration:"
	@echo "  DEBUGFS=$(DEBUGFS)     Enable debugfs in kernel module"
	@echo ""
	@echo "Component Information:"
	@echo ""
	@echo "--- Accelerometers ---"
	@$(MAKE) -C $(ACCELEROMETERS_DIR) info
	@echo ""
	@echo "--- Kernel Module ---"
	@$(MAKE) -C $(MODULE_DIR) info
	@echo ""
	@echo "--- Userspace Daemon ---"
	@$(MAKE) -C $(USERSPACE_DIR) info

help:
	@echo "Chuwi Minibook X Tools - Master Build System"
	@echo ""
	@echo "Usage: make [DEBUGFS=true] [target]"
	@echo ""
	@echo "Main targets:"
	@echo "  all          Build all components (default)"
	@echo "  install      Install all components"
	@echo "  uninstall    Uninstall all components"
	@echo "  clean        Clean all build artifacts"
	@echo "  debug        Build with debug options"
	@echo "  test         Run tests for applicable components"
	@echo "  info         Show detailed project information"
	@echo "  help         Show this help message"
	@echo ""
	@echo "Component-specific targets:"
	@echo "  build-accelerometers    Build only accelerometers tools"
	@echo "  build-module           Build only kernel module"
	@echo "  build-userspace        Build only userspace daemon"
	@echo "  install-accelerometers Install only accelerometers tools"
	@echo "  install-module         Install only kernel module"
	@echo "  install-userspace      Install only userspace daemon"
	@echo "  clean-accelerometers   Clean only accelerometers tools"
	@echo "  clean-module           Clean only kernel module"
	@echo "  clean-userspace        Clean only userspace daemon"
	@echo ""
	@echo "Configuration options:"
	@echo "  DEBUGFS=true           Enable debugfs support in kernel module"
	@echo ""
	@echo "Examples:"
	@echo "  make                   Build all components"
	@echo "  make DEBUGFS=true all  Build all with debugfs enabled"
	@echo "  make install           Install everything"
	@echo "  make clean             Clean all build artifacts"
	@echo ""
	@echo "For component-specific help:"
	@echo "  make -C accelerometers help"
	@echo "  make -C module-v1/module help"
	@echo "  make -C module-v1/module-userspace help"

# Module-specific convenience targets
load-module:
	@echo "=== Loading kernel module ==="
	$(MAKE) -C $(MODULE_DIR) load

unload-module:
	@echo "=== Unloading kernel module ==="
	$(MAKE) -C $(MODULE_DIR) unload

reload-module:
	@echo "=== Reloading kernel module ==="
	$(MAKE) -C $(MODULE_DIR) reload

# Userspace daemon convenience targets
run-userspace:
	@echo "=== Running userspace daemon ==="
	$(MAKE) -C $(USERSPACE_DIR) run

# Development targets
check:
	@echo "=== Running static analysis ==="
	$(MAKE) -C $(USERSPACE_DIR) check

analyze:
	@echo "=== Running analyzer ==="
	$(MAKE) -C $(USERSPACE_DIR) analyze

# Distribution
dist: clean
	@echo "=== Creating distribution package ==="
	tar czf $(PROJECT_NAME)-$(VERSION).tar.gz \
		--exclude='.git*' \
		--exclude='*.tar.gz' \
		--exclude='*-processed' \
		.

.PHONY: all install uninstall clean debug test info help \
        build-accelerometers build-module build-userspace \
        install-accelerometers install-module install-userspace \
        uninstall-accelerometers uninstall-module uninstall-userspace \
        clean-accelerometers clean-module clean-userspace \
        debug-userspace test-accelerometers test-userspace \
        load-module unload-module reload-module run-userspace \
        check analyze dist