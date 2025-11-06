#
# Makefile for CMX - Chuwi Minibook X platform driver
#
# This Makefile provides development and installation commands.
# The actual kernel build configuration is in Kbuild.
#

# Kernel build integration (from kernel documentation pattern)
KDIR ?= /lib/modules/$(shell uname -r)/build
export KBUILD_EXTMOD := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))

include $(KDIR)/Makefile
