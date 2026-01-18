/*
** RISC OS SWI (Software Interrupt) Definitions
** Common SWI calls needed for RISC OS 3.1 application
*/

#ifndef _RISCOS_SWIS_H_
#define _RISCOS_SWIS_H_

/* Common SWI Numbers */
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
#define OS_CallASWI     0x71
#define OS_CallASWIR12  0x72

/* Error definitions */
#define RISCOS_NOERROR  0

/* File I/O flags */
#define O_RDONLY        0x00
#define O_WRONLY        0x01
#define O_RDWR          0x02
#define O_CREAT         0x100
#define O_EXCL          0x200
#define O_TRUNC         0x400
#define O_APPEND        0x800

/* Seek modes */
#define SEEK_SET        0
#define SEEK_CUR        1
#define SEEK_END        2

#endif /* _RISCOS_SWIS_H_ */
