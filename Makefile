# Notes - Obsidian-like Notebook
# Cross-platform Makefile for macOS and Linux

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
TARGET = notes
SRC = obsidian_notebook.c

# Detect platform
UNAME := $(shell uname)

ifeq ($(UNAME), Darwin)
    # macOS
    INCLUDES = -I/opt/homebrew/include -I/usr/local/include
    LIBS = -L/opt/homebrew/lib -L/usr/local/lib
    FRAMEWORKS = -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
    LDFLAGS = $(LIBS) -lraylib $(FRAMEWORKS)
else
    # Linux
    INCLUDES =
    LIBS =
    LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
endif

# Build targets
.PHONY: all clean run install uninstall

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(TARGET) $(SRC) $(LDFLAGS)
	@echo "✅ Build complete: ./$(TARGET)"

clean:
	rm -f $(TARGET)
	@echo "🧹 Cleaned build files"

run: $(TARGET)
	./$(TARGET)

# Install to /usr/local/bin (requires sudo on Linux)
install: $(TARGET)
	cp $(TARGET) /usr/local/bin/
	@echo "📦 Installed to /usr/local/bin/$(TARGET)"

uninstall:
	rm -f /usr/local/bin/$(TARGET)
	@echo "🗑️  Uninstalled $(TARGET)"

# Development helpers
debug: CFLAGS += -g -O0
debug: $(TARGET)

release: CFLAGS += -O3
release: $(TARGET)

# Show help
help:
	@echo "Notes - Makefile Commands"
	@echo "━━━━━━━━━━━━━━━━━━━━━━━━━━"
	@echo "  make          Build the application"
	@echo "  make run      Build and run"
	@echo "  make clean    Remove build files"
	@echo "  make install  Install to /usr/local/bin"
	@echo "  make debug    Build with debug symbols"
	@echo "  make release  Build optimized release"
