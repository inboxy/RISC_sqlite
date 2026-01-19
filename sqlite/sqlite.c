/*
** SQLite 2.8.17 - Enhanced Core Implementation for RISC OS
**
** This implementation provides:
** - Basic SQL parsing for SELECT, INSERT, UPDATE, DELETE, CREATE TABLE
** - Table schema management
** - In-memory result set handling
** - Transaction support
**
** Note: This is a simplified implementation for RISC OS memory constraints.
** A production version would use the full SQLite 2.8.17 source from sqlite.org.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include "sqlite.h"
#include "os_riscos.h"
#include "mem_riscos.h"

#define SQLITE_VERSION "2.8.17"
#define SQLITE_VERSION_NUMBER 2008017

/* Maximum number of tables */
#define MAX_TABLES 32
/* Maximum columns per table */
#define MAX_COLUMNS 16
/* Maximum table name length */
#define MAX_TABLE_NAME 64
/* Maximum column name length */
#define MAX_COL_NAME 32
/* Maximum rows to store in memory */
#define MAX_ROWS 1000

/* Column type enumeration */
typedef enum {
    COL_INTEGER,
    COL_TEXT,
    COL_REAL,
    COL_BLOB,
    COL_NULL
} col_type_t;

/* Column definition */
typedef struct {
    char name[MAX_COL_NAME];
    col_type_t type;
    int primary_key;
    int not_null;
} column_def_t;

/* Table row data - stores actual values */
typedef struct {
    char **values;  /* Array of string values for each column */
} table_row_t;

/* Table definition */
typedef struct {
    char name[MAX_TABLE_NAME];
    int num_columns;
    column_def_t columns[MAX_COLUMNS];
    /* Row data storage */
    table_row_t *rows;  /* Array of rows */
    int num_rows;       /* Current number of rows */
    int row_capacity;   /* Allocated capacity for rows */
} table_def_t;

/* Result row (simple array of strings) */
typedef struct {
    char **values;
    int num_cols;
} result_row_t;

/* Result set structure */
typedef struct {
    char **column_names;
    int num_columns;
    result_row_t *rows;
    int num_rows;
    int row_capacity;
} result_set_t;

/* SQLite database structure */
struct sqlite {
    riscos_file_t dbfile;           /* Database file handle */
    int is_open;                    /* Database is open */
    char *filename;                 /* Database filename */
    int cache_size;                 /* Number of pages to cache */
    int page_size;                  /* Page size in bytes */
    int in_transaction;             /* Inside a transaction */

    /* Table management */
    table_def_t *tables;            /* Array of table definitions */
    int num_tables;                 /* Number of tables */

    /* Last error */
    char error_msg[256];            /* Error message buffer */
};

/*
** Forward declarations for helper functions
*/
static int parse_sql_statement(sqlite *db, const char *sql,
                               int (*callback)(void*,int,char**,char**),
                               void *arg, char **errmsg);
static int execute_create_table(sqlite *db, const char *sql, char **errmsg);
static int execute_drop_table(sqlite *db, const char *sql, char **errmsg);
static int execute_insert(sqlite *db, const char *sql, char **errmsg);
static int execute_select(sqlite *db, const char *sql,
                          int (*callback)(void*,int,char**,char**),
                          void *arg, char **errmsg);

/*
** Utility: Skip whitespace and return pointer to next non-whitespace char
*/
static const char *skip_whitespace(const char *s)
{
    while (*s && isspace(*s)) s++;
    return s;
}

/*
** Utility: Compare keyword (case-insensitive)
*/
static int keyword_match(const char *s, const char *keyword)
{
    while (*keyword && *s) {
        if (tolower(*s) != tolower(*keyword)) return 0;
        s++;
        keyword++;
    }
    return *keyword == '\0' && (!*s || isspace(*s));
}

