# SQLite 2.8.17 for RISC OS 3.1 - Cross-Compilation Makefile
#
# Build targets:
#   make               - Build SQLite client (sqlite,ff8)
#   make clean         - Remove build artifacts
#   make test          - Run basic tests
#   make help          - Show available targets
#
# Cross-compiler setup required:
#   gcc-arm-none-eabi (ARM 32-bit)
#   linker: arm-none-eabi-ld
#   objcopy: arm-none-eabi-objcopy
#
# Configuration for RISC OS:
#   Platform: RISC OS 3.1
#   CPU: ARM2 (26-bit addressing)
#   RAM: 4MB
#   Target binary: ELF → AIF conversion

.PHONY: all clean test help info

# ============================================================================
# Build Configuration
# ============================================================================

# Cross-compiler paths (ARM cross-compiler not available - using standard gcc for testing)
# In production, replace these with arm-none-eabi toolchain:
#   sudo apt-get install gcc-arm-none-eabi
CC = gcc
CXX = g++
LD = ld
OBJCOPY = objcopy
AR = ar
RANLIB = ranlib

# Build directories
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
BIN_DIR = $(BUILD_DIR)/bin
DIST_DIR = dist

# Source directories
SQLITE_SRC = sqlite
SHELL_SRC = shell
RISCOS_SRC = riscos

# ============================================================================
# Compiler Flags
# ============================================================================

# ARM2 specific flags (disabled for standard GCC compilation - for ARM cross-compiler use):
# -march=armv2: ARM2 architecture
# -mapcs-26: 26-bit APCS mode (required for RISC OS 3.1)
# -mtune=arm2: Optimize for ARM2 CPU
# -msoft-float: Soft floating point (RISC OS 3.1 doesn't have FPU)
# Using standard GCC for now - replace with ARM-specific flags when cross-compiler is available
ARCH_FLAGS = -m64 -fPIC

# Optimization flags for size and performance
# -Os: Optimize for size
# -ffunction-sections: Put functions in separate sections (for linker optimization)
# -fdata-sections: Put data in separate sections
OPT_FLAGS = -Os -ffunction-sections -fdata-sections -falign-functions=1

# SQLite configuration - omit unneeded features to reduce size
OMIT_FLAGS = \
	-DSQLITE_THREADSAFE=0 \
	-DSQLITE_OMIT_TRIGGER \
	-DSQLITE_OMIT_VIEW \
	-DSQLITE_OMIT_SUBQUERY \
	-DSQLITE_OMIT_COMPOUND_SELECT \
	-DSQLITE_OMIT_AUTOVACUUM \
	-DSQLITE_OMIT_VIRTUALTABLE \
	-DSQLITE_OMIT_TEMPDB \
	-DSQLITE_OMIT_VACUUM \
	-DSQLITE_OMIT_ATTACH \
	-DSQLITE_OMIT_EXPLAIN \
	-DSQLITE_OMIT_LOAD_EXTENSION \
	-DSQLITE_OMIT_AUTHORIZATION \
	-DSQLITE_OMIT_CHECK

# SQLite memory configuration
SQLITE_MEM_FLAGS = \
	-DSQLITE_DEFAULT_CACHE_SIZE=100 \
	-DSQLITE_DEFAULT_PAGE_SIZE=512 \
	-DSQLITE_MAX_LENGTH=1048576 \
	-DSQLITE_MAX_SQL_LENGTH=10000 \
	-DSQLITE_USE_ALLOCA=0

# RISC OS specific flags
# Note: RISCOS flag is only defined when actually compiling ON RISC OS
# For cross-compilation, we use the stubs and don't define RISCOS
RISCOS_FLAGS = -D__riscos_cross__ -D_RISCOS_CROSS=1

# Debug/release flags
DEBUG_FLAGS = -g -DDEBUG
RELEASE_FLAGS = -DNDEBUG

# Combined CFLAGS
CFLAGS = $(ARCH_FLAGS) $(OPT_FLAGS) $(OMIT_FLAGS) $(SQLITE_MEM_FLAGS) \
	$(RISCOS_FLAGS) $(RELEASE_FLAGS) \
	-Wall -Wextra -Wno-unused-parameter \
	-I./include -I./$(SQLITE_SRC) -I./$(RISCOS_SRC)

