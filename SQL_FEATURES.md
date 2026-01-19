# SQLite 2.8.17 for RISC OS - SQL Features Guide

## Overview

Complete guide to SQL features supported by SQLite on RISC OS 3.1. Describes what works, what's partially implemented, and what's not supported.

---

## Feature Status Summary

| Feature | Status | Notes |
|---------|--------|-------|
| CREATE TABLE | ✅ Full | Basic implementation with in-memory storage |
| DROP TABLE | ✅ Full | Complete support with data cleanup |
| ALTER TABLE | ❌ Not supported | SQLite 2.x limitation |
| INSERT | ✅ Full | INSERT INTO ... VALUES implemented |
| SELECT | ✅ Partial | SELECT * FROM table working, no WHERE yet |
| UPDATE | 🚧 Planned | Framework ready, implementation pending |
| DELETE | 🚧 Planned | Framework ready, implementation pending |
| BEGIN | ✅ Full | Transaction support |
| COMMIT | ✅ Full | Transaction support |
| ROLLBACK | ✅ Full | Transaction support |
| CREATE INDEX | ❌ Omitted | Size optimization |
| CREATE VIEW | ❌ Omitted | Size optimization |
| CREATE TRIGGER | ❌ Omitted | Size optimization |
| PRAGMA | 🟡 Partial | Recognized, may not be implemented |
| WHERE | ❌ Not implemented | Planned for future |
| JOIN | ❌ Not implemented | Not planned (omitted) |
| GROUP BY | ❌ Not implemented | Planned for future |
| ORDER BY | ❌ Not implemented | Planned for future |
| UNION | ❌ Omitted | Compound SELECT disabled |
| SUBQUERIES | ❌ Omitted | Compile-time omission |

---

## Fully Implemented Features

### CREATE TABLE

**Status**: ✅ FULLY IMPLEMENTED

Create new tables in the database.

**Syntax**:
```sql
CREATE TABLE table_name (
    column1 type,
    column2 type,
    ...
);
```

**Limitations**:
- Column definitions are syntax-checked but not strictly enforced
- Constraints (PRIMARY KEY, NOT NULL, UNIQUE) are recognized but not validated
- Column types (INTEGER, TEXT, REAL) are accepted for compatibility

**Examples**:
```sql
CREATE TABLE users (id INTEGER, name TEXT, email TEXT);
CREATE TABLE products (id INTEGER, description TEXT, price REAL);
CREATE TABLE orders (id INTEGER, user_id INTEGER, status TEXT);
```

**Practical Use**:
- Tables can be created
- Schema can be viewed with `.schema`
- Tables appear in `.tables` listing

**Workarounds for Missing Features**:
- Use simple integer IDs manually (no AUTOINCREMENT)
- Manually validate NOT NULL constraints in application code
- Enforce UNIQUE constraints in application layer

---

### DROP TABLE

**Status**: ✅ FULLY IMPLEMENTED

Delete tables from database.

**Syntax**:
```sql
DROP TABLE [IF EXISTS] table_name;
```

**Limitations**:
- IF EXISTS clause accepted but behavior same as without it
- No confirmation required

**Examples**:
```sql
DROP TABLE users;
DROP TABLE old_data;
DROP TABLE IF EXISTS temporary;
```

**Practical Use**:
- Clean up unwanted tables
- Rebuild table structure (drop and recreate)
- Application cleanup

**Workarounds**:
- Before dropping critical tables, verify structure with `.schema`
- Consider backing up database before bulk drops

---

### BEGIN, COMMIT, ROLLBACK

**Status**: ✅ FULLY IMPLEMENTED

Transaction support for atomic operations.

**Syntax**:
```sql
BEGIN [TRANSACTION];
... SQL statements ...
COMMIT;
```

Or:

```sql
BEGIN [TRANSACTION];
... SQL statements ...
ROLLBACK;
```

**Behavior**:
- BEGIN: Starts transaction context
- COMMIT: Finalizes all changes
- ROLLBACK: Discards all changes since BEGIN

**Examples**:

