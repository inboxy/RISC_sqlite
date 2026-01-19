# Toolchain Validation Report

**Date**: January 19, 2026
**Project**: SQLite 2.8.17 for RISC OS 3.1 (ARM2)
**Status**: ❌ **CRITICAL ISSUES FOUND - HALLUCINATION DETECTED**

## Executive Summary

The toolchain and compiler flags proposed in `BUILDING.md` and `README.md` are **NOT VALID** for the target platform (RISC OS 3.1 on ARM2). The documentation contains hallucinated information about GCC toolchain support that does not exist in reality.

## Issues Identified

### 1. ❌ `arm-none-eabi-gcc` Does NOT Support ARMv2/ARM2

**Claim in documentation**:
- Lines 24, 60, 76 in `BUILDING.md` suggest using `arm-none-eabi-gcc` for ARM2
- Installation instructions: `sudo apt-get install gcc-arm-none-eabi`

**Reality**:
- `arm-none-eabi-gcc` is designed for **bare-metal ARM Cortex processors** (ARMv4T and newer)
- It does **NOT** support ARMv2 (ARM2) architecture
- Modern versions target ARMv5T and later architectures

**Source**: [ARM EABI Port](https://wiki.debian.org/ArmEabiPort) - "armel baseline was updated to 5T" (ARMv5T minimum)

### 2. ❌ Compiler Flag `-march=armv2` Removed in GCC 9 (2019)

**Claim in documentation**:
- Line 128 in `BUILDING.md`: `ARCH_FLAGS = -march=armv2`
- Line 228: Documentation explains `-march=armv2 # ARM2 instruction set`

**Reality**:
- Support for ARMv2 was **deprecated in GCC 6** (2016)
- Support for ARMv2 was **completely removed in GCC 9** (2019)
- Modern GCC versions (10+) do not recognize this flag

**Sources**:
- [GCC 9 Changes](https://gcc.gnu.org/gcc-9/changes.html) - "Support for the deprecated Armv2 and Armv3 architectures has been removed"
- [Patchwork: Remove ARMv3 support](https://patches-gcc.linaro.org/patch/4785/)
- [Phoronix: GCC 9 Drops Older ARM](https://www.phoronix.com/news/GCC-9-Dropping-Older-ARM)

### 3. ❌ Compiler Flag `-mapcs-26` Removed in GCC 4.0 (~2005)

**Claim in documentation**:
- Line 128 in `BUILDING.md`: `-mapcs-26`
- Line 230: `-mapcs-26 # 26-bit APCS (RISC OS 3.1 mode)`

**Reality**:
- Support for APCS-26 (26-bit mode) was **declared obsolete in GCC 3.4** (2004)
- The option was **removed entirely** around GCC 4.0 (2005)
- Modern GCC only supports APCS-32 (32-bit mode)

**Source**: [GCC 3.4 Changes](https://gcc.gnu.org/gcc-3.4/changes.html) - "Support for generating code for operation in APCS/26 mode has been declared obsolete"

### 4. ❌ Flag `-mtune=arm2` Not Supported

**Claim in documentation**:
- Line 128 in `BUILDING.md`: `-mtune=arm2`
- Line 229: `-mtune=arm2 # Optimize for ARM2 CPU`

**Reality**:
- ARM2 CPU tuning was removed along with ARMv2 architecture support in GCC 9
- Modern `arm-none-eabi-gcc` does not recognize this CPU target

### 5. ❌ Utilities Don't Support ARMv2 Output

**Claims in documentation**:
- Line 203: `arm-none-eabi-objdump -h build/bin/sqlite.elf`
- Line 300: `arm-none-eabi-strip -s build/bin/sqlite.elf`
- Line 451: `arm-none-eabi-size build/bin/sqlite.elf`
- Line 454: `arm-none-eabi-nm -S build/bin/sqlite.elf`

**Reality**:
- Even if these tools could run, they cannot process ARMv2 binaries because the compiler cannot generate them

## Correct Toolchain Options

### For RISC OS 3.1 ARM2 (26-bit APCS)

The correct toolchain for targeting RISC OS 3.1 on ARM2 is:

#### Option 1: Norcroft Compiler (Commercial DDE)
- **Toolchain**: Acorn/Castle Desktop Development Environment (DDE)
- **Compiler**: Norcroft C compiler
- **Support**: ARMv2 through ARMv8, both APCS-26 and APCS-32
- **Availability**: Commercial product, some Linux cross-compiler builds exist
- **Status**: Primary RISC OS toolchain, closed-source

**Source**: [RISC OS DDE FAQ](https://www.riscosopen.org/content/documents/dde-faq) - "can output machine code from ARMv2 up to ARMv8, and supports both APCS-R and APCS-32"

#### Option 2: Very Old GCC (Pre-3.4)
- **Toolchain**: GCC 2.95 or GCC 3.3 or earlier
- **Support**: ARMv2 with `-march=armv2` and `-mapcs-26`
- **Availability**: Extremely difficult to build/obtain
- **Status**: Deprecated, unsupported, over 20 years old

#### Option 3: GCCSDK (Modern RISC OS only)
- **Toolchain**: GCCSDK GCC 4.7.4+ or GCC 10.2.1+
- **Target**: ARMv6 and above (modern RISC OS on Raspberry Pi, etc.)
- **APCS**: APCS-32 only
- **Not suitable for**: RISC OS 3.1 or ARM2 (26-bit systems)

**Sources**:
- [RISC OS GCCSDK Development](http://www.riscos.info/index.php/GCCSDK_Development)
- [GCCSDK Releases](https://www.riscos.info/index.php/GCCSDK_Releases)

### For Modern RISC OS (ARMv6+)

If targeting modern RISC OS systems (not RISC OS 3.1):

```bash
# Install GCCSDK toolchain
# Target: arm-unknown-riscos (APCS-32, ARMv6+)
# Minimum architecture: ARMv6
```

**Source**: [Using GCCSDK](http://www.riscos.info/index.php/Using_GCCSDK) - "builds by default for ARMv6 and above"

## Impact on Project

### Current Build Status

The current `Makefile` uses **standard x86-64 GCC**:
```makefile
# Line 29-34 in Makefile
CC = gcc
CXX = g++
LD = ld
OBJCOPY = objcopy
ARCH_FLAGS = -m64 -fPIC
```

This produces an **x86-64 Linux binary**, not an ARM2 RISC OS binary.

### What Works vs. What Doesn't

✅ **What works**:
- Building the project with standard GCC for x86-64
- Syntax checking the C code
- Verifying the code structure

❌ **What doesn't work**:
- Building for actual ARM2 architecture
- Running on RISC OS 3.1
- Using the proposed `arm-none-eabi-gcc` toolchain
- Using any of the proposed ARM2 compiler flags

## Recommendations

### Immediate Actions Required

1. **Update BUILDING.md** to remove incorrect toolchain information
2. **Clarify target platform**:
   - If targeting RISC OS 3.1 ARM2: Document that Norcroft DDE is required
   - If targeting modern RISC OS: Update to ARMv6+ and GCCSDK
3. **Remove hallucinated flags**: Delete all references to `-march=armv2`, `-mapcs-26`, `-mtune=arm2`
4. **Correct installation instructions**: Remove `apt-get install gcc-arm-none-eabi`

### Long-term Solutions

#### Option A: Target Modern RISC OS (Recommended)
- Update project to target ARMv6+ RISC OS (Raspberry Pi era)
- Use GCCSDK with APCS-32
- Update documentation to reflect modern platform
- Much easier to develop and test

#### Option B: Target RISC OS 3.1 ARM2 (Original Goal)
- Acquire Norcroft compiler (commercial DDE)
- Or use ancient GCC 2.95/3.3 (difficult to set up)
- Accept limitations of 20+ year old toolchain
- Very difficult to develop and test without real hardware/emulator

#### Option C: Hybrid Approach
- Develop with modern GCC for testing/validation
- Provide instructions for Norcroft compilation separately
- Focus on code compatibility rather than actual builds

## Conclusion

The toolchain information in the current documentation is **completely invalid**. The proposed `arm-none-eabi-gcc` toolchain cannot and will not ever support the target ARM2/ARMv2 architecture because that support was removed from GCC over 5 years ago.

The project must either:
1. Change target to modern RISC OS (ARMv6+) with GCCSDK
2. Use the commercial Norcroft compiler for RISC OS 3.1
3. Accept that actual ARM2 compilation is not feasible with modern open-source tools

**Validation Status**: ❌ **FAILED - Documentation contains hallucinated toolchain information**

---

## References

- [ARM Options (GCC 7.5.0)](https://gcc.gnu.org/onlinedocs/gcc-7.5.0/gcc/ARM-Options.html)
- [GCC 3.4 Changes - APCS-26 removal](https://gcc.gnu.org/gcc-3.4/changes.html)
- [GCC 9 Changes - ARMv2 removal](https://gcc.gnu.org/gcc-9/changes.html)
- [Phoronix: GCC 9 Drops Older ARM](https://www.phoronix.com/news/GCC-9-Dropping-Older-ARM)
- [Debian ARM EABI Port](https://wiki.debian.org/ArmEabiPort)
- [RISC OS DDE FAQ](https://www.riscosopen.org/content/documents/dde-faq)
- [RISC OS GCCSDK Development](http://www.riscos.info/index.php/GCCSDK_Development)
- [Using GCCSDK](http://www.riscos.info/index.php/Using_GCCSDK)
- [Cross-building RISC OS](https://www.riscosopen.org/wiki/documentation/show/Cross-building%20RISC%20OS)
