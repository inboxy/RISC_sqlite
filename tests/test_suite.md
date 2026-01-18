# SQLite 2.8.17 for RISC OS 3.1 - Test Suite

## Overview

Comprehensive test suite for SQLite on RISC OS 3.1. Tests verify functionality, performance, memory usage, and compatibility across all phases.

---

## Phase 1: Core RISC OS Port Tests

### Test 1.1: Application Startup
**Objective**: Verify application launches successfully

**Steps**:
1. Launch SQLite: `*sqlite test.db`
2. Observe application initializes
3. Verify prompt appears: `sqlite> `
4. Database file created if it doesn't exist

**Expected Result**: ✅ SUCCESS
- Application starts
- Prompt displays
- Database created

**Actual Result**: [To be run on RISC OS]

---

### Test 1.2: File I/O Operations
**Objective**: Verify VFS layer file operations work

**Steps**:
1. Create database: `*sqlite filetest.db`
2. Create table: `CREATE TABLE test (id INTEGER);`
3. Exit: `.quit`
4. Reopen: `*sqlite filetest.db`
5. List tables: `.tables`

**Expected Result**: ✅ SUCCESS
- File created on disk
- File survives application exit/restart
- File can be reopened

**Actual Result**: [To be run on RISC OS]

---

### Test 1.3: Memory Management
**Objective**: Verify memory allocation and deallocation

**Steps**:
1. Launch SQLite
2. Monitor RMA usage: `*Memory`
3. Create multiple tables
4. Exit SQLite
5. Check RMA usage again

**Expected Result**: ✅ SUCCESS
- Memory allocated correctly
- Memory released on exit
- No memory leaks (RMA usage decreases)

**Actual Result**: [To be run on RISC OS]

---

## Phase 2: SQL Execution Tests

### Test 2.1: CREATE TABLE
**Objective**: Verify table creation works

**Steps**:
```
sqlite> CREATE TABLE users (id INTEGER, name TEXT);
sqlite> CREATE TABLE products (id INTEGER, price TEXT);
sqlite> .tables
```

**Expected Result**: ✅ SUCCESS
- No error messages
- `.tables` shows both tables
- Output:
  ```
  Tables:
    users
    products
  ```

**Actual Result**: [To be run]

---

### Test 2.2: DROP TABLE
**Objective**: Verify table deletion works

**Steps**:
```
sqlite> CREATE TABLE temp (id INTEGER);
sqlite> .tables
sqlite> DROP TABLE temp;
sqlite> .tables
```

**Expected Result**: ✅ SUCCESS
- Table created
- First `.tables` shows `temp`
- Second `.tables` does NOT show `temp`

**Actual Result**: [To be run]

---

### Test 2.3: Schema Viewing
**Objective**: Verify .schema command works

**Steps**:
```
sqlite> CREATE TABLE contacts (id INTEGER, email TEXT);
sqlite> .schema
sqlite> .schema contacts
```

**Expected Result**: ✅ SUCCESS
- `.schema` shows all tables
- `.schema contacts` shows specific table

**Actual Result**: [To be run]

---

### Test 2.4: Transaction Support
**Objective**: Verify BEGIN/COMMIT/ROLLBACK

**Steps**:
```
sqlite> BEGIN;
sqlite> CREATE TABLE trans_test (id INTEGER);
sqlite> COMMIT;
sqlite> .tables
sqlite> BEGIN;
sqlite> CREATE TABLE trans_test2 (id INTEGER);
sqlite> ROLLBACK;
sqlite> .tables
```

**Expected Result**: ✅ SUCCESS
- `trans_test` appears in tables (committed)
- `trans_test2` does NOT appear (rolled back)

**Actual Result**: [To be run]

---

### Test 2.5: Dot Commands
**Objective**: Verify all dot commands function

| Command | Test | Result |
|---------|------|--------|
| `.quit` | Exit cleanly | ✓ |
| `.exit` | Same as quit | ✓ |
| `.help` | Show help text | ✓ |
| `.tables` | List tables | ✓ |
| `.schema` | Show schema | ✓ |
| `.open test.db` | Switch database | ✓ |
| `.verbose on/off` | Toggle verbose | ✓ |
| `.headers on/off` | Toggle headers | ✓ |

---

## Phase 3: RISC OS Application Tests

### Test 3.1: Application Launch from Desktop
**Objective**: Verify !SQLite application launches from desktop

