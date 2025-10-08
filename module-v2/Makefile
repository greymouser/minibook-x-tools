#
# Makefile for Chuwi Minibook X platform driver
#
# This Makefile supports both:
# - In-tree kernel builds (when CONFIG_CHUWI_MINIBOOK_X is defined)
# - Out-of-tree development builds (make all/clean/install targets)
#

# Module and object definitions (shared by both build modes)
obj-m += chuwi-minibook-x.o

# Core module object files (always include sysfs for out-of-tree)
chuwi-minibook-x-y := chuwi-minibook-x-main.o orientation-detection.o chuwi-minibook-x-sysfs.o

# Detect if we're being called from kernel build system or standalone
ifeq ($(KERNELRELEASE),)
    # Out-of-tree build - standalone development mode
    KERNELDIR ?= /lib/modules/$(shell uname -r)/build
    PWD := $(shell pwd)
    
    # Add debugfs support (enabled by default for out-of-tree builds)
    chuwi-minibook-x-y += chuwi-minibook-x-debugfs.o
    
    # Development compiler flags
    ccflags-y += -Wall -Wextra -DCONFIG_CHUWI_MINIBOOK_X_DEBUGFS

    # Out-of-tree build targets
    all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

    clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean

install: all
	@echo "Installing Chuwi Minibook X module..."
	@echo "Loading required dependencies..."
	-sudo modprobe mxc4005
	-sudo modprobe serial_multi_instantiate  
	@echo "Installing main module..."
	sudo insmod chuwi-minibook-x.ko
	@echo "✓ Module installed successfully!"
	@echo "Check status with: lsmod | grep chuwi"

load: all
	@echo "Loading Chuwi Minibook X module with modprobe..."
	@echo "Copying fresh module to system path..."
	sudo mkdir -p /lib/modules/$(shell uname -r)/extra/
	sudo cp chuwi-minibook-x.ko /lib/modules/$(shell uname -r)/extra/
	sudo depmod -a
	@echo "Loading dependencies..."
	-sudo modprobe mxc4005
	-sudo modprobe serial_multi_instantiate
	@echo "Loading main module..."
	sudo modprobe chuwi_minibook_x
	@echo "✓ Module loaded with dependencies!"
	@echo "Status: $$(lsmod | grep chuwi_minibook_x || echo 'not loaded')"
	@echo "Dependencies: $$(lsmod | grep -E '(mxc4005|serial_multi)' | wc -l) modules"

unload:
	@echo "Unloading Chuwi Minibook X module and dependencies..."
	-sudo modprobe -r chuwi_minibook_x
	-sudo modprobe -r mxc4005
	@if [ -f /lib/modules/$(shell uname -r)/extra/chuwi-minibook-x.ko ]; then \
		echo "Removing temporary module from system path..."; \
		sudo rm -f /lib/modules/$(shell uname -r)/extra/chuwi-minibook-x.ko; \
		sudo depmod -a; \
	fi
	@echo "✓ Module and dependencies unloaded!"
	@echo "Status: $$(lsmod | grep -E '(chuwi_minibook_x|mxc4005)' | wc -l) modules remain"

modules_install: install

help:
	@echo "Chuwi Minibook X Driver Build System"
	@echo ""
	@echo "Out-of-tree development targets:"
	@echo "  all              - Build the module"
	@echo "  clean            - Clean build artifacts"
	@echo "  install          - Install module to system (with dependencies)"
	@echo "  load             - Load module with modprobe (handles dependencies)"
	@echo "  unload           - Unload module and dependencies with modprobe -r"
	@echo "  modules_install  - Alias for install"
	@echo "  help             - Show this help"
	@echo ""
	@echo "Dependencies: This module requires mxc4005 and serial_multi_instantiate"
	@echo "Note: This module is designed for eventual in-tree integration"
	@echo "      with proper CONFIG_CHUWI_MINIBOOK_X support."

.PHONY: all clean install load unload modules_install help

else
    # In-tree kernel build - production mode
    obj-$(CONFIG_CHUWI_MINIBOOK_X) += chuwi-minibook-x.o

    # Core module object files
    chuwi-minibook-x-objs := chuwi-minibook-x-main.o orientation-detection.o chuwi-minibook-x-sysfs.o

    # Add debugging support if enabled via kernel config
    ifdef CONFIG_CHUWI_MINIBOOK_X_DEBUGFS
    chuwi-minibook-x-objs += chuwi-minibook-x-debugfs.o
    ccflags-y += -DCONFIG_CHUWI_MINIBOOK_X_DEBUGFS
    endif

    # Production compiler flags
    ccflags-y += -Wall

endif