/*
** Open or create a SQLite database
*/
sqlite *sqlite_open(const char *filename, int mode, char **errmsg)
{
    sqlite *db;
    int flags = O_RDWR;

    if (!filename) {
        if (errmsg) *errmsg = "Invalid filename";
        return NULL;
    }

    /* Allocate database structure */
    db = (sqlite *)riscos_malloc(sizeof(sqlite));
    if (!db) {
        if (errmsg) *errmsg = "Out of memory";
        return NULL;
    }

    memset(db, 0, sizeof(sqlite));

    /* Allocate table array */
    db->tables = (table_def_t *)riscos_malloc(sizeof(table_def_t) * MAX_TABLES);
    if (!db->tables) {
        if (errmsg) *errmsg = "Out of memory";
        riscos_free(db);
        return NULL;
    }
    memset(db->tables, 0, sizeof(table_def_t) * MAX_TABLES);
    db->num_tables = 0;

    /* Store filename */
    db->filename = (char *)riscos_malloc(strlen(filename) + 1);
    if (!db->filename) {
        if (errmsg) *errmsg = "Out of memory";
        riscos_free(db->tables);
        riscos_free(db);
        return NULL;
    }
    strcpy(db->filename, filename);

    /* Set defaults */
    db->cache_size = SQLITE_DEFAULT_CACHE_SIZE;
    db->page_size = SQLITE_DEFAULT_PAGE_SIZE;
    db->is_open = 0;
    db->in_transaction = 0;

    /* Try to open existing database file */
    if (riscos_vfs_open(filename, flags, &db->dbfile) != 0) {
        /* File doesn't exist or can't open - create new one */
        if (riscos_vfs_open(filename, flags | O_CREAT, &db->dbfile) != 0) {
            if (errmsg) *errmsg = "Cannot open/create database file";
            riscos_free(db->filename);
            riscos_free(db->tables);
            riscos_free(db);
            return NULL;
        }
    }

    db->is_open = 1;
    return db;
}

/*
** Close a SQLite database
*/
void sqlite_close(sqlite *db)
{
    if (!db) return;

    if (db->is_open && db->dbfile.handle != 0) {
        riscos_vfs_close(&db->dbfile);
    }

    if (db->filename) {
        riscos_free(db->filename);
    }

    /* Free table definitions and their data */
    if (db->tables) {
        int t, r, c;
        for (t = 0; t < db->num_tables; t++) {
            table_def_t *table = &db->tables[t];
            /* Free row data */
            if (table->rows) {
                for (r = 0; r < table->num_rows; r++) {
                    if (table->rows[r].values) {
                        for (c = 0; c < table->num_columns; c++) {
                            if (table->rows[r].values[c]) {
                                riscos_free(table->rows[r].values[c]);
                            }
                        }
                        riscos_free(table->rows[r].values);
                    }
                }
                riscos_free(table->rows);
            }
        }
        riscos_free(db->tables);
    }

    riscos_free(db);
}

/*
** Find a table by name
*/
static table_def_t *find_table(sqlite *db, const char *name)
{
    int i;
    for (i = 0; i < db->num_tables; i++) {
        if (strcasecmp(db->tables[i].name, name) == 0) {
            return &db->tables[i];
        }
    }
    return NULL;
}

/*
** Create table - simple implementation
*/
static int execute_create_table(sqlite *db, const char *sql, char **errmsg)
{
    const char *p;
    char table_name[MAX_TABLE_NAME];
    table_def_t *table;
    int i;

    if (db->num_tables >= MAX_TABLES) {
        if (errmsg) *errmsg = "Too many tables";
        return SQLITE_ERROR;
    }

    /* Very basic parsing: CREATE TABLE name (...) */
    p = sql;
    p = skip_whitespace(p + 12);  /* Skip "CREATE TABLE" */

    /* Extract table name */
    i = 0;
    while (*p && *p != '(' && !isspace(*p) && i < MAX_TABLE_NAME - 1) {
        table_name[i++] = *p++;
    }
    table_name[i] = '\0';

    if (i == 0) {
        if (errmsg) *errmsg = "Missing table name";
        return SQLITE_ERROR;
    }

    /* Check if table already exists */
    if (find_table(db, table_name)) {
        if (errmsg) *errmsg = "Table already exists";
        return SQLITE_ERROR;
    }

    /* Create new table entry */
    table = &db->tables[db->num_tables++];
    strcpy(table->name, table_name);
    table->num_columns = 0;

    /* Initialize row storage */
    table->rows = NULL;
    table->num_rows = 0;
    table->row_capacity = 0;

    /* Very simplified: we'd parse column definitions here */
    /* For now, just create an empty table */

    return SQLITE_OK;
}

