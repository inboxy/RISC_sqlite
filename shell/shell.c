/*
** SQLite Interactive Shell for RISC OS
**
** Provides a command-line interface for SQLite operations.
** Similar to the sqlite command-line tool, but optimized for RISC OS environment.
**
** Features:
** - Interactive SQL prompt
** - Dot commands (.tables, .schema, .help, etc.)
** - Result formatting and display
** - Error handling and reporting
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../sqlite/sqlite.h"
#include "../sqlite/mem_riscos.h"

/* Configuration constants */
#define MAX_SQL_BUFFER      10000
#define MAX_RESULT_WIDTH    80
#define MAX_COLUMNS         32
#define PROMPT              "sqlite> "
#define CONTINUE_PROMPT     "   ...> "

/* Global state */
typedef struct {
    sqlite *db;                     /* Current database */
    char sql_buffer[MAX_SQL_BUFFER];  /* SQL accumulation buffer */
    int sql_length;                 /* Current SQL buffer length */
    int in_statement;               /* Still reading SQL statement */
    int verbose;                    /* Verbose output mode */
    int headers;                    /* Show column headers */
} shell_state_t;

static shell_state_t shell = {NULL, "", 0, 0, 0, 1};

/*
** Forward declarations
*/
static int exec_dot_command(const char *line);
static int exec_sql(const char *sql);
static void print_banner(void);
static void print_help(void);

/*
** Callback for SQL result rows
** Called once for each row of results
*/
static int result_callback(void *arg, int argc, char **argv, char **colname)
{
    int i;

    /* Print column headers on first row if enabled */
    if (*(int*)arg == 0 && shell.headers) {
        for (i = 0; i < argc; i++) {
            if (i > 0) printf(" | ");
            printf("%s", colname[i]);
        }
        printf("\n");

        /* Print separator */
        for (i = 0; i < argc; i++) {
            if (i > 0) printf("-+-");
            int len = strlen(colname[i]);
            for (int j = 0; j < len; j++) printf("-");
        }
        printf("\n");

        *(int*)arg = 1;  /* Headers printed */
    }

    /* Print row data */
    for (i = 0; i < argc; i++) {
        if (i > 0) printf(" | ");
        printf("%s", argv[i] ? argv[i] : "NULL");
    }
    printf("\n");

    return 0;
}

/*
** Open a database file
*/
static int cmd_open(const char *filename)
{
    char *errmsg = NULL;

    if (!filename || strlen(filename) == 0) {
        fprintf(stderr, "Usage: .open <filename>\n");
        return -1;
    }

    if (shell.db) {
        sqlite_close(shell.db);
    }

    shell.db = sqlite_open(filename, 0, &errmsg);

    if (!shell.db) {
        fprintf(stderr, "Cannot open database '%s': %s\n", filename,
                errmsg ? errmsg : "Unknown error");
        return -1;
    }

    printf("Opened database '%s'\n", filename);
    return 0;
}

/*
** List tables in database
*/
static int cmd_tables(void)
{
    int i;
    int count;

    if (!shell.db) {
        fprintf(stderr, "No database open\n");
        return -1;
    }

    count = sqlite_get_table_count(shell.db);

    if (count == 0) {
        printf("(No tables)\n");
    } else {
        printf("Tables:\n");
        for (i = 0; i < count; i++) {
            const char *name = sqlite_get_table_name(shell.db, i);
            if (name) {
                printf("  %s\n", name);
            }
        }
    }

    return 0;
}

/*
** Show table schema
*/
static int cmd_schema(const char *table)
{
    int i;
    int col_count;
    const char *col_name;

    if (!shell.db) {
        fprintf(stderr, "No database open\n");
        return -1;
    }

    if (!table || strlen(table) == 0) {
        /* Show all table schemas */
        int table_count = sqlite_get_table_count(shell.db);
        for (i = 0; i < table_count; i++) {
            const char *table_name = sqlite_get_table_name(shell.db, i);
            if (table_name) {
                printf("Table: %s\n", table_name);
                col_count = sqlite_get_column_count(shell.db, table_name);
                if (col_count == 0) {
                    printf("  (no columns defined)\n");
                }
            }
        }
    } else {
        /* Show specific table schema */
        printf("Table: %s\n", table);
        col_count = sqlite_get_column_count(shell.db, table);

        if (col_count == 0) {
            printf("  (table not found or no columns)\n");
        } else {
            for (i = 0; i < col_count; i++) {
                col_name = sqlite_get_column_name(shell.db, table, i);
                if (col_name) {
                    printf("  %s\n", col_name);
                }
            }
        }
    }

    return 0;
}

