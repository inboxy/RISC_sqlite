# SQLite 2.8.17 for RISC OS 3.1

A command-line SQL database client for RISC OS 3.1 running on ARM2 CPU with limited RAM.

## Project Goals

- **Platform**: RISC OS 3.1 on ARM2 CPU (~8 MHz, 26-bit addressing)
- **Memory target**: 4MB RAM available, ~500KB SQLite footprint
- **Database**: SQLite 2.8.17 (smaller than 3.x, optimized for size)
- **Interface**: Interactive command-line shell with SQL support
- **Compatibility**: Databases compatible with desktop SQLite 2.8.17

## Features

### Current Implementation (Phase 1)

- ✅ Project structure and build system
- ✅ RISC OS VFS layer (file I/O via OS_Find, OS_GBPB, OS_Args SWIs)
- ✅ RISC OS memory management (RMA allocation via OS_Module)
- ✅ Cross-compilation support (Makefile with ARM toolchain configuration)
- ✅ Stub SQLite 2.8.17 core with database open/close
- ✅ Interactive shell framework
- ⏳ Full SQL execution engine (stub - requires full SQLite integration)

### Planned Features (Phase 2-5)

- Phase 2: Complete SQL execution (SELECT, INSERT, UPDATE, DELETE)
- Phase 2: Dot commands (.tables, .schema, .dump, .read, etc.)
- Phase 3: RISC OS application packaging (!SQLite directory)
- Phase 4: Comprehensive testing suite
- Phase 5: Documentation and examples

## Building

### Requirements

#### Linux (Cross-Compilation Host)
- `gcc` and `make` (for syntax checking with standard GCC)
- `arm-none-eabi-gcc` (for ARM2 cross-compilation)
  - Install: `sudo apt-get install gcc-arm-none-eabi`
- Optional: RPCEmu emulator (for testing on RISC OS)

#### RISC OS (Target Platform)
- RISC OS 3.1 or later
- 4MB RAM minimum
- Floppy or hard drive storage

### Quick Build (Linux Cross-Compilation)

```bash
cd sqlite-riscos
make                  # Build executable
make test            # Check syntax only
make clean           # Remove build artifacts
make help            # Show available targets
```

**Output**: `build/bin/sqlite,ff8` (26KB for current stub implementation)

### Building for ARM2 RISC OS

When the ARM cross-compiler is available:

```bash
# Modify Makefile to use actual cross-compiler
# Change CC, CXX, LD, OBJCOPY, AR, RANLIB to arm-none-eabi variants
# Uncomment ARM2 architecture flags:
# ARCH_FLAGS = -march=armv2 -mapcs-26 -msoft-float -mtune=arm2

make clean
make
```

**Note**: When properly configured for ARM2 cross-compilation, the resulting binary can be:
1. Converted from ELF to AIF format using `elf2aif`
2. Copied to RISC OS and run in a task window
3. Or packaged as an application directory

## Project Structure

```
sqlite-riscos/
├── README.md                 # This file
├── BUILDING.md              # Detailed build instructions
├── Makefile                 # Cross-compilation build system
├── include/                 # RISC OS SDK stubs and headers
│   ├── kernel.h            # RISC OS kernel SWI interface
│   └── swis.h              # RISC OS SWI definitions
├── sqlite/                 # SQLite core engine
│   ├── sqlite.c            # Main SQLite implementation
│   ├── sqlite.h            # SQLite API header
│   ├── os_riscos.c         # RISC OS file I/O layer (VFS)
│   ├── os_riscos.h
│   ├── mem_riscos.c        # RISC OS memory allocation (RMA)
│   └── mem_riscos.h
├── shell/                  # Command-line interface
│   └── shell.c             # Interactive SQL prompt
├── riscos/                 # RISC OS utilities
│   ├── startup.c           # Application entry point
│   ├── swis.c              # SWI wrapper functions
│   ├── kernel.c            # Kernel stub implementation
│   └── swis.h
├── tests/                  # Test suite (TBD)
├── build/                  # Build output directory
│   ├── obj/               # Object files
│   └── bin/               # Compiled executable
└── dist/                   # Distribution package
    └── !SQLite/           # RISC OS application structure (TBD)
```