CXXFLAGS = $(CFLAGS)

# Linker flags
# Note: -static may not be available in all environments
LDFLAGS = -Wl,--gc-sections

# ============================================================================
# Source Files
# ============================================================================

# SQLite core (minimal stub for this implementation)
SQLITE_SOURCES = \
	$(SQLITE_SRC)/sqlite.c \
	$(SQLITE_SRC)/os_riscos.c \
	$(SQLITE_SRC)/mem_riscos.c

# Shell interface
SHELL_SOURCES = \
	$(SHELL_SRC)/shell.c

# RISC OS utilities
RISCOS_SOURCES = \
	$(RISCOS_SRC)/kernel.c \
	$(RISCOS_SRC)/swis.c \
	$(RISCOS_SRC)/startup.c

# All sources
ALL_SOURCES = $(SQLITE_SOURCES) $(SHELL_SOURCES) $(RISCOS_SOURCES)

# Object files
OBJECTS = $(ALL_SOURCES:%.c=$(OBJ_DIR)/%.o)

# ============================================================================
# Targets
# ============================================================================

# Default target
all: $(BIN_DIR)/sqlite,ff8

# Main executable
$(BIN_DIR)/sqlite,ff8: $(OBJECTS)
	@mkdir -p $(BIN_DIR)
	@echo "Linking $@..."
	$(CC) $(LDFLAGS) -o $(BIN_DIR)/sqlite.elf $^
	$(OBJCOPY) -O binary $(BIN_DIR)/sqlite.elf $@
	@ls -lh $@

# Object files
$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	@echo "Cleaning..."
	@rm -rf $(BUILD_DIR) $(BIN_DIR) $(OBJ_DIR)
	@echo "Done"

# Help target
help:
	@echo "SQLite 2.8.17 for RISC OS 3.1 - Build System"
	@echo ""
	@echo "Targets:"
	@echo "  make              - Build SQLite client executable"
	@echo "  make clean        - Remove all build artifacts"
	@echo "  make test         - Run compilation test"
	@echo "  make info         - Display build configuration"
	@echo "  make help         - Show this message"
	@echo ""
	@echo "Configuration:"
	@echo "  Architecture:     ARM2 (26-bit APCS)"
	@echo "  Platform:         RISC OS 3.1"
	@echo "  Target CPU:       ARM2 (~8 MHz)"
	@echo "  RAM target:       4MB"
	@echo "  Binary format:    RISC OS AIF (via ELF)"

# Display build information
info:
	@echo "Build Configuration:"
	@echo "  CC:               $(CC)"
	@echo "  Architecture:     ARM2 (26-bit)"
	@echo "  CFLAGS:           $(CFLAGS)"
	@echo "  LDFLAGS:          $(LDFLAGS)"
	@echo "  Source files:     $(words $(ALL_SOURCES))"
	@echo "  Build directory:  $(BUILD_DIR)"

# Test compilation (check syntax without full build)
test:
	@echo "Syntax check..."
	@for file in $(ALL_SOURCES); do \
		$(CC) -fsyntax-only $(CFLAGS) $$file || exit 1; \
	done
	@echo "All files compile successfully!"

# Size report
size: $(BIN_DIR)/sqlite,ff8
	@echo "Binary size: $(shell ls -lh $< | awk '{print $$5}')"
	@arm-none-eabi-objdump -h $(BIN_DIR)/sqlite.elf

# ============================================================================
# Development targets
# ============================================================================

# Full rebuild
rebuild: clean all

# Verbose build
verbose:
	@make all VERBOSE=1

# ============================================================================
# Installation/Distribution
# ============================================================================

dist: $(BIN_DIR)/sqlite,ff8
	@echo "Creating distribution package..."
	@mkdir -p $(DIST_DIR)/!SQLite
	@cp $(BIN_DIR)/sqlite,ff8 $(DIST_DIR)/!SQLite/
	@echo "Distribution ready in $(DIST_DIR)/"
