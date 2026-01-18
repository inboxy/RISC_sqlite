/*
** RISC OS VFS (Virtual File System) Layer for SQLite
** Implements file I/O using RISC OS SWI calls
**
** Key SWIs used:
** - OS_Find (find/open/close files)
** - OS_GBPB (read/write bytes)
** - OS_Args (file pointer operations)
** - OS_File (file operations like delete)
*/

#ifndef _OS_RISCOS_H_
#define _OS_RISCOS_H_

#include <stdio.h>

/* RISC OS SWI definitions */
#define OS_Find     0x0D
#define OS_GBPB     0x0C
#define OS_Args     0x09
#define OS_File     0x08
#define OS_Module   0x1E

/* OS_Find reason codes */
#define OSFIND_OPENREAD     0x40    /* Open file for reading */
#define OSFIND_OPENWRITE    0x80    /* Open file for writing */
#define OSFIND_OPENUPDATE   0xC0    /* Open file for update */
#define OSFIND_CLOSE        0x00    /* Close file */

/* File handle structure */
typedef struct {
    int handle;                     /* RISC OS file handle */
    int file_pos;                   /* Current position in file */
    int file_size;                  /* File size in bytes */
} riscos_file_t;

/* VFS function declarations */
int riscos_vfs_open(const char *path, int flags, riscos_file_t *file);
int riscos_vfs_close(riscos_file_t *file);
int riscos_vfs_read(riscos_file_t *file, void *buf, int count);
int riscos_vfs_write(riscos_file_t *file, const void *buf, int count);
int riscos_vfs_seek(riscos_file_t *file, int offset, int whence);
int riscos_vfs_tell(riscos_file_t *file);
int riscos_vfs_filesize(riscos_file_t *file);
int riscos_vfs_sync(riscos_file_t *file);
int riscos_vfs_delete(const char *path);

/* Path translation utilities */
char *riscos_translate_path(const char *unix_path);
void riscos_free_path(char *path);

#endif /* _OS_RISCOS_H_ */
