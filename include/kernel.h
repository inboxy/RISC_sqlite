/*
** RISC OS kernel.h stub
** Used for cross-platform compilation when RISC OS SDK headers aren't available
*/

#ifndef _KERNEL_H_
#define _KERNEL_H_

/* RISC OS kernel structure definitions */
/* These are the same on both RISC OS and cross-compilation platforms */

typedef struct {
    int r[16];
} _kernel_swi_regs;

typedef struct {
    int errnum;
    char errmess[252];
} _kernel_oserror;

/* Declare the _kernel_swi function (implementation in kernel.c or kernel stub) */
_kernel_oserror *_kernel_swi(int swi_number, _kernel_swi_regs *in, _kernel_swi_regs *out);

#endif /* _KERNEL_H_ */
