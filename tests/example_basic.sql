-- Example: Basic SQLite operations for RISC OS
-- This script demonstrates the current capabilities of SQLite on RISC OS 3.1

-- Create a simple table
CREATE TABLE users (id INTEGER, name TEXT, email TEXT);

-- Display the tables
-- This command can be run with: .tables

-- Display schema
-- This command can be run with: .schema users

-- Create another table
CREATE TABLE products (id INTEGER, name TEXT, price INTEGER);

-- Show all tables
-- Type: .tables

-- Show specific table schema
-- Type: .schema products

-- Display help
-- Type: .help

-- Transaction support
BEGIN;

-- Multiple statements in transaction
-- Additional operations would go here

COMMIT;

-- Exit when done
-- Type: .quit
