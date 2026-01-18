/*
** RISC OS Application Startup Code
** Entry point and initialization for SQLite client
**
** This module handles:
** - Application startup and initialization
** - Memory initialization
** - Environment setup
** - Main entry point
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef RISCOS
#include <kernel.h>
#endif

#include "swis.h"
#include "../sqlite/sqlite.h"
#include "../sqlite/mem_riscos.h"

/* Forward declaration of main shell function */
extern int sqlite_shell_main(int argc, char **argv);

/*
** Application entry point
**
** In RISC OS, we need to set up the environment and then call the main function.
** The ARM2 processor uses 26-bit addressing mode, which affects how we set up
** the initial state.
*/
void app_startup(void)
{
    /* This would be called from the RISC OS loader */
    /* In practice, this is typically handled by the C runtime */
}

/*
** C runtime entry point - called by RISC OS loader
*/
int main(int argc, char **argv)
{
    int result;

    /* Initialize memory system */
    /* RISC OS 3.1 provides RMA (Relocatable Module Area) for allocation */

    /* Print banner */
    printf("SQLite 2.8.17 for RISC OS 3.1\n");
    printf("ARM2 porting project\n");
    printf("Memory limit: 4MB, Target: 500KB footprint\n");
    printf("\n");

    /* Check memory availability */
    int available = riscos_mem_available();
    printf("Available memory: %d bytes\n", available);

    if (available < 500000) {
        fprintf(stderr, "Error: Not enough memory (need at least 500KB)\n");
        return 1;
    }

    /* Call the interactive shell */
    result = sqlite_shell_main(argc, argv);

    return result;
}

/*
** Module information structure (for RISC OS module interface)
*/
#ifdef RISCOS
struct {
    char start_code;
    char initialisation_code;
    char finalisation_code;
    char service_call_handler;
    char title_string;
    char help_string;
    char help_table;
    char swi_base;
    char swi_handler;
    char swi_decode_table;
    char messages_file_name;
} module_header = {
    /* This would contain proper module header if compiled as a module */
};
#endif

/*
** Application cleanup on exit
*/
void app_cleanup(void)
{
    /* Print memory statistics before exit */
    printf("\n");
    printf("SQLite session ended.\n");

    /* Print final memory statistics if DEBUG is enabled */
#ifdef DEBUG
    riscos_mem_report();
#endif
}

/*
** Alternative entry point for RISC OS loader (if needed)
*/
int __riscos_main(int argc, char **argv)
{
    return main(argc, argv);
}

/*
** Stack check function (required by some toolchains)
*/
void __stack_check_fail(void)
{
    fprintf(stderr, "Stack overflow!\n");
    exit(1);
}

/*
** Assert failure handler
*/
void __assert(const char *file, int line, const char *assertion)
{
    fprintf(stderr, "Assertion failed: %s at %s:%d\n", assertion, file, line);
    exit(1);
}

/*
** Memory allocation error handler
*/
void *malloc_failed(size_t size)
{
    fprintf(stderr, "Memory allocation failed: %zu bytes requested\n", size);
    return NULL;
}
