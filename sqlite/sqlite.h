/*
** SQLite 2.8.17 - Minimal header for RISC OS port
** This is a stub for cross-platform compatibility
*/

#ifndef _SQLITE_H_
#define _SQLITE_H_

#include <stdio.h>
#include <stdlib.h>

typedef struct sqlite sqlite;

/* Function prototypes - Database Management */
sqlite *sqlite_open(const char *filename, int mode, char **errmsg);
void sqlite_close(sqlite *db);
int sqlite_exec(sqlite *db, const char *sql, int (*callback)(void*,int,char**,char**),
                void *arg, char **errmsg);

/* Function prototypes - Table Information */
int sqlite_get_table_count(sqlite *db);
const char *sqlite_get_table_name(sqlite *db, int index);
int sqlite_get_column_count(sqlite *db, const char *table_name);
const char *sqlite_get_column_name(sqlite *db, const char *table_name, int col_index);

/* Function prototypes - Utility */
char *sqlite_libversion(void);

/* Error codes */
#define SQLITE_OK           0
#define SQLITE_ERROR        1
#define SQLITE_INTERNAL     2
#define SQLITE_PERM         3
#define SQLITE_ABORT        4
#define SQLITE_BUSY         5
#define SQLITE_LOCKED       6
#define SQLITE_NOMEM        7
#define SQLITE_READONLY     8
#define SQLITE_IOERR        9

/* Configuration constants */
#define SQLITE_DEFAULT_CACHE_SIZE 100
#define SQLITE_DEFAULT_PAGE_SIZE 512
#define SQLITE_MAX_LENGTH 1048576
#define SQLITE_MAX_SQL_LENGTH 10000

#endif /* _SQLITE_H_ */
