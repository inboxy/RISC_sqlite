/*
** Test program for SELECT functionality
*/

#include <stdio.h>
#include <string.h>
#include "sqlite/sqlite.h"

/* Callback function to print results */
int print_callback(void *arg, int argc, char **argv, char **col_names)
{
    int i;

    /* Print column names on first call */
    if (*(int*)arg == 0) {
        for (i = 0; i < argc; i++) {
            printf("%s", col_names[i]);
            if (i < argc - 1) printf("|");
        }
        printf("\n");
        *(int*)arg = 1;
    }

    /* Print row values */
    for (i = 0; i < argc; i++) {
        printf("%s", argv[i] ? argv[i] : "NULL");
        if (i < argc - 1) printf("|");
    }
    printf("\n");

    return 0;
}

int main(void)
{
    sqlite *db;
    char *errmsg = NULL;
    int first_row = 0;
    int rc;

    printf("Testing SQLite SELECT implementation...\n\n");

    /* Open database */
    db = sqlite_open("test_select.db", 0, &errmsg);
    if (!db) {
        printf("ERROR: Cannot open database: %s\n", errmsg ? errmsg : "unknown error");
        return 1;
    }

    printf("Database opened successfully\n\n");

    /* Create table */
    printf("Creating table 'users'...\n");
    rc = sqlite_exec(db, "CREATE TABLE users", NULL, NULL, &errmsg);
    if (rc != SQLITE_OK) {
        printf("ERROR: %s\n", errmsg ? errmsg : "unknown error");
        sqlite_close(db);
        return 1;
    }
    printf("Table created\n\n");

    /* Insert data */
    printf("Inserting data...\n");
    rc = sqlite_exec(db, "INSERT INTO users VALUES ('Alice')", NULL, NULL, &errmsg);
    if (rc != SQLITE_OK) {
        printf("ERROR: %s\n", errmsg ? errmsg : "unknown error");
        sqlite_close(db);
        return 1;
    }

    rc = sqlite_exec(db, "INSERT INTO users VALUES ('Bob')", NULL, NULL, &errmsg);
    if (rc != SQLITE_OK) {
        printf("ERROR: %s\n", errmsg ? errmsg : "unknown error");
        sqlite_close(db);
        return 1;
    }

    rc = sqlite_exec(db, "INSERT INTO users VALUES ('Charlie')", NULL, NULL, &errmsg);
    if (rc != SQLITE_OK) {
        printf("ERROR: %s\n", errmsg ? errmsg : "unknown error");
        sqlite_close(db);
        return 1;
    }
    printf("Data inserted\n\n");

    /* Select data */
    printf("Selecting data:\n");
    first_row = 0;
    rc = sqlite_exec(db, "SELECT * FROM users", print_callback, &first_row, &errmsg);
    if (rc != SQLITE_OK) {
        printf("ERROR: %s\n", errmsg ? errmsg : "unknown error");
        sqlite_close(db);
        return 1;
    }

    printf("\nSELECT test completed successfully!\n");

    /* Close database */
    sqlite_close(db);

    return 0;
}