/*
** Execute a dot command
*/
static int exec_dot_command(const char *line)
{
    const char *cmd = line + 1;  /* Skip the dot */
    const char *args;
    char cmd_buf[32];
    int cmd_len;

    /* Find end of command name */
    args = cmd;
    while (*args && !isspace(*args)) args++;
    cmd_len = args - cmd;

    if (cmd_len >= sizeof(cmd_buf)) cmd_len = sizeof(cmd_buf) - 1;

    strncpy(cmd_buf, cmd, cmd_len);
    cmd_buf[cmd_len] = '\0';

    /* Skip whitespace after command */
    while (*args && isspace(*args)) args++;

    /* Dispatch to command handler */
    if (strcmp(cmd_buf, "quit") == 0 || strcmp(cmd_buf, "exit") == 0) {
        return 1;  /* Signal to exit */
    } else if (strcmp(cmd_buf, "help") == 0) {
        print_help();
        return 0;
    } else if (strcmp(cmd_buf, "open") == 0) {
        return cmd_open(args);
    } else if (strcmp(cmd_buf, "tables") == 0) {
        return cmd_tables();
    } else if (strcmp(cmd_buf, "schema") == 0) {
        return cmd_schema(args);
    } else if (strcmp(cmd_buf, "verbose") == 0) {
        if (strlen(args) > 0) {
            if (strcasecmp(args, "on") == 0) {
                shell.verbose = 1;
            } else if (strcasecmp(args, "off") == 0) {
                shell.verbose = 0;
            } else {
                printf("Usage: .verbose [on|off]\n");
                return 0;
            }
        } else {
            shell.verbose = !shell.verbose;
        }
        printf("Verbose mode %s\n", shell.verbose ? "on" : "off");
        return 0;
    } else if (strcmp(cmd_buf, "headers") == 0) {
        if (strlen(args) > 0) {
            if (strcasecmp(args, "on") == 0) {
                shell.headers = 1;
            } else if (strcasecmp(args, "off") == 0) {
                shell.headers = 0;
            } else {
                printf("Usage: .headers [on|off]\n");
                return 0;
            }
        } else {
            shell.headers = !shell.headers;
        }
        printf("Headers %s\n", shell.headers ? "on" : "off");
        return 0;
    } else {
        fprintf(stderr, "Unknown command: .%s\n", cmd_buf);
        fprintf(stderr, "Type .help for available commands\n");
        return 0;
    }
}

/*
** Execute SQL statement
*/
static int exec_sql(const char *sql)
{
    char *errmsg = NULL;
    int result = 0;
    int headers_printed = 0;

    if (!shell.db) {
        fprintf(stderr, "No database open\n");
        return -1;
    }

    if (!sql || strlen(sql) == 0) {
        return 0;
    }

    if (shell.verbose) {
        printf("Executing: %s\n", sql);
    }

    result = sqlite_exec(shell.db, sql, result_callback, &headers_printed, &errmsg);

    if (result != SQLITE_OK) {
        fprintf(stderr, "SQL Error: %s\n", errmsg ? errmsg : "Unknown error");
        return -1;
    }

    return 0;
}

/*
** Print welcome banner
*/
static void print_banner(void)
{
    printf("SQLite 2.8.17 for RISC OS 3.1\n");
    printf("ARM2 targeting system with 4MB RAM\n");
    printf("Type '.help' for available commands\n");
    printf("\n");
}