*Committed transaction*:
```sql
BEGIN;
CREATE TABLE users (id INTEGER);
CREATE TABLE posts (id INTEGER);
COMMIT;        -- Both tables created
.tables        -- Shows both tables
```

*Rolled back transaction*:
```sql
BEGIN;
CREATE TABLE temp (id INTEGER);
ROLLBACK;      -- Table NOT created
.tables        -- Does not show temp table
```

**Practical Use**:
- Group related operations
- All-or-nothing operations
- Error recovery

**Note**: Currently works for table creation/deletion. Will work for all operations once data layer is implemented.

---

### PRAGMA

**Status**: 🟡 PARTIALLY IMPLEMENTED

Configuration pragmas (recognized for compatibility).

**Common Pragmas**:
```sql
PRAGMA cache_size=100;
PRAGMA page_size=512;
PRAGMA temp_store=MEMORY;
PRAGMA synchronous=OFF;
```

**Current Status**:
- Pragmas are recognized and accepted
- May or may not be actually enforced
- Included for SQLite compatibility

**Practical Use**:
- Compatibility with SQLite scripts
- Future-proofing application code
- Standard SQLite practices

---

### INSERT

**Status**: ✅ FULLY IMPLEMENTED

Insert data into tables with in-memory storage.

**Syntax**:
```sql
INSERT INTO table_name VALUES (value1, value2, ...);
```

**Features**:
- Supports quoted strings: `'Alice'`, `"Bob"`
- Supports unquoted values: `123`, `45.67`
- Dynamic row allocation (grows automatically)
- Maximum 1000 rows per table
- Automatic column creation for simple tables

**Examples**:
```sql
CREATE TABLE users;
INSERT INTO users VALUES ('Alice');
INSERT INTO users VALUES ('Bob');
INSERT INTO users VALUES ('Charlie');

CREATE TABLE scores;
INSERT INTO scores VALUES (100, 'Alice');
INSERT INTO scores VALUES (95, 'Bob');
```

**Limitations**:
- Does not support: `INSERT INTO table (col1, col2) VALUES (...)`
- Must use positional values matching table column order
- Maximum 1000 rows per table (MAX_ROWS)
- All values stored as strings internally

**Practical Use**:
- Add data to any table
- Build datasets for testing
- Populate reference tables
- Import data from external sources

---

### SELECT

**Status**: ✅ PARTIALLY IMPLEMENTED

Query data from tables with callback-based results.

**Syntax**:
```sql
SELECT * FROM table_name;
```

**Features**:
- Returns all rows from specified table
- Delivers results via callback function
- Provides column names and values
- Efficient in-memory query execution

**Examples**:
```sql
-- Select all from users table
SELECT * FROM users;

-- Results shown via shell callback
value
Alice
Bob
Charlie
```

**Current Limitations**:
- Only supports `SELECT *` (all columns)
- No column selection: `SELECT col1, col2` not supported yet
- No WHERE clause
- No ORDER BY, LIMIT, or OFFSET
- No JOIN operations
- No aggregate functions (COUNT, SUM, etc.)

**Practical Use**:
- View all table data
- Verify INSERT operations
- Export data
- Debugging and inspection

**Roadmap**:
- Column selection: `SELECT col1, col2 FROM table`
- WHERE clause: `SELECT * FROM table WHERE id = 5`
- ORDER BY: `SELECT * FROM table ORDER BY name`
- Aggregates: `COUNT(*), MAX(), MIN()`

---

## Partially Implemented Features

### UPDATE, DELETE

**Status**: 🚧 FRAMEWORK READY, IMPLEMENTATION PENDING

**Current Behavior**:
- SQL syntax recognized
- Statements accepted without error
- Return placeholder message: "[Command] not yet fully implemented"
- System continues working (no crash)

**Example**:
```
sqlite> UPDATE users SET name='NewName' WHERE id=1;
UPDATE not yet fully implemented
sqlite>
```

**When These Will Be Available**:
- Planned for Phase 2 continuation
- Will include WHERE clause support
- Will use existing in-memory storage

---

## Not Supported Features

### ALTER TABLE

**Status**: ❌ NOT SUPPORTED

