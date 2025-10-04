# Makefile for chuwi-minibook-x-tablet-mode kernel module
#
# Copyright (c) 2025 Armando DiCianno <armando@noonshy.com>
# Licensed under GPL-2.0

# Module name and version
MODULE_NAME := chuwi-minibook-x-tablet-mode
MODULE_VERSION := 1.0

# Kernel build directory
KDIR ?= /lib/modules/$(shell uname -r)/build

# Installation directories
DESTDIR ?=
INSTALL_MOD_PATH ?= $(DESTDIR)
INSTALL_DIR := $(INSTALL_MOD_PATH)/lib/modules/$(shell uname -r)/extra

# Configuration options (can be overridden)
CONFIG_CHUWI_MINIBOOK_X_TABLET_MODE_DEBUGFS ?= n

# Conditional compilation flags
ifeq ($(CONFIG_CHUWI_MINIBOOK_X_TABLET_MODE_DEBUGFS),y)
    ccflags-y += -DCONFIG_CHUWI_MINIBOOK_X_TABLET_MODE_DEBUGFS
endif

# Source files
obj-m := $(MODULE_NAME).o
chuwi-minibook-x-tablet-mode-objs := tablet_mode_main.o orientation_detection.o

# Default target
all: modules

# Build the module
modules:
	$(MAKE) -C $(KDIR) M=$(shell pwd) modules

# Build with debugfs support
debugfs:
	$(MAKE) CONFIG_CHUWI_MINIBOOK_X_TABLET_MODE_DEBUGFS=y modules

# Build without debugfs support (explicit)
no-debugfs:
	$(MAKE) CONFIG_CHUWI_MINIBOOK_X_TABLET_MODE_DEBUGFS=n modules

# Install the module
install: modules
	@echo "Installing $(MODULE_NAME) module..."
	install -d $(INSTALL_DIR)
	install -m 644 $(MODULE_NAME).ko $(INSTALL_DIR)/
	@if [ -z "$(DESTDIR)" ]; then \
		/sbin/depmod -a; \
	else \
		echo "Note: Run '/sbin/depmod -a' after installation to update module dependencies"; \
	fi

# Uninstall the module
uninstall:
	@echo "Uninstalling $(MODULE_NAME) module..."
	rm -f $(INSTALL_DIR)/$(MODULE_NAME).ko
	/sbin/depmod -a

# Clean build artifacts
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f Module.markers modules.order

# Load the module
load:
	@echo "Loading $(MODULE_NAME) module..."
	insmod $(MODULE_NAME).ko

# Unload the module
unload:
	@echo "Unloading $(MODULE_NAME) module..."
	rmmod $(MODULE_NAME)

# Reload the module (unload + load)
reload: unload load

# Show module information
info:
	@if [ -f $(MODULE_NAME).ko ]; then \
		echo "Module information:"; \
		modinfo $(MODULE_NAME).ko; \
	else \
		echo "Module not built. Run 'make' first."; \
	fi

# Help target
help:
	@echo "Available targets:"
	@echo "  all (default) - Build the module"
	@echo "  modules       - Build the module"
	@echo "  debugfs       - Build with debugfs support enabled"
	@echo "  no-debugfs    - Build with debugfs support disabled"
	@echo "  install       - Install the module (use DESTDIR= for staging)"
	@echo "  uninstall     - Uninstall the module"
	@echo "  clean         - Clean build artifacts"
	@echo "  load          - Load the module into kernel"
	@echo "  unload        - Unload the module from kernel"
	@echo "  reload        - Reload the module (unload + load)"
	@echo "  info          - Show module information"
	@echo "  help          - Show this help"
	@echo ""
	@echo "Configuration:"
	@echo "  CONFIG_CHUWI_MINIBOOK_X_TABLET_MODE_DEBUGFS=y  - Enable debugfs support"
	@echo "  CONFIG_CHUWI_MINIBOOK_X_TABLET_MODE_DEBUGFS=n  - Disable debugfs support (default)"
	@echo ""
	@echo "Variables:"
	@echo "  KDIR          - Kernel build directory (default: /lib/modules/\$$(uname -r)/build)"
	@echo "  DESTDIR       - Staging directory for installation"
	@echo "  INSTALL_MOD_PATH - Module installation prefix"

.PHONY: all modules install uninstall clean load unload reload info help
