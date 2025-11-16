# Simple Makefile for cmxd tests
# Builds test executables from .c files that contain main() functions

# Build configuration
CC ?= gcc
CFLAGS := -std=gnu11 -Wall -Wextra -O2 -g
CPPFLAGS := -D_GNU_SOURCE -I../src
LDFLAGS := 
LIBS := -lm -lpthread

# Test programs with main() functions
TEST_TARGETS := analyze-logs

# Default target - build all tests
all: $(TEST_TARGETS)

# Build analyze-logs with calculation dependencies
analyze-logs: analyze-logs.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $< ../src/cmxd-calculations.o $(LIBS)

# Clean all test executables
clean:
	rm -f $(TEST_TARGETS)

# Show help
help:
	@echo "Available targets:"
	@echo "  all           - Build all test executables (default)"
	@echo "  clean         - Remove all test executables"
	@echo "  help          - Show this help"
	@echo ""
	@echo "Test executables:"
	@echo "$(TEST_TARGETS)"

# Show what would be built
list:
	@echo "Test targets: $(TEST_TARGETS)"

.PHONY: all clean help list