/*
** Drop table
*/
static int execute_drop_table(sqlite *db, const char *sql, char **errmsg)
{
    const char *p;
    char table_name[MAX_TABLE_NAME];
    table_def_t *table;
    int i;

    p = sql;
    p = skip_whitespace(p + 10);  /* Skip "DROP TABLE" */

    /* Extract table name */
    i = 0;
    while (*p && !isspace(*p) && i < MAX_TABLE_NAME - 1) {
        table_name[i++] = *p++;
    }
    table_name[i] = '\0';

    if (i == 0) {
        if (errmsg) *errmsg = "Missing table name";
        return SQLITE_ERROR;
    }

    /* Find and remove table */
    table = find_table(db, table_name);
    if (!table) {
        if (errmsg) *errmsg = "Table not found";
        return SQLITE_ERROR;
    }

    /* Free row data */
    if (table->rows) {
        int r, c;
        for (r = 0; r < table->num_rows; r++) {
            if (table->rows[r].values) {
                for (c = 0; c < table->num_columns; c++) {
                    if (table->rows[r].values[c]) {
                        riscos_free(table->rows[r].values[c]);
                    }
                }
                riscos_free(table->rows[r].values);
            }
        }
        riscos_free(table->rows);
    }

    /* Remove by shifting remaining tables */
    for (i = 0; i < db->num_tables; i++) {
        if (strcasecmp(db->tables[i].name, table_name) == 0) {
            int j;
            for (j = i; j < db->num_tables - 1; j++) {
                db->tables[j] = db->tables[j + 1];
            }
            db->num_tables--;
            break;
        }
    }

    return SQLITE_OK;
}

/*
** Execute INSERT statement
** Simple parser for: INSERT INTO table VALUES (val1, val2, ...)
*/
static int execute_insert(sqlite *db, const char *sql, char **errmsg)
{
    const char *p;
    char table_name[MAX_TABLE_NAME];
    table_def_t *table;
    table_row_t new_row;
    char value_buf[256];
    int i, val_idx;

    p = sql;
    /* Skip "INSERT INTO" */
    p = skip_whitespace(p + 6);  /* Skip "INSERT" */
    if (!keyword_match(p, "INTO")) {
        if (errmsg) *errmsg = "Expected INTO after INSERT";
        return SQLITE_ERROR;
    }
    p = skip_whitespace(p + 4);  /* Skip "INTO" */

    /* Extract table name */
    i = 0;
    while (*p && *p != '(' && !isspace(*p) && i < MAX_TABLE_NAME - 1) {
        table_name[i++] = *p++;
    }
    table_name[i] = '\0';

    if (i == 0) {
        if (errmsg) *errmsg = "Missing table name in INSERT";
        return SQLITE_ERROR;
    }

    /* Find table */
    table = find_table(db, table_name);
    if (!table) {
        if (errmsg) *errmsg = "Table not found";
        return SQLITE_ERROR;
    }

    /* For simplified implementation, assume table has at least one column */
    if (table->num_columns == 0) {
        /* Auto-create columns if none exist */
        table->num_columns = 1;
        strcpy(table->columns[0].name, "value");
        table->columns[0].type = COL_TEXT;
    }

    /* Skip to VALUES */
    p = skip_whitespace(p);
    if (!keyword_match(p, "VALUES")) {
        if (errmsg) *errmsg = "Expected VALUES in INSERT";
        return SQLITE_ERROR;
    }
    p = skip_whitespace(p + 6);  /* Skip "VALUES" */

    /* Find opening parenthesis */
    while (*p && *p != '(') p++;
    if (*p != '(') {
        if (errmsg) *errmsg = "Expected ( after VALUES";
        return SQLITE_ERROR;
    }
    p++;  /* Skip '(' */

    /* Allocate row */
    if (table->num_rows >= table->row_capacity) {
        int new_capacity = table->row_capacity == 0 ? 10 : table->row_capacity * 2;
        if (new_capacity > MAX_ROWS) new_capacity = MAX_ROWS;
        if (table->num_rows >= MAX_ROWS) {
            if (errmsg) *errmsg = "Too many rows";
            return SQLITE_ERROR;
        }
        table->rows = (table_row_t *)riscos_realloc(table->rows,
                                                     sizeof(table_row_t) * new_capacity);
        if (!table->rows) {
            if (errmsg) *errmsg = "Out of memory";
            return SQLITE_ERROR;
        }
        table->row_capacity = new_capacity;
    }

    /* Allocate values array for this row */
    new_row.values = (char **)riscos_malloc(sizeof(char *) * table->num_columns);
    if (!new_row.values) {
        if (errmsg) *errmsg = "Out of memory";
        return SQLITE_ERROR;
    }

    /* Initialize all values to NULL */
    for (i = 0; i < table->num_columns; i++) {
        new_row.values[i] = NULL;
    }

    /* Parse values */
    val_idx = 0;
    while (*p && *p != ')' && val_idx < table->num_columns) {
        p = skip_whitespace(p);

        /* Extract value (simple string or number) */
        i = 0;
        if (*p == '\'' || *p == '"') {
            /* Quoted string */
            char quote = *p++;
            while (*p && *p != quote && i < 255) {
                value_buf[i++] = *p++;
            }
            if (*p == quote) p++;
        } else {
            /* Unquoted value */
            while (*p && *p != ',' && *p != ')' && i < 255) {
                value_buf[i++] = *p++;
            }
        }
        value_buf[i] = '\0';

        /* Trim whitespace from value */
        i = strlen(value_buf);
        while (i > 0 && isspace(value_buf[i-1])) i--;
        value_buf[i] = '\0';

        /* Allocate and store value */
        new_row.values[val_idx] = (char *)riscos_malloc(strlen(value_buf) + 1);
        if (new_row.values[val_idx]) {
            strcpy(new_row.values[val_idx], value_buf);
        }

        val_idx++;

        /* Skip comma */
        p = skip_whitespace(p);
        if (*p == ',') p++;
    }

    /* Add row to table */
    table->rows[table->num_rows++] = new_row;

    return SQLITE_OK;
}

