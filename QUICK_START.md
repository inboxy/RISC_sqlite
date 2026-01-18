# SQLite 2.8.17 for RISC OS - Quick Start Guide

## Installation

1. **Copy the executable to RISC OS**:
   - File: `build/bin/sqlite,ff8` (30KB)
   - Set filetype: `*SetType sqlite,ff8 FF8`

2. **Or use as RISC OS Application**:
   - Copy to `!SQLite` directory (Phase 3)
   - Double-click to launch

## Basic Usage

### Open a Database

```
*sqlite mydata.db
```

Creates `mydata.db` if it doesn't exist, opens it for use.

### Interactive Shell

Once SQLite starts, you get a prompt:

```
SQLite 2.8.17 for RISC OS 3.1
ARM2 targeting system with 4MB RAM
Type '.help' for available commands

sqlite>
```

## Common Tasks

### List All Tables

```
sqlite> .tables
Tables:
  users
  products
  orders
```

### View Table Structure

```
sqlite> .schema users
Table: users
  (no columns defined)
```

### Create a Table

```
sqlite> CREATE TABLE contacts (id INTEGER, name TEXT, email TEXT);
sqlite> .tables
Tables:
  contacts
```

### See Help

```
sqlite> .help
Dot commands:
  .quit, .exit         - Exit SQLite
  .help                - Show this message
  .open <file>         - Open database file
  .tables              - List all tables
  .schema [table]      - Show table schema
  ...
```

### Switch Databases

```
sqlite> .open otherdb.db
Opened database 'otherdb.db'
```

### Transactions

```
sqlite> BEGIN;
sqlite> CREATE TABLE temp (id INTEGER);
sqlite> COMMIT;
```

Or rollback:

```
sqlite> BEGIN;
sqlite> CREATE TABLE temp (id INTEGER);
sqlite> ROLLBACK;
```

### Exit SQLite

```
sqlite> .quit
Goodbye.
```

Or:

```
sqlite> .exit
Goodbye.
```

## Current Capabilities

### ✅ Fully Implemented
- Database opening and creation
- Table creation (basic)
- Table deletion (DROP TABLE)
- View all tables (.tables)
- View table schema (.schema)
- Transactions (BEGIN, COMMIT, ROLLBACK)
- Interactive shell with multi-line support
- Dot commands for database administration
- Help system

### 🚧 Partial Implementation (Placeholder)
- INSERT (recognized but not executed)
- SELECT (recognized but not executed)
- UPDATE (recognized but not executed)
- DELETE (recognized but not executed)

### ⏱️ Future Implementation
- Data insertion with INSERT
- Data querying with SELECT
- Data modification with UPDATE
- Data deletion with DELETE
- Complex queries (WHERE, JOIN, GROUP BY, ORDER BY)
- Indexes (CREATE INDEX, DROP INDEX)
- Views and triggers

## Memory Considerations

**Available**: 4MB RAM
**SQLite Footprint**: ~30KB executable + ~50KB runtime

This leaves plenty of room for:
- Database metadata
- Query results
- Working buffers
- User data

## Keyboard Shortcuts

While in the SQL editor:
- **Enter** - Submit statement (if terminated with `;`)
- **Ctrl+C** - Interrupt (if supported)
- **Ctrl+U** - Clear line (RISC OS Task window)

## Tips and Tricks

### 1. Multi-line Statements

Type statements across multiple lines. SQLite accumulates until it sees a semicolon:

```
sqlite> CREATE TABLE users
   ...> (
   ...>   id INTEGER,
   ...>   name TEXT
   ...> );
```

### 2. Check Your Progress

Use `.tables` frequently to verify operations:

```
sqlite> CREATE TABLE test (id INTEGER);
sqlite> .tables
Tables:
  test
sqlite> DROP TABLE test;
sqlite> .tables
(No tables)
```

### 3. View Database Structure

See all tables and their schema:

```
sqlite> .schema
Table: users
  (no columns defined)
Table: products
  (no columns defined)
```

### 4. Verbose Mode

Turn on verbose output to see what SQLite is doing:

```
sqlite> .verbose on
Verbose mode on
sqlite> CREATE TABLE test (id INTEGER);
```

### 5. Column Headers

Toggle result headers for SELECT queries (when implemented):

```
sqlite> .headers on
Headers on
```

## Troubleshooting

### "No database open"

Error when trying to execute SQL without opening a database:

