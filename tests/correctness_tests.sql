-- SQLite Correctness Test Suite
-- Tests basic SQL functionality and error handling
-- Run with: sqlite test.db < correctness_tests.sql

-- ============================================================================
-- TEST 1: Database and Table Operations
-- ============================================================================

-- Create test table
CREATE TABLE test_table (id INTEGER, name TEXT, value INTEGER);

-- Verify table was created (use .tables command in shell)
.tables

-- Show table schema
.schema test_table

-- ============================================================================
-- TEST 2: Multiple Table Management
-- ============================================================================

-- Create multiple tables for comprehensive testing
CREATE TABLE users (id INTEGER, username TEXT);
CREATE TABLE products (id INTEGER, description TEXT);
CREATE TABLE orders (id INTEGER, status TEXT);

-- List all tables (should show 4 tables: test_table, users, products, orders)
.tables

-- ============================================================================
-- TEST 3: Schema Inspection
-- ============================================================================

-- Show specific table schema
.schema users

-- Show all schemas
.schema

-- ============================================================================
-- TEST 4: Transaction Support
-- ============================================================================

-- Test transaction: CREATE and COMMIT
BEGIN;
CREATE TABLE committed_table (id INTEGER);
COMMIT;

-- Verify table exists (should appear in .tables)
.tables

-- Test transaction: CREATE and ROLLBACK
BEGIN;
CREATE TABLE rolled_back_table (id INTEGER);
ROLLBACK;

-- Verify table was rolled back (should NOT appear in .tables)
.tables

-- ============================================================================
-- TEST 5: Error Handling
-- ============================================================================

-- Test 5.1: Duplicate table creation (should error)
CREATE TABLE users (id INTEGER);

-- Test 5.2: Invalid SQL (should error gracefully)
INVALID COMMAND;

-- Test 5.3: Create and drop same table
CREATE TABLE error_test (id INTEGER);
DROP TABLE error_test;
DROP TABLE error_test;  -- Should error: table not found

-- ============================================================================
-- TEST 6: Maximum Tables
-- ============================================================================

-- Create up to 32 tables (system maximum)
-- This tests the limit of the system
-- If system crashes or behaves incorrectly, this is a failure

CREATE TABLE t1 (id INTEGER);
CREATE TABLE t2 (id INTEGER);
CREATE TABLE t3 (id INTEGER);
CREATE TABLE t4 (id INTEGER);
CREATE TABLE t5 (id INTEGER);
CREATE TABLE t6 (id INTEGER);
CREATE TABLE t7 (id INTEGER);
CREATE TABLE t8 (id INTEGER);
CREATE TABLE t9 (id INTEGER);
CREATE TABLE t10 (id INTEGER);
CREATE TABLE t11 (id INTEGER);
CREATE TABLE t12 (id INTEGER);
CREATE TABLE t13 (id INTEGER);
CREATE TABLE t14 (id INTEGER);
CREATE TABLE t15 (id INTEGER);
CREATE TABLE t16 (id INTEGER);
CREATE TABLE t17 (id INTEGER);
CREATE TABLE t18 (id INTEGER);
CREATE TABLE t19 (id INTEGER);
CREATE TABLE t20 (id INTEGER);
CREATE TABLE t21 (id INTEGER);
CREATE TABLE t22 (id INTEGER);
CREATE TABLE t23 (id INTEGER);
CREATE TABLE t24 (id INTEGER);
CREATE TABLE t25 (id INTEGER);
CREATE TABLE t26 (id INTEGER);

-- List all tables (should show many)
.tables

-- Try to exceed maximum (might fail with "Too many tables")
CREATE TABLE t27 (id INTEGER);

-- ============================================================================
-- TEST 7: PRAGMAs
-- ============================================================================

-- Test PRAGMA statements (should be accepted even if not fully implemented)
PRAGMA cache_size=100;
PRAGMA page_size=512;

-- ============================================================================
-- TEST 8: Cleanup - Drop all test tables
-- ============================================================================

DROP TABLE test_table;
DROP TABLE users;
DROP TABLE products;
DROP TABLE orders;
DROP TABLE committed_table;
DROP TABLE t1;
DROP TABLE t2;
DROP TABLE t3;
DROP TABLE t4;
DROP TABLE t5;
DROP TABLE t6;
DROP TABLE t7;
DROP TABLE t8;
DROP TABLE t9;
DROP TABLE t10;
DROP TABLE t11;
DROP TABLE t12;
DROP TABLE t13;
DROP TABLE t14;
DROP TABLE t15;
DROP TABLE t16;
DROP TABLE t17;
DROP TABLE t18;
DROP TABLE t19;
DROP TABLE t20;
DROP TABLE t21;
DROP TABLE t22;
DROP TABLE t23;
DROP TABLE t24;
DROP TABLE t25;
DROP TABLE t26;
DROP TABLE t27;

-- Verify database is empty
.tables

-- ============================================================================
-- TEST SUMMARY
-- ============================================================================
-- If all tests executed without crashing:
-- - CREATE TABLE: ✅ PASS
-- - DROP TABLE: ✅ PASS
-- - .tables command: ✅ PASS
-- - .schema command: ✅ PASS
-- - Transaction support: ✅ PASS
-- - Error handling: ✅ PASS
-- - Maximum tables: ✅ PASS (or appropriate error)
-- - PRAGMA support: ✅ PASS
--
-- Overall Status: ✅ PASS if no crashes, errors as expected
--
-- Exit the shell
.quit
