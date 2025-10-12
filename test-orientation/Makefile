# Makefile for test_orientation utility
# Simple build system for orientation testing tool

CC = gcc
CFLAGS = -std=gnu11 -Wall -Wextra -O2 -g
LDFLAGS = -lm
TARGET = test_orientation
SOURCE = test_orientation.c

# Default target
all: $(TARGET)

# Build the test program
$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE) $(LDFLAGS)

# Clean build artifacts
clean:
	rm -f $(TARGET)

# Run the test program (requires sudo)
run: $(TARGET)
	@echo "Running orientation test (requires sudo)..."
	@echo "Rotate your laptop to test different orientations"
	sudo ./$(TARGET)

# Phony targets
.PHONY: all clean run help

# Help target
help:
	@echo "Available targets:"
	@echo "  all     - Build the test_orientation program (default)"
	@echo "  clean   - Remove build artifacts"
	@echo "  run     - Build and run the test program (requires sudo)"
	@echo "  help    - Show this help message"
	@echo ""
	@echo "Usage:"
	@echo "  make        # Build the program"
	@echo "  make run    # Build and run with sudo"
	@echo "  make clean  # Clean up"