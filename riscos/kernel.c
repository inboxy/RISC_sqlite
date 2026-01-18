/*
** RISC OS kernel stub implementation
** Provides stub implementation of _kernel_swi for non-RISC OS compilation
**
** On actual RISC OS, this would be replaced with the real kernel.a library
*/

#include <stdio.h>
#include <stddef.h>
#include <kernel.h>

#ifdef RISCOS
/* On RISC OS, _kernel_swi would be provided by the RISC OS kernel library */
/* This stub should not be compiled on RISC OS */
#error "kernel.c should not be compiled on RISC OS - link with kernel.a instead"
#else

/*
** Stub implementation of _kernel_swi
** Does nothing and returns no error
**
** On RISC OS, this would invoke the actual SWI through CPU interrupts
** For cross-platform testing, we just return NULL (no error)
*/
_kernel_oserror *_kernel_swi(int swi_number, _kernel_swi_regs *in, _kernel_swi_regs *out)
{
    /* Stub: copy input registers to output (many SWIs don't modify input) */
    if (in && out) {
        *out = *in;
    }

    /* Return NULL to indicate success (no error) */
    return NULL;
}

#endif /* RISCOS */