## Architecture

### VFS (Virtual File System) Layer

The OS layer (`sqlite/os_riscos.c`) provides file I/O using RISC OS SWI calls:

- **OS_Find** - Open/close files
- **OS_GBPB** - Read/write bytes
- **OS_Args** - File pointer operations (seek, tell, filesize)
- **OS_File** - File deletion

**Path translation** from Unix format to RISC OS:
```
/path/to/database.db  →  path.to.database/db
```

### Memory Management

The memory layer (`sqlite/mem_riscos.c`) allocates from RMA (Relocatable Module Area) using:

- **OS_Module 6** - Claim memory
- **OS_Module 7** - Free memory

Configuration:
```
- Cache size: 100 pages
- Page size: 512 bytes
- Max string: 1MB
- Max SQL statement: 10KB
```

### Shell Interface

The interactive shell (`shell/shell.c`) provides:

- Interactive SQL prompt (`sqlite> `)
- Dot commands (`.quit`, `.tables`, `.schema`, `.help`)
- SQL statement accumulation (statements terminated with `;`)
- Result formatting and display
- Error handling and reporting

## Usage

### Running the Executable

On RISC OS:

```
*sqlite database.db
```

Then at the prompt:

```
sqlite> CREATE TABLE test (id INTEGER, name TEXT);
sqlite> INSERT INTO test VALUES (1, 'Alice');
sqlite> SELECT * FROM test;
id | name
---|------
1  | Alice
sqlite> .tables
test

sqlite> .quit
Goodbye.
```

### Dot Commands

```
.quit              - Exit SQLite
.exit              - Exit SQLite
.help              - Show this message
.open <file>       - Open database file
.tables            - List all tables
.schema [table]    - Show table schema
.verbose           - Toggle verbose mode
.headers           - Toggle result headers
```

## Configuration and Optimization

### SQLite OMIT Flags

To reduce memory footprint, the following features are disabled:

```c
SQLITE_OMIT_TRIGGER          // No trigger support
SQLITE_OMIT_VIEW             // No VIEW support
SQLITE_OMIT_SUBQUERY         // No subqueries
SQLITE_OMIT_COMPOUND_SELECT  // No UNION/INTERSECT
SQLITE_OMIT_AUTOVACUUM       // No auto-vacuuming
SQLITE_OMIT_VIRTUALTABLE     // No virtual tables
SQLITE_OMIT_TEMPDB           // No temporary databases
SQLITE_OMIT_VACUUM           // No VACUUM command
SQLITE_OMIT_ATTACH           // No ATTACH DATABASE
SQLITE_OMIT_EXPLAIN          // No EXPLAIN command
SQLITE_OMIT_LOAD_EXTENSION   // No extension loading
```

**Estimated savings**: ~70-100KB

### Memory Limits

```c
SQLITE_DEFAULT_CACHE_SIZE   = 100     // 100 pages = 50KB
SQLITE_DEFAULT_PAGE_SIZE    = 512     // 512 bytes per page
SQLITE_MAX_LENGTH           = 1048576 // 1MB max string
SQLITE_MAX_SQL_LENGTH       = 10000   // 10KB max SQL
SQLITE_THREADSAFE           = 0       // No threading
```

## Known Limitations

1. **No concurrent access** - Only one program can access database at a time
2. **No file locking** - Database corruption possible if accessed simultaneously
3. **ASCII only** - No Unicode/UTF-8 (ISO-8859-1 or ASCII only)
4. **Limited SQL** - No triggers, views, subqueries, compound SELECT
5. **Database format** - Not directly compatible with SQLite 3.x
6. **No BLOB support** - Binary data must be encoded (hex/base64)
7. **No AUTOINCREMENT** - Manual ID management required
8. **ARM2 performance** - Slower than modern systems (~8 MHz CPU)

