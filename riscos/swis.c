/*
** RISC OS SWI Helper Functions
** Provides convenient wrappers around common SWI calls
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef RISCOS
#include <kernel.h>
#include <swis.h>
#endif

#include "swis.h"

/*
** Write character to output
*/
void riscos_writec(int c)
{
#ifdef RISCOS
    _kernel_swi_regs regs;
    regs.r[0] = c;
    _kernel_swi(OS_WriteC, &regs, &regs);
#else
    putchar(c);
#endif
}

/*
** Write string to output
*/
void riscos_writes(const char *s)
{
#ifdef RISCOS
    _kernel_swi_regs regs;
    regs.r[0] = (int)s;
    _kernel_swi(OS_WriteS, &regs, &regs);
#else
    printf("%s", s);
#endif
}

/*
** Write null-terminated string
*/
void riscos_write0(const char *s)
{
#ifdef RISCOS
    _kernel_swi_regs regs;
    regs.r[0] = (int)s;
    _kernel_swi(OS_Write0, &regs, &regs);
#else
    printf("%s", s);
#endif
}

/*
** Write newline
*/
void riscos_newline(void)
{
#ifdef RISCOS
    _kernel_swi_regs regs;
    _kernel_swi(OS_NewLine, &regs, &regs);
#else
    printf("\n");
#endif
}

/*
** Read character from input
*/
int riscos_readc(void)
{
#ifdef RISCOS
    _kernel_swi_regs regs;
    _kernel_swi(OS_ReadC, &regs, &regs);
    return regs.r[0];
#else
    return getchar();
#endif
}

/*
** Exit program with code
*/
void riscos_exit(int code)
{
#ifdef RISCOS
    _kernel_swi_regs regs;
    regs.r[0] = code;
    _kernel_swi(OS_Exit, &regs, &regs);
#else
    exit(code);
#endif
}

/*
** Get environment variable
*/
const char *riscos_getenv(const char *name)
{
#ifdef RISCOS
    _kernel_swi_regs regs;
    static char buffer[256];

    regs.r[0] = (int)name;
    regs.r[1] = (int)buffer;
    regs.r[2] = sizeof(buffer) - 1;

    if (_kernel_swi(OS_GetEnv, &regs, &regs) != NULL) {
        return NULL;
    }

    buffer[255] = '\0';
    return buffer;
#else
    return getenv(name);
#endif
}

/*
** CLI - Execute a command
*/
int riscos_cli(const char *cmd)
{
#ifdef RISCOS
    _kernel_swi_regs regs;
    _kernel_oserror *err;

    regs.r[0] = (int)cmd;

    err = _kernel_swi(OS_CLI, &regs, &regs);

    return err ? -1 : 0;
#else
    return system(cmd);
#endif
}

/*
** Allocate memory from dynamic area (alternative to RMA)
*/
void *riscos_claim_memory(unsigned int size)
{
#ifdef RISCOS
    _kernel_swi_regs regs;
    _kernel_oserror *err;

    regs.r[0] = 6;  /* Claim */
    regs.r[3] = size;

    err = _kernel_swi(OS_Module, &regs, &regs);

    if (err) return NULL;

    return (void *)regs.r[2];
#else
    return malloc(size);
#endif
}

/*
** Free allocated memory
*/
void riscos_free_memory(void *ptr)
{
#ifdef RISCOS
    _kernel_swi_regs regs;

    regs.r[0] = 7;  /* Free */
    regs.r[2] = (int)ptr;

    _kernel_swi(OS_Module, &regs, &regs);
#else
    free(ptr);
#endif
}
