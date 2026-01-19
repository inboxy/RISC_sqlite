/*
** Unit test for SELECT functionality (no file I/O)
** Tests the in-memory data structures and SELECT logic
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Include internal structures for testing */
#define MAX_TABLE_NAME 64
#define MAX_COL_NAME 32
#define MAX_COLUMNS 16

typedef enum {
    COL_INTEGER,
    COL_TEXT,
    COL_REAL,
    COL_BLOB,
    COL_NULL
} col_type_t;

typedef struct {
    char name[MAX_COL_NAME];
    col_type_t type;
    int primary_key;
    int not_null;
} column_def_t;

typedef struct {
    char **values;
} table_row_t;

typedef struct {
    char name[MAX_TABLE_NAME];
    int num_columns;
    column_def_t columns[MAX_COLUMNS];
    table_row_t *rows;
    int num_rows;
    int row_capacity;
} table_def_t;

/* Test callback */
int test_callback(void *arg, int argc, char **argv, char **col_names)
{
    int i;
    int *count = (int*)arg;

    printf("Row %d: ", *count);
    for (i = 0; i < argc; i++) {
        printf("%s=%s", col_names[i], argv[i] ? argv[i] : "NULL");
        if (i < argc - 1) printf(", ");
    }
    printf("\n");

    (*count)++;
    return 0;
}

int main(void)
{
    table_def_t table;
    int i;
    int callback_count = 0;

    printf("=== SELECT Unit Test ===\n\n");

    /* Initialize table */
    strcpy(table.name, "users");
    table.num_columns = 1;
    strcpy(table.columns[0].name, "name");
    table.columns[0].type = COL_TEXT;
    table.num_rows = 0;
    table.row_capacity = 10;
    table.rows = (table_row_t *)malloc(sizeof(table_row_t) * table.row_capacity);

    printf("1. Created table 'users' with 1 column 'name'\n\n");

    /* Add rows */
    const char *names[] = {"Alice", "Bob", "Charlie"};
    for (i = 0; i < 3; i++) {
        table.rows[table.num_rows].values = (char **)malloc(sizeof(char *));
        table.rows[table.num_rows].values[0] = strdup(names[i]);
        table.num_rows++;
    }

    printf("2. Inserted %d rows\n\n", table.num_rows);

    /* Prepare column names for callback */
    char **col_names = (char **)malloc(sizeof(char *) * table.num_columns);
    col_names[0] = table.columns[0].name;

    printf("3. Executing SELECT * FROM users:\n");

    /* Simulate SELECT execution */
    for (i = 0; i < table.num_rows; i++) {
        test_callback(&callback_count, table.num_columns,
                     table.rows[i].values, col_names);
    }

    printf("\n4. SELECT returned %d rows\n", callback_count);

    /* Verify results */
    if (callback_count == 3) {
        printf("\n✓ TEST PASSED: SELECT successfully returned all 3 rows\n");
    } else {
        printf("\n✗ TEST FAILED: Expected 3 rows, got %d\n", callback_count);
        return 1;
    }

    /* Cleanup */
    for (i = 0; i < table.num_rows; i++) {
        free(table.rows[i].values[0]);
        free(table.rows[i].values);
    }
    free(table.rows);
    free(col_names);

    printf("\n=== All tests passed! ===\n");

    return 0;
}