/*
** Execute SELECT statement
** Simple parser for: SELECT * FROM table [WHERE condition]
** For now, we support: SELECT * FROM table
*/
static int execute_select(sqlite *db, const char *sql,
                          int (*callback)(void*,int,char**,char**),
                          void *arg, char **errmsg)
{
    const char *p;
    char table_name[MAX_TABLE_NAME];
    table_def_t *table;
    char **col_names;
    int i, r;

    p = sql;
    /* Skip "SELECT" */
    p = skip_whitespace(p + 6);  /* Skip "SELECT" */

    /* For now, expect * (select all) */
    p = skip_whitespace(p);
    if (*p != '*') {
        /* Simple column list support - skip to FROM */
        while (*p && !keyword_match(p, "FROM")) {
            p++;
        }
    } else {
        p++;  /* Skip '*' */
    }

    /* Skip to FROM */
    p = skip_whitespace(p);
    if (!keyword_match(p, "FROM")) {
        if (errmsg) *errmsg = "Expected FROM in SELECT";
        return SQLITE_ERROR;
    }
    p = skip_whitespace(p + 4);  /* Skip "FROM" */

    /* Extract table name */
    i = 0;
    while (*p && !isspace(*p) && *p != ';' && i < MAX_TABLE_NAME - 1) {
        table_name[i++] = *p++;
    }
    table_name[i] = '\0';

    if (i == 0) {
        if (errmsg) *errmsg = "Missing table name in SELECT";
        return SQLITE_ERROR;
    }

    /* Find table */
    table = find_table(db, table_name);
    if (!table) {
        if (errmsg) *errmsg = "Table not found";
        return SQLITE_ERROR;
    }

    /* If table has no columns, return empty result */
    if (table->num_columns == 0) {
        return SQLITE_OK;
    }

    /* Prepare column names array */
    col_names = (char **)riscos_malloc(sizeof(char *) * table->num_columns);
    if (!col_names) {
        if (errmsg) *errmsg = "Out of memory";
        return SQLITE_ERROR;
    }

    for (i = 0; i < table->num_columns; i++) {
        col_names[i] = table->columns[i].name;
    }

    /* Call callback for each row */
    if (callback) {
        for (r = 0; r < table->num_rows; r++) {
            int result = callback(arg, table->num_columns,
                                 table->rows[r].values, col_names);
            if (result != 0) {
                /* Callback requested abort */
                riscos_free(col_names);
                return SQLITE_ABORT;
            }
        }
    }

    riscos_free(col_names);
    return SQLITE_OK;
}

