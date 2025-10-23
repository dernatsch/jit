# Compiler and flags
CC := cc
CFLAGS := -O2 -Wall -Wextra -std=c99
LDFLAGS := 

# Targets
TARGET := bfjit
OBJECTS := bfjit.o stencils.o
HEADERS := jit.h

# Default target
all: $(TARGET)

# Main executable
$(TARGET): bfjit.o
	$(CC) $(LDFLAGS) $^ -o $@

# Object files
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Stencils disassembly (using existing stencils.o)
stencils-disasm: stencils.o
	objdump -d stencils.o

# Debug build
debug: CFLAGS += -g -DDEBUG
debug: $(TARGET)

# Release build with additional optimizations
release: CFLAGS += -O3 -DNDEBUG
release: $(TARGET)

# Clean build artifacts
clean:
	$(RM) $(TARGET) $(OBJECTS)

# Phony targets
.PHONY: all clean debug release stencils-disasm

# Help target
help:
	@echo "Available targets:"
	@echo "  all              - Build everything (default)"
	@echo "  debug            - Build with debug symbols"
	@echo "  release          - Build with maximum optimizations"
	@echo "  stencils-disasm  - Show disassembly of stencils.o"
	@echo "  clean            - Remove build artifacts"
	@echo "  install          - Install to /usr/local/bin/"
	@echo "  help             - Show this help message"