**Steps**:
1. Copy !SQLite directory to Apps folder
2. Double-click !SQLite icon
3. Observe application launches
4. SQLite prompt appears

**Expected Result**: ✅ SUCCESS
- Application launches
- Prompt displays
- Database ready

**Actual Result**: [To be run]

---

### Test 3.2: WimpSlot Configuration
**Objective**: Verify memory allocation is correct

**Steps**:
1. Check system memory: `*Memory`
2. Launch !SQLite
3. Check memory again
4. Verify allocation matches !Boot setting

**Expected Result**: ✅ SUCCESS
- WimpSlot allocates 2-3MB
- Application runs stably
- No memory errors

**Actual Result**: [To be run]

---

### Test 3.3: Command-Line Launch
**Objective**: Verify launching with database parameter

**Steps**:
1. `*SQLite mydata.db` (from command line)
2. Application launches with database
3. Can create tables

**Expected Result**: ✅ SUCCESS
- Database opened
- Ready for SQL commands

**Actual Result**: [To be run]

---

## Phase 4: Performance Tests

### Test 4.1: Startup Performance
**Objective**: Measure time to launch application

**Procedure**:
1. Time startup from launch to prompt
2. Repeat 5 times
3. Calculate average

**Expected Result**: < 2 seconds average startup

**Actual Result**: [To be measured on RISC OS]

---

### Test 4.2: Table Creation Performance
**Objective**: Measure time to create multiple tables

**Procedure**:
```
Time creation of 32 tables (maximum)
sqlite> CREATE TABLE t1 (id INTEGER);
sqlite> CREATE TABLE t2 (id INTEGER);
... (repeat 32 times)
```

**Expected Result**: < 1 second for all 32 tables

**Actual Result**: [To be measured]

---

### Test 4.3: Schema Viewing Performance
**Objective**: Measure time to display schema

**Procedure**:
1. Create 32 tables
2. Time `.schema` command
3. Repeat 5 times
4. Calculate average

**Expected Result**: < 100ms per command

**Actual Result**: [To be measured]

---

## Phase 5: Stress & Edge Case Tests

### Test 5.1: Maximum Tables
**Objective**: Verify system handles maximum table count (32)

**Steps**:
1. Create 32 tables
2. Verify all listed in `.tables`
3. Try to create 33rd table

**Expected Result**: ✅ SUCCESS
- 32 tables created
- 33rd table fails with "Too many tables"

**Actual Result**: [To be run]

---

### Test 5.2: Long Table Names
**Objective**: Verify handling of maximum-length table names

**Steps**:
1. Create table with 64-character name (max)
2. Display schema
3. Drop table
4. Try table with 65+ characters

**Expected Result**: ✅ SUCCESS
- 64-char names work
- 65+ char names fail gracefully

**Actual Result**: [To be run]

---

### Test 5.3: Error Recovery
**Objective**: Verify graceful handling of invalid SQL

**Steps**:
```
sqlite> INVALID COMMAND;
sqlite> CREATE TABLE test;  (missing column definition)
sqlite> .invalid_command
sqlite> CREATE TABLE test (id INTEGER);  (should work)
```

**Expected Result**: ✅ SUCCESS
- Errors reported clearly
- System continues working
- Subsequent valid commands work

**Actual Result**: [To be run]

---

### Test 5.4: Database Swap
**Objective**: Verify switching between databases

**Steps**:
```
sqlite> .open db1.db
sqlite> CREATE TABLE t1 (id INTEGER);
sqlite> .open db2.db
sqlite> CREATE TABLE t2 (id INTEGER);
sqlite> .open db1.db
sqlite> .tables
```

**Expected Result**: ✅ SUCCESS
- Can switch databases
- Each database maintains own tables
- Reopening db1 shows t1, not t2

**Actual Result**: [To be run]

---

### Test 5.5: Repeated Operations
**Objective**: Verify stability during extended use

**Procedure**:
1. Run 100+ table creation/deletion cycles
2. Monitor for memory leaks
3. Verify no crashes or corruption

**Expected Result**: ✅ SUCCESS
- No crashes
- No memory leaks (RMA stable)
- Data integrity maintained

**Actual Result**: [To be run on RISC OS, long-term test]

---

## Memory Tests

### Test M1: Memory Footprint
**Objective**: Measure actual memory usage