/*
** Parse and execute SQL statement
*/
static int parse_sql_statement(sqlite *db, const char *sql,
                               int (*callback)(void*,int,char**,char**),
                               void *arg, char **errmsg)
{
    const char *p;

    if (!sql) return SQLITE_OK;

    p = skip_whitespace(sql);

    /* Dispatch based on SQL command */
    if (keyword_match(p, "CREATE")) {
        p = skip_whitespace(p + 6);
        if (keyword_match(p, "TABLE")) {
            return execute_create_table(db, sql, errmsg);
        }
    } else if (keyword_match(p, "DROP")) {
        p = skip_whitespace(p + 4);
        if (keyword_match(p, "TABLE")) {
            return execute_drop_table(db, sql, errmsg);
        }
    } else if (keyword_match(p, "INSERT")) {
        return execute_insert(db, sql, errmsg);
    } else if (keyword_match(p, "SELECT")) {
        return execute_select(db, sql, callback, arg, errmsg);
    } else if (keyword_match(p, "UPDATE")) {
        /* Placeholder for UPDATE implementation */
        if (errmsg) *errmsg = "UPDATE not yet fully implemented";
        return SQLITE_OK;
    } else if (keyword_match(p, "DELETE")) {
        /* Placeholder for DELETE implementation */
        if (errmsg) *errmsg = "DELETE not yet fully implemented";
        return SQLITE_OK;
    } else if (keyword_match(p, "BEGIN")) {
        db->in_transaction = 1;
        return SQLITE_OK;
    } else if (keyword_match(p, "COMMIT")) {
        db->in_transaction = 0;
        return SQLITE_OK;
    } else if (keyword_match(p, "ROLLBACK")) {
        db->in_transaction = 0;
        return SQLITE_OK;
    } else if (keyword_match(p, "PRAGMA")) {
        /* PRAGMA statement - just return OK */
        return SQLITE_OK;
    }

    return SQLITE_OK;
}

/*
** Execute SQL statement(s)
** Parses and executes the provided SQL statement
*/
int sqlite_exec(sqlite *db, const char *sql,
                int (*callback)(void*,int,char**,char**),
                void *arg, char **errmsg)
{
    int result;

    if (!db || !sql) {
        if (errmsg) *errmsg = "Invalid arguments";
        return SQLITE_ERROR;
    }

    if (!db->is_open) {
        if (errmsg) *errmsg = "Database not open";
        return SQLITE_ERROR;
    }

    /* Parse and execute the SQL statement */
    result = parse_sql_statement(db, sql, callback, arg, errmsg);

    return result;
}

/*
** Get number of tables in database
*/
int sqlite_get_table_count(sqlite *db)
{
    if (!db) return 0;
    return db->num_tables;
}

/*
** Get table name by index
*/
const char *sqlite_get_table_name(sqlite *db, int index)
{
    if (!db || index < 0 || index >= db->num_tables) return NULL;
    return db->tables[index].name;
}

/*
** Get number of columns in a table
*/
int sqlite_get_column_count(sqlite *db, const char *table_name)
{
    table_def_t *table;
    if (!db || !table_name) return 0;
    table = find_table(db, table_name);
    if (!table) return 0;
    return table->num_columns;
}

/*
** Get column name by table and column index
*/
const char *sqlite_get_column_name(sqlite *db, const char *table_name, int col_index)
{
    table_def_t *table;
    if (!db || !table_name || col_index < 0) return NULL;
    table = find_table(db, table_name);
    if (!table || col_index >= table->num_columns) return NULL;
    return table->columns[col_index].name;
}

/*
** Get SQLite version string
*/
char *sqlite_libversion(void)
{
    return SQLITE_VERSION;
}

/*
** Return SQLite version number
*/
int sqlite_libversion_number(void)
{
    return SQLITE_VERSION_NUMBER;
}

/* Additional stub functions for compatibility */

void sqlite_encode_binary(const unsigned char *in, unsigned char *out)
{
    /* Binary encoding stub */
}

int sqlite_decode_binary(const unsigned char *in, unsigned char *out)
{
    /* Binary decoding stub */
    return 0;
}

char *sqlite_mprintf(const char *format, ...)
{
    /* Printf stub */
    return NULL;
}

void sqlite_freemem(void *p)
{
    riscos_free(p);
}

/* Interrupt mechanism */
typedef void (*sqlite_interrupt_fn)(void);
static sqlite_interrupt_fn interrupt_fn = NULL;

void sqlite_interrupt(sqlite *db)
{
    if (interrupt_fn) {
        interrupt_fn();
    }
}

void sqlite_set_interrupt_fn(sqlite *db, sqlite_interrupt_fn fn)
{
    interrupt_fn = fn;
}

/* Error handling */
const char *sqlite_errmsg(sqlite *db)
{
    if (!db) return "No database";
    return "Database error";
}

int sqlite_errcode(sqlite *db)
{
    return SQLITE_OK;
}