Modify existing table structure.

**Why Not Supported**:
- SQLite 2.8.17 limitation (SQLite 3.x added this)
- Limited utility in low-memory environments
- Complex implementation in constrained space

**Workaround**:
1. Create new table with desired structure
2. Copy data (when data layer implemented)
3. Drop old table
4. Rename new table

Example:
```sql
CREATE TABLE users_new (id INTEGER, name TEXT, email TEXT);
-- (Copy data from old table when supported)
DROP TABLE users;
-- ALTER TABLE users RENAME TO users_old;  -- Not supported
-- ALTER TABLE users_new RENAME TO users;  -- Not supported
```

---

### CREATE INDEX, CREATE VIEW, CREATE TRIGGER

**Status**: ❌ NOT SUPPORTED (INTENTIONALLY OMITTED)

**Why Omitted**:
- Size optimization for 4MB system
- Saved ~70-100KB of code space
- Non-essential for basic database operations

**Workaround**:
- Indexes: Sort results in application (when SELECT available)
- Views: Store queries/logic in application
- Triggers: Implement logic in application code

---

### WHERE, JOIN, GROUP BY, ORDER BY

**Status**: ❌ NOT IMPLEMENTED

Advanced query features.

**Status in Code**:
- Parser not implemented
- Would require full query engine
- Part of data layer implementation

**Timeline**:
- Planned for Phase 2.5+ (full SQL engine)

**Workaround**:
- Retrieve all data and filter in application (when SELECT works)
- Manual sorting and grouping in application code
- Prepare data structures to avoid complex queries

---

### UNION, Compound SELECT

**Status**: ❌ OMITTED

Multiple SELECT with UNION.

**Why Omitted**:
- Compile-time flag: -DSQLITE_OMIT_COMPOUND_SELECT
- ~20KB code size savings
- Rarely needed for basic applications

**Workaround**:
- Execute separate SELECT statements
- Combine results in application

---

### Subqueries

**Status**: ❌ OMITTED

Nested SELECT in WHERE clause.

**Why Omitted**:
- Compile-time flag: -DSQLITE_OMIT_SUBQUERY
- Complex query planning
- ~15KB code savings
- Rarely needed for simple applications

**Workaround**:
- Use multiple queries in application
- Join results in application logic

---

## Data Type Support

### Supported Types

| Type | Support | Notes |
|------|---------|-------|
| INTEGER | ✅ Recognized | 32-bit signed integer |
| TEXT | ✅ Recognized | ASCII/ISO-8859-1 only, no UTF-8 |
| REAL | ✅ Recognized | Floating-point (soft float) |
| BLOB | ❌ Not supported | Binary data omitted for size |
| NULL | ✅ Supported | NULL values in any column |

### Type Enforcement

Currently:
- Column types are recognized for schema
- Not strictly enforced at data insertion (when INSERT available)
- Application should validate types

---

## SQL Syntax Specifications

### Identifiers

**Table Names**:
- Maximum length: 64 characters
- Allowed: Letters, numbers, underscore
- Must start with letter or underscore
- Example: `users`, `user_accounts`, `t1`

**Column Names** (when supported):
- Maximum length: 32 characters
- Same rules as table names
- Example: `id`, `user_name`, `email_address`

### String Literals

**Format**:
```sql
'single quoted string'
```

**Special Characters**:
- Single quote: `''` (escape with double quote)
- Newlines: Supported in literals
- Maximum length: 1MB (configurable)

**Example**:
```sql
'Alice Smith'
'O''Brien'  (escapes single quote)
```

### Comments

