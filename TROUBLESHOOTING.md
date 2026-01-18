# SQLite 2.8.17 for RISC OS - Troubleshooting Guide

## Common Issues and Solutions

### Application Won't Start

#### Problem: "Command not found"

**Symptom**: `*SQLite` or `*sqlite` gives "Command not found"

**Causes**:
1. SQLite executable not in path
2. Incorrect file type
3. File not executable

**Solutions**:
1. Use full path: `*!Boot:$.Apps.!SQLite.sqlite,ff8`
2. Set file type: `*SetType sqlite,ff8 FF8`
3. Verify file exists: `*Dir build:bin`
4. Copy executable to known location

**Example**:
```
*SetType SQLite,FF8 FF8
*SQLite database.db
```

---

#### Problem: "Out of memory"

**Symptom**: SQLite fails to start, insufficient memory error

**Causes**:
1. 4MB system too full
2. Other applications consuming memory
3. WimpSlot setting too low

**Solutions**:
1. Close other applications
2. Increase WimpSlot in !Boot:
   ```
   WimpSlot -min 2048K -max 3072K
   ```
3. Verify available RAM: `*Memory`
4. Use in-memory database: `*SQLite` (no filename)

---

#### Problem: Application crashes on startup

**Symptom**: SQLite starts then immediately closes

**Causes**:
1. Corrupted database file
2. File system error
3. Memory access violation
4. RISC OS version incompatibility

**Solutions**:
1. Delete problematic database file
2. Start fresh: `*SQLite newdb.db`
3. Check RMA state: `*Memory`
4. Verify RISC OS 3.1 or later
5. Check disk space available

---

### "No database open" Error

#### Problem

**Symptom**:
```
sqlite> CREATE TABLE test (id INTEGER);
Error: No database open
```

**Cause**: No database specified when starting SQLite

**Solution**: Specify database on command line

```
*SQLite mydata.db
```

Or use `.open` command:
```
sqlite> .open mydata.db
```

---

### "Table already exists" Error

#### Problem

**Symptom**:
```
sqlite> CREATE TABLE users (id INTEGER);
sqlite> CREATE TABLE users (id INTEGER);
Error: Table already exists
```

**Cause**: Table with same name already exists

**Solutions**:
1. Use different table name:
   ```sql
   CREATE TABLE users_v2 (id INTEGER);
   ```

2. Delete existing table first:
   ```sql
   DROP TABLE users;
   CREATE TABLE users (id INTEGER);
   ```

3. Check tables:
   ```
   sqlite> .tables
   ```

---

### "Table not found" Error

#### Problem

**Symptom**:
```
sqlite> DROP TABLE nonexistent;
Error: Table not found
```

**Cause**: Trying to drop table that doesn't exist

**Solutions**:
1. Verify table exists: `.tables`
2. Check exact name (case-sensitive, no quotes)
3. List schema: `.schema`

**Example**:
```
sqlite> .tables
Tables:
  users
  products
sqlite> DROP TABLE users
```

---

### "Too many tables" Error

#### Problem

**Symptom**:
```
sqlite> CREATE TABLE t32 (id INTEGER);
Error: Too many tables
```

**Cause**: Exceeded 32-table maximum

**Solutions**:
1. Combine related tables (normalize schema)
2. Delete unused tables:
   ```sql
   DROP TABLE old_data;
   ```
3. Create new database for additional tables

**Workaround**:
- Carefully plan schema to stay under 32 tables
- Use multi-column tables instead of separate tables

---

### "Cannot open/create database file" Error

#### Problem

**Symptom**:
```
*SQLite data.db
Error: Cannot open/create database file
```

**Causes**:
1. Disk full
2. Path doesn't exist
3. Permission denied
4. Invalid filename

**Solutions**:
1. Check disk space: `*Rename $.Disk 0` and observe free space
2. Use valid RISC OS filename
3. Try current directory: `*SQLite test.db`
4. Check path exists: `*Dir /path/to/dir`
5. Try different location

**Valid Filenames**:
```
sqlite test.db
sqlite mydata
sqlite /full/path/data.db
sqlite $.relative.path.db
```

**Invalid Filenames**:
```
sqlite >file|name<
sqlite data*.db
sqlite path/with/forward/slash  (use dots instead)
```

---

### Lost Data

#### Problem

**Symptom**: Tables or data disappeared

**Causes**:
1. Database file deleted
2. DROP TABLE executed
3. ROLLBACK discarded changes
4. Database file corrupted