/*
** Print help information
*/
static void print_help(void)
{
    printf("Dot commands:\n");
    printf("  .quit, .exit         - Exit SQLite\n");
    printf("  .help                - Show this message\n");
    printf("  .open <file>         - Open database file\n");
    printf("  .tables              - List all tables\n");
    printf("  .schema [table]      - Show table schema\n");
    printf("  .verbose on|off      - Toggle verbose mode\n");
    printf("  .headers on|off      - Toggle result headers\n");
    printf("\n");
    printf("SQL statements:\n");
    printf("  Enter SQL statements at the prompt.\n");
    printf("  Statements are executed when terminated with a semicolon (;)\n");
    printf("  Multi-line statements are supported.\n");
    printf("\n");
    printf("Supported SQL:\n");
    printf("  CREATE TABLE - Create new table\n");
    printf("  DROP TABLE   - Drop table\n");
    printf("  INSERT       - Insert data (partial implementation)\n");
    printf("  SELECT       - Query data (partial implementation)\n");
    printf("  UPDATE       - Update data (partial implementation)\n");
    printf("  DELETE       - Delete data (partial implementation)\n");
    printf("  BEGIN        - Start transaction\n");
    printf("  COMMIT       - Commit transaction\n");
    printf("  ROLLBACK     - Rollback transaction\n");
    printf("  PRAGMA       - SQLite pragmas\n");
}

/*
** Interactive shell main loop
*/
int sqlite_shell_main(int argc, char **argv)
{
    char line_buffer[MAX_SQL_BUFFER];
    const char *db_file = ":memory:";
    char *errmsg = NULL;
    int exit_requested = 0;

    print_banner();

    /* Open database file if provided as argument */
    if (argc > 1) {
        db_file = argv[1];
    }

    shell.db = sqlite_open(db_file, 0, &errmsg);
    if (!shell.db) {
        fprintf(stderr, "Cannot open database '%s': %s\n", db_file,
                errmsg ? errmsg : "Unknown error");
        return 1;
    }

    if (argc > 1) {
        printf("Opened '%s'\n", db_file);
    } else {
        printf("Opened in-memory database\n");
    }

    /* Main interactive loop */
    while (!exit_requested) {
        const char *prompt = shell.in_statement ? CONTINUE_PROMPT : PROMPT;

        printf("%s", prompt);
        fflush(stdout);

        /* Read input line */
        if (fgets(line_buffer, sizeof(line_buffer), stdin) == NULL) {
            break;  /* EOF */
        }

        /* Remove trailing newline */
        size_t len = strlen(line_buffer);
        if (len > 0 && line_buffer[len - 1] == '\n') {
            line_buffer[len - 1] = '\0';
            len--;
        }

        /* Skip empty lines */
        if (len == 0) continue;

        /* Handle dot commands */
        if (line_buffer[0] == '.' && !shell.in_statement) {
            int ret = exec_dot_command(line_buffer);
            if (ret > 0) {
                exit_requested = 1;
            }
            continue;
        }

        /* Accumulate SQL statement */
        if (shell.sql_length + len + 2 < MAX_SQL_BUFFER) {
            if (shell.sql_length > 0) {
                shell.sql_buffer[shell.sql_length++] = ' ';
            }
            strcpy(shell.sql_buffer + shell.sql_length, line_buffer);
            shell.sql_length += len;
        } else {
            fprintf(stderr, "SQL statement too long (max %d bytes)\n", MAX_SQL_BUFFER);
            shell.sql_length = 0;
            shell.in_statement = 0;
            continue;
        }

        /* Check for statement terminator (semicolon) */
        if (shell.sql_buffer[shell.sql_length - 1] == ';') {
            shell.sql_buffer[shell.sql_length - 1] = '\0';  /* Remove semicolon */

            /* Execute SQL */
            exec_sql(shell.sql_buffer);

            /* Reset for next statement */
            shell.sql_length = 0;
            shell.in_statement = 0;
        } else {
            shell.in_statement = 1;
        }
    }

    /* Cleanup */
    if (shell.db) {
        sqlite_close(shell.db);
        shell.db = NULL;
    }

    printf("\nGoodbye.\n");
    return 0;
}