**Not Supported**:
- SQL-style comments (-- ) not currently implemented
- C-style comments (/* */) not supported
- Script comments (#) not recognized

**Workaround**:
- Use semicolon to skip lines in batch mode

---

## Limitations and Constraints

### Table Limits

| Limit | Value |
|-------|-------|
| Max tables per database | 32 |
| Max columns per table | 16 (framework) |
| Max rows per table | 1000 (MAX_ROWS) |
| Max table name length | 64 characters |
| Max column name length | 32 characters |
| Max table count | 32 (error if exceeded) |

### Statement Limits

| Limit | Value |
|-------|-------|
| Max SQL statement | 10,000 bytes |
| Max string/text | 1MB |
| Max number precision | 32-bit integer, floating-point |

### File System

| Aspect | Details |
|--------|---------|
| Database files | Any RISC OS valid filename |
| Path format | Unix-style (/) translated to RISC OS (.) |
| Max file size | Limited by filesystem (typically 2GB+) |
| Concurrent access | Not supported (single program only) |

---

## Best Practices

### For Table Design

1. **Use simple types**:
   ```sql
   CREATE TABLE users (id INTEGER, name TEXT);  -- Good
   ```

2. **Keep names simple**:
   ```sql
   CREATE TABLE user_profiles (...)  -- Good
   CREATE TABLE user_profile_info (...) -- OK but longer
   ```

3. **Limit to 32 tables total**:
   - Plan schema carefully
   - Combine related data into single tables when possible

4. **Use transactions for related operations**:
   ```sql
   BEGIN;
   CREATE TABLE users (id INTEGER);
   CREATE TABLE posts (id INTEGER);
   COMMIT;
   ```

### For Compatibility

1. **Use standard SQL**:
   ```sql
   CREATE TABLE name (column type);  -- Standard
   ```

2. **Avoid unsupported features**:
   - Don't use: ALTER TABLE, triggers, views, subqueries
   - Don't rely on: Indexes, UNION, constraints

3. **Test on target system**:
   - Verify on RPCEmu or real RISC OS
   - Don't assume desktop SQLite behavior

4. **Plan for future**:
   - Write code assuming INSERT/SELECT will work
   - Design schema for full SQL support
   - Keep database files portable

---

## SQL Feature Timeline

### Phase 1-2 (Complete)
- ✅ CREATE TABLE with in-memory storage
- ✅ DROP TABLE with data cleanup
- ✅ BEGIN/COMMIT/ROLLBACK
- ✅ Basic PRAGMA
- ✅ INSERT INTO VALUES (full implementation)
- ✅ SELECT * FROM table (partial - no WHERE)

### Phase 2 Continuation (In Progress)
- 🚧 UPDATE (modify data)
- 🚧 DELETE (remove data)
- 🚧 WHERE clause (conditional queries)
- 🚧 Dot commands (.tables, .schema, etc.)

### Phase 3+ (Future)
- ⏳ Column selection in SELECT
- ⏳ GROUP BY / ORDER BY (result processing)
- ⏳ Aggregate functions (COUNT, SUM, AVG, etc.)
- ⏳ File persistence (save/load databases)
- ⏳ JOIN operations (if space permits)

---

## Examples

### Creating a Database Schema

```sql
-- Create users table
CREATE TABLE users (
    id INTEGER,
    username TEXT,
    email TEXT
);

-- Create posts table
CREATE TABLE posts (
    id INTEGER,
    user_id INTEGER,
    title TEXT
);

-- Create comments table
CREATE TABLE comments (
    id INTEGER,
    post_id INTEGER,
    user_id INTEGER,
    text TEXT
);
```

### Using Transactions

```sql
-- Create related tables atomically
BEGIN;
CREATE TABLE accounts (id INTEGER);
CREATE TABLE transactions (id INTEGER);
CREATE TABLE balance_history (id INTEGER);
COMMIT;

-- Or rollback if needed
BEGIN;
CREATE TABLE temporary (id INTEGER);
-- Change mind and rollback
ROLLBACK;
```

### Schema Inspection

```sql
-- View all tables
.tables

-- View all schemas
.schema

-- View specific table
.schema users

-- List and view
.tables
.schema posts
.schema comments
```

---

## See Also

- API_REFERENCE.md - Complete API documentation
- QUICK_START.md - Quick start guide
- TROUBLESHOOTING.md - Problem solving
- README.md - Project overview

---

**Document Version**: 1.1
**Last Updated**: January 2026
**For**: SQLite 2.8.17 RISC OS 3.1
**Features**: INSERT and SELECT now fully/partially implemented (30KB executable)