**Procedure**:
1. Check RMA before launch: `*Memory`
2. Launch SQLite: `*SQLite test.db`
3. Check RMA after launch
4. Calculate difference

**Expected Result**: < 500KB additional RMA used

**Actual Result**: [To be measured]

---

### Test M2: Memory Stability
**Objective**: Verify memory doesn't degrade over time

**Procedure**:
1. Create and drop 50 tables
2. Record RMA usage after each cycle
3. Verify RMA usage remains stable

**Expected Result**: ✅ SUCCESS
- RMA usage stable
- No fragmentation growth
- Recovered properly

**Actual Result**: [To be run]

---

## Compatibility Tests

### Test C1: Database Compatibility
**Objective**: Verify databases can be transferred between systems

**Steps**:
1. Create database on RISC OS with SQLite
2. Copy file to Linux/desktop
3. Open with desktop SQLite 2.8.17
4. Verify data integrity

**Expected Result**: ✅ SUCCESS
- Database opens
- Content matches
- No corruption

**Actual Result**: [To be run]

---

### Test C2: SQL Feature Compatibility
**Objective**: Verify SQL features work as expected

**Procedure**:
1. CREATE TABLE
2. DROP TABLE
3. PRAGMA statements
4. Transaction commands

**Expected Result**: ✅ SUCCESS
- All supported features work
- All commands recognized
- Appropriate messages for unsupported features

**Actual Result**: [To be run]

---

## Test Execution Checklist

### Phase 1 Tests (Core Port)
- [ ] Test 1.1: Application Startup
- [ ] Test 1.2: File I/O Operations
- [ ] Test 1.3: Memory Management

### Phase 2 Tests (SQL Engine)
- [ ] Test 2.1: CREATE TABLE
- [ ] Test 2.2: DROP TABLE
- [ ] Test 2.3: Schema Viewing
- [ ] Test 2.4: Transaction Support
- [ ] Test 2.5: Dot Commands

### Phase 3 Tests (RISC OS App)
- [ ] Test 3.1: Application Launch from Desktop
- [ ] Test 3.2: WimpSlot Configuration
- [ ] Test 3.3: Command-Line Launch

### Performance Tests
- [ ] Test 4.1: Startup Performance
- [ ] Test 4.2: Table Creation Performance
- [ ] Test 4.3: Schema Viewing Performance

### Stress Tests
- [ ] Test 5.1: Maximum Tables
- [ ] Test 5.2: Long Table Names
- [ ] Test 5.3: Error Recovery
- [ ] Test 5.4: Database Swap
- [ ] Test 5.5: Repeated Operations

### Memory Tests
- [ ] Test M1: Memory Footprint
- [ ] Test M2: Memory Stability

### Compatibility Tests
- [ ] Test C1: Database Compatibility
- [ ] Test C2: SQL Feature Compatibility

---

## Test Results Summary

### Completion Status
- Total Tests: 21
- Tests Completed: [To be updated]
- Tests Passed: [To be updated]
- Tests Failed: [To be updated]
- Pass Rate: [To be updated]

### Performance Results
- Average Startup: [To be measured]
- Table Creation: [To be measured]
- Schema Display: [To be measured]
- Memory Usage: [To be measured]

### Quality Assessment
- Stability: [To be assessed]
- Reliability: [To be assessed]
- Performance: [To be assessed]
- Overall: [To be assessed]

---

## How to Run Tests

### On RISC OS 3.1

1. **Basic Functionality Test**:
   ```
   *SQLite testdb.db
   sqlite> CREATE TABLE test (id INTEGER);
   sqlite> .tables
   sqlite> DROP TABLE test;
   sqlite> .quit
   ```

2. **Comprehensive Test** (see test procedures above):
   - Run each test in sequence
   - Record results
   - Compare with expected outcomes

3. **Performance Test**:
   - Use `*Time` command to measure
   - Record timestamps
   - Calculate averages

---

## Known Issues to Verify

1. **INSERT/SELECT not implemented**: Expected placeholder message
2. **Column definitions not parsed**: Expected limitation
3. **No concurrent access**: Expected and by design
4. **ASCII only**: Expected limitation

---

## Sign-Off

**Test Suite Created**: January 2026
**Test Harness Ready**: Yes
**Documentation Complete**: Yes
**Ready for Testing**: Yes

---

**Next Steps**: Run tests on RISC OS 3.1 and record results