**Solutions**:
1. **Backup**: Keep copies of important databases
   ```
   *Copy database.db database.backup
   ```

2. **Restore from backup**: If available
   ```
   *Copy database.backup database.db
   ```

3. **Recovery**: Limited recovery options for text-based SQLite 2.8.17
   - File may be human-readable if not corrupted
   - Try opening with text editor

4. **Prevention**:
   - Use transactions for related operations
   - Verify schema before critical drops
   - Regular backups

---

### Slow Performance

#### Problem

**Symptom**: SQLite commands take long time

**Causes**:
1. Large database
2. System low on memory
3. Disk full/fragmented
4. Other applications running

**Solutions**:
1. Close other applications
2. Free up memory: `*Memory`
3. Optimize disk: Defragment or clean up
4. Reduce database size (delete old tables)
5. Use simpler operations

**Performance Tips**:
- Minimize number of tables (combine when logical)
- Use transactions for batch operations
- Keep tables small (< 10MB ideally)
- Regular cleanup of old data

---

### File I/O Issues

#### Problem: "I/O error"

**Symptom**: Cannot read/write database file

**Causes**:
1. Disk error
2. File locked by another program
3. File system error
4. Bad sectors

**Solutions**:
1. Close all applications
2. Check disk with RISC OS tools
3. Verify file not corrupted: Compare with backup
4. Try different drive/location
5. Copy file and retry

---

### Memory Leaks

#### Problem

**Symptom**: Memory usage increases over time, system slows down

**Causes**:
- Repeated operations without cleanup (bug)
- Large number of transactions

**Solutions**:
1. Exit and restart SQLite
2. Check RMA: `*Memory`
3. Restart RISC OS if memory fragmented
4. Report issue if consistently happens

**Monitoring**:
```
*Memory                    (before SQLite)
*SQLite test.db
...operations...
*Memory                    (after SQLite)
sqlite> .quit
*Memory                    (after exit - should recover)
```

---

### Database Corruption

#### Problem

**Symptom**: Cannot open database, or commands fail

**Causes**:
1. Incomplete write (power loss)
2. Corrupted file system
3. Bad sectors
4. File truncated

**Solutions**:
1. **Backup first**: Don't modify original
   ```
   *Copy corrupted.db corrupted.backup
   ```

2. **Verify backup**: If you have one
   ```
   *SQLite backup.db
   sqlite> .tables
   ```

3. **Recovery options**:
   - For SQLite 2.8.17 (text format): Can inspect with editor
   - Try desktop SQLite 2.8.17 recovery tools
   - Last resort: Manual data reconstruction

4. **Prevention**:
   - Regular backups
   - Avoid power loss during operations
   - Use transactions for atomic ops

---

### Display/Formatting Issues

#### Problem: "Garbled output"

**Symptom**: Output shows strange characters

**Causes**:
1. Character encoding mismatch
2. Terminal settings
3. Control characters in data

**Solutions**:
1. Check RISC OS character set settings
2. Try different terminal/task window
3. Use `.headers off` to simplify output
4. ASCII text only (no Unicode)

---

### Command Doesn't Work

#### Problem: Unknown command or unexpected behavior

**Symptom**:
```
sqlite> .invalid_command
Error: Unknown command
```

**Solutions**:
1. Check command syntax: `.help`
2. Verify command is supported (see SQL_FEATURES.md)
3. Check for typos
4. Some commands may be placeholders:
   ```
   sqlite> INSERT INTO users VALUES (1, 'Alice');
   INSERT not yet fully implemented
   ```

**Command Help**:
```
sqlite> .help
```

---

### Port and Path Issues

#### Problem: Path format confusion

**Symptom**: Cannot access database file with slash path

**Cause**: RISC OS uses dots (.) not slashes (/) for paths

**Solution**: Convert paths
```
Unix path:    /path/to/database.db
RISC OS path: path.to.database/db

Correct:
*SQLite path.to.database/db

Or use current directory:
*SQLite database.db
```

---

### Cannot Close Database Cleanly

#### Problem: Stuck in SQLite

**Symptom**: Ctrl+C doesn't exit

**Solution**:
```
sqlite> .quit
```

or:
```
sqlite> .exit
```

or:
```
CTRL+C (if implemented)
```

If none work:
1. Close task window (may lose uncommitted data)
2. Or restart RISC OS

---

### Database Compatibility Issues

#### Problem: Cannot open desktop-created database

