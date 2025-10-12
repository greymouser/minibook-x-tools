CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -g -D_GNU_SOURCE
LDFLAGS = -lm
TARGET = iio_event_test
SOURCES = iio_event_test.c

.PHONY: all clean install

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES) $(LDFLAGS)

clean:
	rm -f $(TARGET) *.o

install: $(TARGET)
	install -m 755 $(TARGET) /usr/local/bin/

test: $(TARGET)
	@echo "Running IIO event test (requires root)..."
	@echo "Make sure to run as: sudo ./$(TARGET)"

help:
	@echo "Available targets:"
	@echo "  all     - Build the program"
	@echo "  clean   - Remove build artifacts"
	@echo "  install - Install to /usr/local/bin"
	@echo "  test    - Show how to run the test"
	@echo "  help    - Show this help"