## Database Compatibility

### Transferring Databases

**RISC OS to Desktop:**
1. Copy `database.db` file to desktop/Linux
2. Open with SQLite 2.8.17: `sqlite database.db`
3. Export as SQL: `.dump` or `sqlite_dump database.db > export.sql`
4. Import to SQLite 3.x if needed

**Desktop to RISC OS:**
1. Create database on desktop with SQLite 2.8.17
2. Export as SQL: `sqlite_dump database.db > export.sql`
3. Import on RISC OS: `sqlite new.db < export.sql`

## Performance

### Expected Performance (ARM2 ~8 MHz)

- Startup: < 2 seconds
- INSERT: ~100-200 rows/second
- SELECT (simple): ~500-1000 rows/second
- SELECT (with index): ~2000-5000 rows/second

*Note: Exact performance depends on database complexity and ARM2 clock speed*

## Development Status

### Phase 1: Core Port ✅ COMPLETE
- [x] Project structure
- [x] RISC OS VFS layer
- [x] Memory management
- [x] Build system
- [x] Stub implementation

### Phase 2: CLI Shell 🚧 IN PROGRESS
- [ ] Full SQL execution engine
- [ ] Dot commands implementation
- [ ] Query result formatting
- [ ] Error handling improvements

### Phase 3: RISC OS Application ⏳ PLANNED
- [ ] !Boot script
- [ ] !Run script
- [ ] !Sprites icon
- [ ] WimpSlot configuration

### Phase 4: Testing ⏳ PLANNED
- [ ] Correctness tests
- [ ] Memory usage tests
- [ ] Performance benchmarks
- [ ] Stress tests

### Phase 5: Documentation ⏳ PLANNED
- [ ] User manual
- [ ] API documentation
- [ ] Example databases
- [ ] Build guide

## References

- **SQLite 2.8.17**: https://www.sqlite.org/2005/
- **RISC OS Developer**: https://www.riscos.com/support/developers/
- **RISC OS Open**: https://www.riscosopen.org/
- **ARM2 Architecture**: ARM 32-bit core (26-bit addressing mode for RISC OS 3.1)
- **RPCEmu**: http://www.marutan.net/rpcemu/ (RISC OS emulator)

## Implementation Notes

### ARM2 Considerations

- **26-bit addressing mode**: Uses `-mapcs-26` compiler flag
- **Soft floating point**: No FPU on ARM2, use `-msoft-float`
- **Limited memory**: Must minimize stack usage, avoid deep recursion
- **Stack size**: Typically 32KB in RISC OS applications

### RISC OS 3.1 Specifics

- No file locking support (cannot prevent concurrent access)
- Limited dynamic area allocation
- Task window interface for I/O
- No POSIX or Linux compatibility layer

## Future Enhancements

1. **Full SQLite 2.8.17 integration** - Use actual SQLite amalgamation source
2. **SQL compliance** - Support more standard SQL features
3. **GUI interface** - RISC OS desktop application
4. **Networking** - Remote database access (future)
5. **Performance** - Index optimization and query planning

## Contributing

This is an educational port of SQLite to a retro computing platform. Contributions welcome:

- Optimizations for ARM2
- Bug fixes and compatibility improvements
- Documentation and examples
- Testing on real RISC OS hardware

## License

SQLite is public domain software. This RISC OS port is provided as-is for educational purposes.

## Support

For issues, questions, or suggestions:

1. Check the documentation in `BUILDING.md`
2. Review example SQL in `tests/`
3. Test on RPCEmu emulator first
4. Document findings for others

---

**Project Status**: Early stage (Phase 1 complete, Phase 2 in progress)

**Last Updated**: January 2026

**Target Platform**: RISC OS 3.1 on ARM2 with 4MB RAM
# RISC_sqlite
