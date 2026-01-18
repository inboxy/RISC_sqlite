/*
** RISC OS swis.h stub
** SWI number definitions for RISC OS
** Used for cross-platform compilation
*/

#ifndef _SWIS_H_
#define _SWIS_H_

/* SWI Number definitions - same on all platforms */
#define SWI_NumberFromString(s) 0  /* Stub */

/* SWI numbers */
#define OS_WriteC       0x00
#define OS_WriteS       0x01
#define OS_Write0       0x02
#define OS_NewLine      0x03
#define OS_ReadC        0x04
#define OS_CLI          0x05
#define OS_Byte         0x06
#define OS_Word         0x07
#define OS_File         0x08
#define OS_Args         0x09
#define OS_GBPB         0x0C
#define OS_Find         0x0D
#define OS_ReadLine     0x0E
#define OS_GetEnv       0x10
#define OS_Exit         0x11
#define OS_Module       0x1E

#endif /* _SWIS_H_ */
