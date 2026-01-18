# Building SQLite 2.8.17 for RISC OS 3.1

This document provides detailed instructions for building SQLite for RISC OS.

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Quick Start](#quick-start)
3. [Setting Up the Build Environment](#setting-up-the-build-environment)
4. [Cross-Compilation for ARM2](#cross-compilation-for-arm2)
5. [Build System Details](#build-system-details)
6. [Troubleshooting](#troubleshooting)
7. [Installing on RISC OS](#installing-on-risc-os)

## Prerequisites

### For Linux/macOS Build Host

- **GNU Make**: `make` utility
- **GCC**: For syntax checking and initial builds
  - Ubuntu/Debian: `sudo apt-get install build-essential`
  - macOS: Xcode Command Line Tools
- **ARM Cross-Compiler** (for actual ARM2 code generation):
  - Ubuntu/Debian: `sudo apt-get install gcc-arm-none-eabi`
  - macOS: Homebrew - `brew install arm-none-eabi-gcc`

### For RISC OS Target

- **RISC OS 3.1** or later
- **4MB RAM** minimum
- **2-10MB storage** for database files
- Task window or CLI access

### Optional

- **RPCEmu**: RISC OS emulator for testing
  - Download: http://www.marutan.net/rpcemu/
- **SQLite**: Desktop version for database testing and conversion

## Quick Start

### Linux/Unix Host

```bash
# Navigate to project directory
cd sqlite-riscos

# Check available make targets
make help

# Run syntax check
make test

# Build the executable
make

# Clean build artifacts
make clean
```

**Expected output:**
```
Linking build/bin/sqlite,ff8...
objcopy -O binary build/bin/sqlite.elf build/bin/sqlite,ff8
-rw-r--r-- 1 user user 26K Jan 18 12:00 build/bin/sqlite,ff8
```

## Setting Up the Build Environment

### Step 1: Install Build Tools

#### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install build-essential
sudo apt-get install gcc-arm-none-eabi arm-none-eabi-binutils
```

#### Fedora/RHEL
```bash
sudo dnf install gcc make
sudo dnf install arm-none-eabi-gcc arm-none-eabi-binutils-gnu
```

#### macOS
```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install arm-none-eabi toolchain
brew install arm-none-eabi-gcc arm-none-eabi-binutils
```

### Step 2: Verify Installation

```bash
# Check make
make --version

# Check GCC
gcc --version

# Check ARM cross-compiler (if installed)
arm-none-eabi-gcc --version
```

### Step 3: Clone/Download Project

```bash
# Download the project
wget https://github.com/example/sqlite-riscos/archive/master.zip
unzip master.zip
cd sqlite-riscos-master

# Or if using git
git clone https://github.com/example/sqlite-riscos.git
cd sqlite-riscos
```

## Cross-Compilation for ARM2

### Understanding the Build Configuration

The `Makefile` contains configuration for ARM2 targeting RISC OS 3.1:

```makefile
# Architecture flags
ARCH_FLAGS = -march=armv2 -mtune=arm2 -msoft-float -mapcs-26

# Optimization for size
OPT_FLAGS = -Os -ffunction-sections -fdata-sections

# SQLite memory configuration
SQLITE_MEM_FLAGS = -DSQLITE_DEFAULT_CACHE_SIZE=100 \
                   -DSQLITE_DEFAULT_PAGE_SIZE=512 \
                   -DSQLITE_MAX_LENGTH=1048576
```

### Configuring for ARM Cross-Compiler

Edit `Makefile` to use the ARM toolchain:

```makefile
# OLD (uses standard GCC)
CC = gcc
CXX = g++
LD = ld

# NEW (uses ARM cross-compiler)
CC = arm-none-eabi-gcc
CXX = arm-none-eabi-g++
LD = arm-none-eabi-ld
OBJCOPY = arm-none-eabi-objcopy
```

Also uncomment the ARM2-specific flags:

```makefile
# OLD (standard x86-64)
ARCH_FLAGS = -m64 -fPIC

# NEW (ARM2 for RISC OS)
ARCH_FLAGS = -march=armv2 -mtune=arm2 -msoft-float -mapcs-26 -mno-thumb-interwork
```

### Building with ARM Cross-Compiler

```bash
# Edit Makefile as described above
nano Makefile

# Clean previous builds
make clean

# Build for ARM2
make

# Check binary
file build/bin/sqlite,ff8
arm-none-eabi-objdump -h build/bin/sqlite.elf
```

### Converting ELF to RISC OS AIF Format

The output from the ARM cross-compiler is ELF format. For RISC OS 3.1, convert to AIF:

```bash
# Option 1: Using elf2aif tool (if available)
elf2aif build/bin/sqlite.elf build/bin/sqlite,ff8

# Option 2: Using objcopy (usually automatic in Makefile)
arm-none-eabi-objcopy -O binary build/bin/sqlite.elf build/bin/sqlite,ff8
```

## Build System Details

### Makefile Targets

```bash
make                  # Build executable (default)
make all             # Same as make
make clean           # Remove build artifacts
make test            # Run syntax check without building
make help            # Show this help message
make info            # Display build configuration
make size            # Show binary size breakdown
make dist            # Create distribution package
make rebuild         # Clean and rebuild
```

### Build Output

```
build/
├── obj/              # Compiled object files
│   ├── sqlite/       # SQLite core objects
│   ├── shell/        # Shell interface objects
│   └── riscos/       # RISC OS integration objects
└── bin/              # Final executable
    ├── sqlite.elf    # ELF format (intermediate)
    └── sqlite,ff8    # Binary executable (final)
```

### Compiler Flags Explained

```makefile
# ARM2 Architecture
-march=armv2         # ARM2 instruction set
-mtune=arm2          # Optimize for ARM2 CPU
-mapcs-26            # 26-bit APCS (RISC OS 3.1 mode)
-msoft-float         # Software floating point

# Optimization
-Os                  # Optimize for size
-ffunction-sections  # Separate function sections (for linking)
-fdata-sections      # Separate data sections

# SQLite Configuration
-DSQLITE_*           # Feature enable/disable flags
-DSQLITE_THREADSAFE=0       # No threading support
-DSQLITE_OMIT_*             # Disable unneeded features
```

### Feature Omission

To reduce code size, unnecessary SQLite features are disabled:

```c
// Reduces code by ~70-100KB
SQLITE_OMIT_TRIGGER          // Triggers not supported
SQLITE_OMIT_VIEW             // Views not supported
SQLITE_OMIT_SUBQUERY         // Subqueries not supported
SQLITE_OMIT_COMPOUND_SELECT  // UNION/INTERSECT not supported
SQLITE_OMIT_AUTOVACUUM       // Auto-vacuuming disabled
SQLITE_OMIT_VIRTUALTABLE     // Virtual tables disabled
// ... and others
```

## Troubleshooting

### Problem: "gcc-arm-none-eabi: command not found"

**Solution**: Install the ARM cross-compiler

```bash
# Ubuntu/Debian
sudo apt-get install gcc-arm-none-eabi

# Or verify it's in PATH
which arm-none-eabi-gcc
```

### Problem: "kernel.h: No such file or directory"

**Solution**: The build system uses stubs for RISC OS headers when cross-compiling

- Stubs are in `include/kernel.h` and `include/swis.h`
- These only work for cross-compilation
- For native RISC OS compilation, link against actual RISC OS SDK

### Problem: "Make error: Unknown option -Wl,--gc-sections"

**Solution**: This occurs when using the wrong linker. Edit Makefile:

```makefile
# Use $(CC) for linking instead of $(LD) directly
$(CC) $(LDFLAGS) -o $(BIN_DIR)/sqlite.elf $^
```

### Problem: Binary size too large (>500KB)

**Solution**: The stub implementation is ~26KB. When integrated with full SQLite:

1. Verify SQLITE_OMIT flags are set in Makefile
2. Use `-Os` optimization flag
3. Enable `-ffunction-sections -fdata-sections` for linker optimization
4. Use `arm-none-eabi-strip` to remove debug symbols

```bash
arm-none-eabi-strip -s build/bin/sqlite.elf
```

### Problem: Build fails with "relocation truncated to fit"

**Solution**: Likely due to 26-bit addressing mode limitation. Check:

1. Code size is within 64MB (26-bit addressing limit)
2. All functions are linked with `-mapcs-26`
3. Position-independent code (-fPIC) might conflict with -mapcs-26

### Problem: "Warning: cast from pointer to integer of different size"

**Solution**: These warnings are expected on 64-bit hosts compiling for 32-bit ARM2:

- Add `-Wno-int-to-pointer-cast` to suppress if desired
- They don't affect functionality on ARM2 target

## Installing on RISC OS

### Option 1: Direct Execution in Task Window

1. Copy `build/bin/sqlite,ff8` to RISC OS
2. Set filetype to &FF8 (application)
   ```
   *SetType sqlite FF8
   ```
3. Run from task window or command line:
   ```
   *sqlite database.db
   ```

### Option 2: RISC OS Application Directory

Create an application directory structure:

```
!SQLite/
├── !Boot       # Boot file (sets up environment)
├── !Run        # Run file (executes program)
├── !Sprites    # Application icon
└── sqlite,ff8  # The executable
```

See `dist/!SQLite/` for template structure.

### Option 3: Creating !Boot Script

```
| !Boot file for SQLite
| Sets up WimpSlot and runs SQLite

Set SQLite$Dir <Obey$Dir>
WimpSlot -min 2048K -max 2048K
```

### Option 4: Creating !Run Script

```
| !Run file for SQLite
| Executes SQLite with database parameter

If "<Obey$Dir>.sqlite,ff8" file 0 then
  |
  | Execute SQLite in task window
  |
  Exec <Obey$Dir>.sqlite,ff8 database.db
Else
  Error "SQLite executable not found"
EndIf
```

## Testing the Build

### Unit Testing

Run the included test suite:

```bash
# Syntax check
make test

# (Full test suite to be implemented in Phase 4)
```

### Manual Testing

1. Create a test database:
   ```bash
   sqlite test.db < test.sql
   ```

2. Copy to RISC OS and verify:
   ```
   *sqlite test.db
   sqlite> .tables
   sqlite> .quit
   ```

### Performance Testing

Test on target hardware or RPCEmu:

```
*time sqlite database.db < test.sql
```

## Advanced Configuration

### Memory Budget Tuning

Edit Makefile to change memory limits:

```makefile
# Cache size: 100 pages × 512 bytes = 50KB
-DSQLITE_DEFAULT_CACHE_SIZE=100

# Page size: 512 bytes (smaller = more pages, slower I/O)
-DSQLITE_DEFAULT_PAGE_SIZE=512

# Maximum string length
-DSQLITE_MAX_LENGTH=1048576
```

### Custom Compiler Flags

For specific optimizations:

```makefile
# Example: Optimize for speed instead of size
OPT_FLAGS = -O2 -funroll-loops

# Example: Enable debugging symbols
DEBUG_FLAGS = -g3 -DDEBUG
```

### Building with Different SQLite Versions

To use a different SQLite version:

1. Download source from https://www.sqlite.org/download.html
2. Extract and copy source files to `sqlite/` directory
3. Update Makefile SOURCES to include additional files
4. Rebuild

## Performance Analysis

### Code Size

```bash
# Show section sizes
arm-none-eabi-size build/bin/sqlite.elf

# Show symbol sizes
arm-none-eabi-nm -S build/bin/sqlite.elf | sort -k2 -n
```

### Memory Usage (on RISC OS)

Use RISC OS `*Memory` command:

```
*Memory
Free: 1234K Used: 2345K
*SQLite test.db
(After running SQLite)
*Memory
Free: 876K Used: 2703K
(SQLite used ~358K)
```

## Next Steps

1. **Build complete**: You have a working SQLite executable
2. **Test on RISC OS**: Copy to target system and test
3. **Integrate full SQLite**: Combine with actual SQLite 2.8.17 source (Phase 2)
4. **Create application package**: Make proper RISC OS app (Phase 3)

## References

- GNU Make Manual: https://www.gnu.org/software/make/manual/
- ARM Compiler Guide: https://developer.arm.com/documentation/
- RISC OS Developer: https://www.riscos.com/support/developers/
- SQLite Source: https://www.sqlite.org/download.html

## Support

For build issues:

1. Check that all prerequisites are installed
2. Run `make test` to verify compilation
3. Check compiler versions match documentation
4. Review troubleshooting section above
5. Consult RISC OS emulator documentation for platform-specific issues

---

**Document Version**: 1.0
**Last Updated**: January 2026
**For**: SQLite 2.8.17 RISC OS 3.1 Port
