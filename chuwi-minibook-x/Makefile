#
# Makefile for Chuwi Minibook X platform driver
#
# This Makefile builds the unified driver that combines mount matrix support
# with complete tablet mode detection functionality.
#

# Module definition - single unified module
obj-m += chuwi-minibook-x.o

# Inject version from VERSION file
VERSION_STRING := $(shell cat VERSION 2>/dev/null || echo "3.0")
ccflags-y += -DMODULE_VERSION_STRING=\"$(VERSION_STRING)\"

# The module is a single source file

# Detect if we're being called from kernel build system or standalone
ifeq ($(KERNELRELEASE),)
    # Out-of-tree build - standalone development mode
    KERNELDIR ?= /lib/modules/$(shell uname -r)/build
    PWD := $(shell pwd)
    
    # Development compiler flags
    ccflags-y += -Wall -Wextra

    # Out-of-tree build targets
    all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

    clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	@# VERSION file is kept during clean

    install: all
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install
	depmod -a

    # Test targets
    load: all
	@echo "Loading chuwi-minibook-x module (mxc4005 will auto-load via MODULE_SOFTDEP)"
	sudo insmod chuwi-minibook-x.ko

    unload:
	@echo "Unloading modules in proper order..."
	sudo rmmod chuwi-minibook-x || true
	sudo rmmod mxc4005 || true
	@echo "Modules unloaded"

    reload: unload load

    test: reload
	@echo "=== Testing hardware detection ==="
	@sleep 2
	@echo "Checking dmesg for driver messages:"
	@dmesg | tail -20 | grep -i "chuwi\|minibook" || echo "No driver messages found"
	@echo "Checking sysfs interface:"
	@if [ -d /sys/devices/platform/chuwi-minibook-x ]; then \
		echo "Platform driver loaded successfully"; \
		find /sys/devices/platform/chuwi-minibook-x -name "*" -type f 2>/dev/null | head -10; \
	else \
		echo "Platform driver not found in sysfs"; \
	fi

    # Development helpers
    dmesg:
	dmesg | tail -30 | grep -i "chuwi\|minibook\|MDA6655\|mxc" || echo "No relevant messages"

else
    # In-tree build (if ever used)
    # Module is a single file, no additional objects needed
endif

# Build targets help
help:
	@echo "Chuwi Minibook X Driver"
	@echo "Available targets:"
	@echo "  all     - Build the driver"
	@echo "  clean   - Clean build artifacts"
	@echo "  load    - Load the driver module"
	@echo "  unload  - Unload the driver module"
	@echo "  reload  - Unload and reload the driver"
	@echo "  test    - Load driver and run basic tests"
	@echo "  dmesg   - Show recent kernel messages"

.PHONY: all clean install load unload reload test dmesg help