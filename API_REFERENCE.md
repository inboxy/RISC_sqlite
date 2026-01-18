# SQLite 2.8.17 for RISC OS - API Reference

## Overview

Complete API documentation for SQLite running on RISC OS 3.1. This document describes all functions, structures, and commands available to users and developers.

---

## Table of Contents

1. [Interactive Commands](#interactive-commands)
2. [SQL Statements](#sql-statements)
3. [Dot Commands](#dot-commands)
4. [C API Functions](#c-api-functions)
5. [Data Types](#data-types)
6. [Error Codes](#error-codes)
7. [Configuration](#configuration)

---

## Interactive Commands

### Starting SQLite

#### Command-Line Usage

```bash
sqlite [options] [database_file]
```

**Arguments**:
- `database_file` - Path to database file (created if doesn't exist)
- Optional: pass database name as argument

**Examples**:
```bash
*sqlite                    # In-memory database
*sqlite mydata.db         # Open mydata.db
*sqlite /path/to/data.db  # Full path
```

#### From RISC OS Application

Double-click !SQLite icon to open interactive shell.

### Interactive Prompt

Once SQLite is running:

```
SQLite 2.8.17 for RISC OS 3.1
ARM2 targeting system with 4MB RAM
Type '.help' for available commands

sqlite>
```

---

## SQL Statements

### CREATE TABLE

Create a new table in the database.

**Syntax**:
```sql
CREATE TABLE table_name (column_definitions);
```

**Description**:
Creates a new table with the specified name and columns. Currently, the RISC OS port accepts the syntax but does not fully parse column definitions. Tables are created with basic metadata only.

**Parameters**:
- `table_name` - Name of the table (max 64 characters)
- `column_definitions` - Column specifications (syntax accepted, not parsed)

**Examples**:
```sql
CREATE TABLE users (id INTEGER, name TEXT);
CREATE TABLE products (id INTEGER, price REAL, description TEXT);
CREATE TABLE orders (id INTEGER, user_id INTEGER, status TEXT);
```

**Limitations**:
- Column types are not validated
- Constraints (PRIMARY KEY, NOT NULL, etc.) are not enforced
- Column definitions are syntax-checked but not stored

**Related**:
- `.schema` - View table schema
- `.tables` - List all tables
- `DROP TABLE` - Delete table

---

### DROP TABLE

Delete a table from the database.

**Syntax**:
```sql
DROP TABLE table_name;
```

**Description**:
Removes the specified table and its schema from the database. Once dropped, the table cannot be recovered unless the database file is restored from backup.

**Parameters**:
- `table_name` - Name of table to delete

**Examples**:
```sql
DROP TABLE users;
DROP TABLE old_products;
```

**Error Handling**:
- Error if table doesn't exist: "Table not found"

**Related**:
- `CREATE TABLE` - Create table
- `.tables` - List tables

---

### BEGIN

Start a transaction.

**Syntax**:
```sql
BEGIN;
```

**Description**:
Begins a transaction context. All subsequent SQL statements are grouped together. Use `COMMIT` to save changes or `ROLLBACK` to discard them.

**Examples**:
```sql
BEGIN;
CREATE TABLE temp (id INTEGER);
COMMIT;
```

**Related**:
- `COMMIT` - Save changes
- `ROLLBACK` - Discard changes

---

### COMMIT

Commit (save) the current transaction.

**Syntax**:
```sql
COMMIT;
```

**Description**:
Finalizes all changes made since the last `BEGIN` statement. The changes are permanent.

**Examples**:
```sql
BEGIN;
CREATE TABLE users (id INTEGER);
COMMIT;     -- Table is now created
```

**Related**:
- `BEGIN` - Start transaction
- `ROLLBACK` - Undo changes

---

### ROLLBACK

Discard (undo) the current transaction.

**Syntax**:
```sql
ROLLBACK;
```

**Description**:
Cancels all changes made since the last `BEGIN` statement. The database returns to its pre-transaction state.

**Examples**:
```sql
BEGIN;
CREATE TABLE temp (id INTEGER);
ROLLBACK;   -- Table is NOT created
```

**Related**:
- `BEGIN` - Start transaction
- `COMMIT` - Save changes

---

### PRAGMA

SQLite pragmas and configuration.

**Syntax**:
```sql
PRAGMA pragma_name [= value];
```

**Description**:
Execute pragmas (configuration commands). Currently, pragmas are accepted for compatibility but may not be fully implemented.

**Common Pragmas**:
```sql
PRAGMA cache_size=100;        -- Set page cache size
PRAGMA page_size=512;          -- Set page size
PRAGMA temp_store=MEMORY;      -- Use memory for temporary storage
```

**Note**: These are recognized but may not be fully implemented. Included for SQLite compatibility.

---

## Dot Commands

### .help

Display help information.

**Syntax**:
```
.help
```

**Description**:
Displays comprehensive help covering all available commands, SQL statements, and usage examples.

**Example**:
```
sqlite> .help
Dot commands:
  .quit, .exit         - Exit SQLite
  .help                - Show this message
  ...
```

---

### .quit / .exit

Exit SQLite.

**Syntax**:
```
.quit
.exit
```

**Description**:
Both commands exit the SQLite shell cleanly. Any uncommitted transactions are lost.

**Example**:
```
sqlite> .quit
Goodbye.
```

---

### .open <file>

Open or switch to a different database.

**Syntax**:
```
.open database_file
```

**Description**:
Opens the specified database file or creates it if it doesn't exist. If a database is already open, it will be closed first.

**Parameters**:
- `database_file` - Path to database file

**Examples**:
```
sqlite> .open test.db
Opened database 'test.db'

sqlite> .open /data/app.db
Opened database '/data/app.db'
```

**Error Handling**:
- Error if file cannot be opened: "Cannot open/create database file"

---

### .tables

List all tables in the current database.

**Syntax**:
```
.tables
```

**Description**:
Displays a list of all tables in the current database. Tables are listed one per line with indentation.

**Example**:
```
sqlite> .tables
Tables:
  users
  products
  orders
```

**Special Cases**:
- If no tables exist: "(No tables)"

---

### .schema [table_name]

Display table schema.

**Syntax**:
```
.schema
.schema table_name
```

**Description**:
Shows the schema (structure) of tables. Without arguments, shows all tables. With a table name, shows specific table.

**Parameters** (optional):
- `table_name` - Specific table to show

**Examples**:
```
sqlite> .schema
Table: users
  (no columns defined)
Table: products
  (no columns defined)

sqlite> .schema users
Table: users
  (no columns defined)
```

**Note**: Currently, column definitions are not fully parsed, so "(no columns defined)" appears for all tables.

---

### .verbose [on|off]

Toggle verbose output mode.

**Syntax**:
```
.verbose
.verbose on
.verbose off
```

**Description**:
When verbose mode is on, SQL statements are echoed before execution. Useful for debugging.

**Parameters** (optional):
- `on` - Enable verbose mode
- `off` - Disable verbose mode
- (no parameter) - Toggle current state

**Examples**:
```
sqlite> .verbose on
Verbose mode on
sqlite> CREATE TABLE test (id INTEGER);
Executing: CREATE TABLE test (id INTEGER);
```

---

### .headers [on|off]

Toggle column headers in results.

**Syntax**:
```
.headers
.headers on
.headers off
```

**Description**:
Controls whether column headers are displayed with query results. Useful for SELECT queries when fully implemented.

**Parameters** (optional):
- `on` - Show headers
- `off` - Hide headers
- (no parameter) - Toggle current state

**Examples**:
```
sqlite> .headers on
Headers on
```

**Note**: Currently mainly functional for query result headers when data queries are fully implemented.

---

## C API Functions

### sqlite_open()

Open or create a database file.

**Prototype**:
```c
sqlite *sqlite_open(const char *filename, int mode, char **errmsg);
```

**Parameters**:
- `filename` - Path to database file
- `mode` - Open mode (reserved for future use, pass 0)
- `errmsg` - Pointer to error message buffer (set on error)

**Returns**:
- Pointer to sqlite structure on success
- NULL on error

**Errors**:
- "Invalid filename"
- "Out of memory"
- "Cannot open/create database file"

**Example**:
```c
char *errmsg = NULL;
sqlite *db = sqlite_open("test.db", 0, &errmsg);
if (!db) {
    fprintf(stderr, "Error: %s\n", errmsg);
    return 1;
}
```

---

### sqlite_close()

Close a database.

**Prototype**:
```c
void sqlite_close(sqlite *db);
```

**Parameters**:
- `db` - Database pointer from sqlite_open()

**Description**:
Closes the database and frees all associated resources.

**Example**:
```c
sqlite_close(db);
```

---

### sqlite_exec()

Execute an SQL statement.

**Prototype**:
```c
int sqlite_exec(sqlite *db, const char *sql,
                int (*callback)(void*,int,char**,char**),
                void *arg, char **errmsg);
```

**Parameters**:
- `db` - Database pointer
- `sql` - SQL statement
- `callback` - Result callback function (NULL for no callback)
- `arg` - User data passed to callback
- `errmsg` - Pointer to error message

**Returns**:
- SQLITE_OK (0) on success
- SQLITE_ERROR (1) on error

**Callback Function**:
```c
int callback(void *arg, int argc, char **argv, char **colname)
{
    // arg: User-supplied argument
    // argc: Number of columns
    // argv: Column values
    // colname: Column names
    return 0;  // Return 0 to continue, 1 to stop
}
```

**Example**:
```c
char *errmsg = NULL;
int result = sqlite_exec(db, "CREATE TABLE test (id INTEGER);",
                         NULL, NULL, &errmsg);
if (result != SQLITE_OK) {
    fprintf(stderr, "Error: %s\n", errmsg);
}
```

---

### sqlite_get_table_count()

Get number of tables in database.

**Prototype**:
```c
int sqlite_get_table_count(sqlite *db);
```

**Returns**:
- Number of tables (0 if empty or error)

---

### sqlite_get_table_name()

Get table name by index.

**Prototype**:
```c
const char *sqlite_get_table_name(sqlite *db, int index);
```

**Parameters**:
- `db` - Database pointer
- `index` - Table index (0-based)

**Returns**:
- Table name on success
- NULL if index out of range

---

### sqlite_get_column_count()

Get number of columns in a table.

**Prototype**:
```c
int sqlite_get_column_count(sqlite *db, const char *table_name);
```

**Returns**:
- Number of columns
- 0 if table not found

---

### sqlite_get_column_name()

Get column name by index.

**Prototype**:
```c
const char *sqlite_get_column_name(sqlite *db, const char *table_name, int col_index);
```

**Returns**:
- Column name on success
- NULL if column not found

---

### sqlite_libversion()

Get SQLite version string.

**Prototype**:
```c
char *sqlite_libversion(void);
```

**Returns**:
- Version string "2.8.17"

**Example**:
```c
printf("SQLite version: %s\n", sqlite_libversion());
```

---

## Data Types

### Supported Column Types

SQLite supports the following column types (recognized but not strictly enforced in current implementation):

| Type | Description |
|------|-------------|
| INTEGER | 32-bit signed integer |
| TEXT | Text string (ASCII/ISO-8859-1) |
| REAL | Floating-point number |
| BLOB | Binary data (not implemented) |
| NULL | NULL value |

---

## Error Codes

### Return Values

| Code | Name | Meaning |
|------|------|---------|
| 0 | SQLITE_OK | Success |
| 1 | SQLITE_ERROR | General error |
| 2 | SQLITE_INTERNAL | Internal error |
| 3 | SQLITE_PERM | Permission denied |
| 4 | SQLITE_ABORT | Operation aborted |
| 5 | SQLITE_BUSY | Database busy |
| 6 | SQLITE_LOCKED | Database locked |
| 7 | SQLITE_NOMEM | Out of memory |
| 8 | SQLITE_READONLY | Database is read-only |
| 9 | SQLITE_IOERR | I/O error |

---

## Configuration

### Compile-Time Configuration

**Memory Settings** (in Makefile):
```makefile
-DSQLITE_DEFAULT_CACHE_SIZE=100    # Page cache size
-DSQLITE_DEFAULT_PAGE_SIZE=512     # Page size in bytes
-DSQLITE_MAX_LENGTH=1048576        # Max string length
-DSQLITE_MAX_SQL_LENGTH=10000      # Max SQL statement
```

**Feature Omission** (in Makefile):
```makefile
-DSQLITE_OMIT_TRIGGER              # No triggers
-DSQLITE_OMIT_VIEW                 # No views
-DSQLITE_OMIT_SUBQUERY             # No subqueries
... (13 OMIT flags total)
```

### Runtime Configuration

**Via PRAGMA**:
```sql
PRAGMA cache_size=100;
PRAGMA page_size=512;
PRAGMA temp_store=MEMORY;
```

---

## Example Programs

### Basic Database Usage

```c
#include <stdio.h>
#include "sqlite.h"

int main() {
    char *errmsg = NULL;
    sqlite *db;

    // Open database
    db = sqlite_open("example.db", 0, &errmsg);
    if (!db) {
        fprintf(stderr, "Open error: %s\n", errmsg);
        return 1;
    }

    // Create table
    if (sqlite_exec(db, "CREATE TABLE users (id INTEGER, name TEXT);",
                    NULL, NULL, &errmsg) != SQLITE_OK) {
        fprintf(stderr, "Create error: %s\n", errmsg);
        sqlite_close(db);
        return 1;
    }

    // List tables
    int count = sqlite_get_table_count(db);
    printf("Tables: %d\n", count);
    for (int i = 0; i < count; i++) {
        printf("  - %s\n", sqlite_get_table_name(db, i));
    }

    // Close database
    sqlite_close(db);

    return 0;
}
```

---

## Frequently Asked Questions

### Q: Can I use INSERT/SELECT?
**A**: These commands are recognized but not fully implemented in Phase 2. They will return a placeholder message. Phase 2.5+ will implement full data operations.

### Q: What's the maximum database size?
**A**: Limited by available RAM. Typical limit ~2MB with 4MB system.

### Q: Can I use multiple databases?
**A**: Yes, use `.open` command to switch between databases.

### Q: Are there any transactions?
**A**: Yes, BEGIN/COMMIT/ROLLBACK are supported for table operations.

### Q: What about Unicode support?
**A**: Current implementation supports ASCII and ISO-8859-1 only.

---

## See Also

- README.md - Project overview
- QUICK_START.md - Quick start guide
- SQL_FEATURES.md - SQL feature guide
- TROUBLESHOOTING.md - Common problems and solutions

---

**Document Version**: 1.0
**Last Updated**: January 2026
**For**: SQLite 2.8.17 RISC OS 3.1