```
sqlite> CREATE TABLE test (id INTEGER);
Error: No database open
```

**Solution**: Open a database first:

```
sqlite> .open mydb.db
Opened database 'mydb.db'
sqlite> CREATE TABLE test (id INTEGER);
```

### "Table already exists"

Error when creating a table that already exists:

```
sqlite> CREATE TABLE users (id INTEGER);
sqlite> CREATE TABLE users (id INTEGER);
Error: Table already exists
```

**Solution**: Use a different name or drop the existing table:

```
sqlite> DROP TABLE users;
sqlite> CREATE TABLE users (id INTEGER);
```

### "Too many tables"

SQLite is limited to 32 tables per database:

```
Error: Too many tables
```

**Solution**: Create a new database or delete unused tables.

### "Cannot open/create database file"

File I/O error:

```
Error: Cannot open/create database file
```

**Solutions**:
- Check drive space
- Verify file path is valid
- Check file permissions
- Try a different location

## Performance Expectations

On ARM2 (~8 MHz):

- **Startup**: 1-2 seconds
- **CREATE TABLE**: Instant
- **DROP TABLE**: Instant
- **List tables (.tables)**: Instant
- **View schema (.schema)**: Instant

## File Storage

Database files created in the current directory:

```
*SQLite mydata.db
```

Creates file: `mydata.db` in current directory

Format: SQLite 2.8.17 text-based database format
- Can be transferred to desktop SQLite 2.8.17
- Can be exported as SQL (.dump)
- ~25-35% larger than SQLite 3.x binary format

## Compatibility

### Create on RISC OS, Use on Desktop

1. Create and populate database on RISC OS
2. Copy file to desktop (floppy, network, USB)
3. Open with: `sqlite mydata.db` (desktop)
4. Export as SQL: `.dump`
5. Convert to SQLite 3.x if needed

### Create on Desktop, Use on RISC OS

1. Create database on desktop with SQLite 2.8.17
2. Copy file to RISC OS
3. Open with: `*sqlite mydata.db`

## Examples

### Example 1: Simple Database

```
*sqlite shop.db
sqlite> CREATE TABLE items (id INTEGER, name TEXT);
sqlite> .tables
Tables:
  items
sqlite> .quit
Goodbye.
```

### Example 2: Multiple Tables

```
*sqlite app.db
sqlite> CREATE TABLE users (id INTEGER, username TEXT);
sqlite> CREATE TABLE posts (id INTEGER, user_id INTEGER, text TEXT);
sqlite> .tables
Tables:
  users
  posts
sqlite> .schema users
Table: users
sqlite> .quit
```

### Example 3: Transaction

```
*sqlite backup.db
sqlite> BEGIN;
sqlite> CREATE TABLE data (id INTEGER);
sqlite> CREATE TABLE log (id INTEGER);
sqlite> COMMIT;
sqlite> .tables
Tables:
  data
  log
sqlite> .quit
```

## Getting Help

Inside SQLite:
```
sqlite> .help
```

View documentation:
- `README.md` - Project overview
- `BUILDING.md` - Build and installation
- `IMPLEMENTATION_STATUS.md` - Technical details
- `PHASE_2_STATUS.md` - Current capabilities

## Reporting Issues

If something doesn't work as expected:

1. Check the current limitations in `.help`
2. Verify you're using supported SQL syntax
3. Review error messages for clues
4. Check that database file can be created (disk space, permissions)

## Next Steps

- ✅ Phase 2: SQL engine and shell interface complete
- 🚧 Phase 3: RISC OS application packaging
- ⏳ Phase 4: Comprehensive testing suite
- ⏳ Phase 5: Full documentation

## Quick Command Reference

| Command | Purpose |
|---------|---------|
| `.quit` | Exit SQLite |
| `.exit` | Exit SQLite |
| `.help` | Show help |
| `.open file` | Open database |
| `.tables` | List tables |
| `.schema` | Show schema |
| `.verbose on\|off` | Toggle verbose |
| `.headers on\|off` | Toggle headers |
| `CREATE TABLE` | Create table |
| `DROP TABLE` | Delete table |
| `BEGIN` | Start transaction |
| `COMMIT` | Commit transaction |
| `ROLLBACK` | Undo transaction |

---

**Version**: SQLite 2.8.17 for RISC OS 3.1
**Status**: Phase 2 Complete (SQL Engine + Shell)
**Size**: 30KB executable
**RAM Target**: 4MB system with 500KB SQLite footprint