**Symptom**: File opens on desktop but not on RISC OS

**Causes**:
1. Created with SQLite 3.x (wrong version)
2. File format incompatible
3. Path issues

**Solutions**:
1. Verify you have SQLite 2.8.17 on desktop
2. Export to SQL and reimport:
   - Desktop: `sqlite db.db .dump > export.sql`
   - RISC OS: `sqlite newdb.db < export.sql`
3. Use SQLite 2 conversion tools

**Compatibility**:
- ✅ SQLite 2.8.17 ↔ SQLite 2.8.17
- ✅ Export/import via SQL works
- ⚠️ SQLite 3.x needs conversion

---

## Diagnostic Procedures

### Basic Health Check

1. Check system memory:
   ```
   *Memory
   ```

2. Start SQLite:
   ```
   *SQLite test.db
   ```

3. Test basic operations:
   ```
   sqlite> CREATE TABLE test (id INTEGER);
   sqlite> .tables
   sqlite> .schema
   sqlite> DROP TABLE test;
   sqlite> .quit
   ```

4. Check final state:
   ```
   *Memory
   ```

If all above work, SQLite is functioning correctly.

---

### Performance Diagnosis

1. Measure startup:
   ```
   *Time *SQLite startup.db
   ```

2. Create table:
   ```
   sqlite> *Time CREATE TABLE t (id INTEGER);
   sqlite> *Time CREATE TABLE t2 (id INTEGER);
   ```

3. Check memory:
   ```
   sqlite> (within RISC OS task window)
   Shift+F12 or ALT+Ctrl+Z to show RISC OS desktop
   *Memory
   ```

4. Record times and memory for comparison

---

### File Integrity Check

1. Backup database:
   ```
   *Copy database.db database.backup
   ```

2. Open and verify:
   ```
   *SQLite database.db
   sqlite> .tables
   sqlite> .schema
   sqlite> .quit
   ```

3. Compare size:
   ```
   *Info database.db
   *Info database.backup
   ```

---

## Getting Help

### Documentation

1. Start with this file (TROUBLESHOOTING.md)
2. Check QUICK_START.md for basic usage
3. Review API_REFERENCE.md for command details
4. Read SQL_FEATURES.md for feature status

### Built-in Help

```
sqlite> .help
```

Shows all available commands and basic usage.

### Example Scripts

See `tests/` directory:
- `example_basic.sql` - Basic operations
- `correctness_tests.sql` - Comprehensive testing
- `stress_tests.sql` - Stress testing

### Debug Mode

Use `.verbose` for debugging:
```
sqlite> .verbose on
```

This shows SQL statements being executed.

---

## Reporting Issues

If you encounter problems:

1. **Document the issue**:
   - Exact error message
   - Steps to reproduce
   - System configuration
   - Available memory

2. **Provide examples**:
   - SQL statements involved
   - Database file (if possible)
   - Previous success/failure history

3. **Check known issues**:
   - See SQL_FEATURES.md for limitations
   - Review IMPLEMENTATION_STATUS.md

4. **Reference information**:
   - SQLite version: 2.8.17
   - RISC OS version: 3.1 or later
   - Binary version (date)

---

## Prevention

### Best Practices

1. **Regular backups**:
   ```
   *Copy important.db important.backup
   ```

2. **Test before critical operations**:
   - Create test database
   - Verify operations work
   - Then operate on real data

3. **Use transactions**:
   ```sql
   BEGIN;
   -- operations here
   COMMIT;  -- or ROLLBACK;
   ```

4. **Check before drop**:
   ```
   sqlite> .schema table_name
   sqlite> DROP TABLE table_name;
   ```

5. **Monitor disk space**:
   - Keep sufficient free space
   - Avoid disk full errors

---

## Quick Reference

| Problem | Solution |
|---------|----------|
| Won't start | Check file type, use full path |
| No database open | Use `.open` or specify on command line |
| Table exists | Drop first or use different name |
| Too many tables | Limit to 32, combine related tables |
| Out of memory | Close other apps, increase WimpSlot |
| Slow | Free memory, simplify operations |
| Data loss | Restore from backup |
| Garbled output | Check terminal encoding |
| Command unknown | Use `.help`, check spelling |
| Can't exit | Use `.quit` or close task window |

---

**Document Version**: 1.0
**Last Updated**: January 2026
**For**: SQLite 2.8.17 RISC OS 3.1

**Remember**: Always keep backups of important databases!
