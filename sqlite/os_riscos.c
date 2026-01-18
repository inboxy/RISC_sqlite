/*
** RISC OS VFS Implementation for SQLite 2.8.17
**
** Provides file I/O access using RISC OS SWI calls instead of standard C I/O.
** This allows SQLite to work within the RISC OS environment on ARM2 with 4MB RAM.
**
** Build flags: -DRISCOS -D__riscos__
** ARM2 flags: -march=armv2 -mapcs-26
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#ifdef RISCOS
#include <kernel.h>
#include <swis.h>
#endif

#include "os_riscos.h"

/*
** Translation from Unix paths to RISC OS format
**
** Examples:
**   /database/test.db  -> database.test/db
**   test.db            -> test/db
**   /var/db/app.db     -> var.db.app/db
**
** RISC OS uses:
**   . as directory separator (not /)
**   ,xxx as file type suffix (not file extension)
**   No leading path = current directory
*/
char *riscos_translate_path(const char *unix_path)
{
    char *result;
    size_t len;
    int i, j;

    if (!unix_path) return NULL;

    len = strlen(unix_path);
    result = (char *)malloc(len + 32);  /* Extra space for filetype suffix */

    if (!result) return NULL;

    i = 0;
    j = 0;

    /* Skip leading / */
    if (unix_path[0] == '/') i++;

    /* Convert path separators and add to result */
    while (i < len) {
        if (unix_path[i] == '/') {
            result[j++] = '.';          /* Convert / to . */
        } else if (unix_path[i] == '.') {
            /* Check if this is the final dot (file extension) */
            const char *p = unix_path + i + 1;
            int is_extension = 1;

            /* Look for slashes after this dot - if found, it's a directory dot */
            while (*p) {
                if (*p == '/') {
                    is_extension = 0;
                    break;
                }
                p++;
            }

            if (is_extension) {
                /* This is a file extension - convert dot to comma for RISC OS filetype */
                result[j++] = ',';
            } else {
                /* This is a directory component - keep as is for now */
                result[j++] = '.';
            }
        } else {
            result[j++] = unix_path[i];
        }
        i++;
    }

    result[j] = '\0';
    return result;
}

void riscos_free_path(char *path)
{
    if (path) free(path);
}

/*
** Open a file on RISC OS using OS_Find SWI
**
** flags: O_RDONLY, O_WRONLY, O_RDWR from fcntl.h equivalents
** Returns: 0 on success, -1 on error
*/
int riscos_vfs_open(const char *path, int flags, riscos_file_t *file)
{
#ifdef RISCOS
    _kernel_swi_regs regs;
    _kernel_oserror *err;
    char *riscos_path;
    int reason_code = OSFIND_OPENREAD;

    if (!path || !file) return -1;

    /* Translate Unix path to RISC OS format */
    riscos_path = riscos_translate_path(path);
    if (!riscos_path) return -1;

    /* Determine open mode from flags */
    if (flags & O_WRONLY) {
        reason_code = OSFIND_OPENWRITE;
    } else if ((flags & O_RDWR) == O_RDWR) {
        reason_code = OSFIND_OPENUPDATE;
    }

    /* Call OS_Find */
    regs.r[0] = reason_code;
    regs.r[1] = (int)riscos_path;

    err = _kernel_swi(OS_Find, &regs, &regs);

    riscos_free_path(riscos_path);

    if (err || regs.r[0] == 0) {
        return -1;  /* Failed to open */
    }

    file->handle = regs.r[0];
    file->file_pos = 0;
    file->file_size = 0;

    return 0;
#else
    /* Stub for non-RISC OS systems */
    return -1;
#endif
}

/*
** Close a file on RISC OS
*/
int riscos_vfs_close(riscos_file_t *file)
{
#ifdef RISCOS
    _kernel_swi_regs regs;
    _kernel_oserror *err;

    if (!file || file->handle == 0) return -1;

    /* Call OS_Find with reason code 0 (close) */
    regs.r[0] = OSFIND_CLOSE;
    regs.r[1] = file->handle;

    err = _kernel_swi(OS_Find, &regs, &regs);

    file->handle = 0;
    return err ? -1 : 0;
#else
    return -1;
#endif
}

/*
** Read bytes from file using OS_GBPB SWI
** Returns number of bytes read, -1 on error
*/
int riscos_vfs_read(riscos_file_t *file, void *buf, int count)
{
#ifdef RISCOS
    _kernel_swi_regs regs;
    _kernel_oserror *err;

    if (!file || !buf || file->handle == 0) return -1;

    /* Call OS_GBPB 4 (read from file pointer) */
    regs.r[0] = 4;                     /* Reason code: read from file pointer */
    regs.r[1] = file->handle;
    regs.r[2] = (int)buf;
    regs.r[3] = count;

    err = _kernel_swi(OS_GBPB, &regs, &regs);

    if (err) return -1;

    /* r[3] contains number of bytes NOT transferred */
    int bytes_read = count - regs.r[3];
    file->file_pos += bytes_read;

    return bytes_read;
#else
    return -1;
#endif
}

/*
** Write bytes to file using OS_GBPB SWI
** Returns number of bytes written, -1 on error
*/
int riscos_vfs_write(riscos_file_t *file, const void *buf, int count)
{
#ifdef RISCOS
    _kernel_swi_regs regs;
    _kernel_oserror *err;

    if (!file || !buf || file->handle == 0) return -1;

    /* Call OS_GBPB 2 (write from file pointer) */
    regs.r[0] = 2;                     /* Reason code: write from file pointer */
    regs.r[1] = file->handle;
    regs.r[2] = (int)buf;
    regs.r[3] = count;

    err = _kernel_swi(OS_GBPB, &regs, &regs);

    if (err) return -1;

    /* r[3] contains number of bytes NOT transferred */
    int bytes_written = count - regs.r[3];
    file->file_pos += bytes_written;

    return bytes_written;
#else
    return -1;
#endif
}

/*
** Seek to position in file using OS_Args SWI
** whence: SEEK_SET, SEEK_CUR, SEEK_END
** Returns: 0 on success, -1 on error
*/
int riscos_vfs_seek(riscos_file_t *file, int offset, int whence)
{
#ifdef RISCOS
    _kernel_swi_regs regs;
    _kernel_oserror *err;
    int new_pos;

    if (!file || file->handle == 0) return -1;

    switch (whence) {
        case SEEK_SET:
            new_pos = offset;
            break;
        case SEEK_CUR:
            new_pos = file->file_pos + offset;
            break;
        case SEEK_END:
            if (file->file_size == 0) {
                /* Get file size first */
                if (riscos_vfs_filesize(file) < 0) return -1;
            }
            new_pos = file->file_size + offset;
            break;
        default:
            return -1;
    }

    /* Call OS_Args 1 (set file pointer) */
    regs.r[0] = 1;
    regs.r[1] = file->handle;
    regs.r[2] = new_pos;

    err = _kernel_swi(OS_Args, &regs, &regs);

    if (err) return -1;

    file->file_pos = new_pos;
    return 0;
#else
    return -1;
#endif
}

/*
** Get current file position
*/
int riscos_vfs_tell(riscos_file_t *file)
{
#ifdef RISCOS
    _kernel_swi_regs regs;
    _kernel_oserror *err;

    if (!file || file->handle == 0) return -1;

    /* Call OS_Args 0 (read file pointer) */
    regs.r[0] = 0;
    regs.r[1] = file->handle;

    err = _kernel_swi(OS_Args, &regs, &regs);

    if (err) return -1;

    /* r[2] contains current file pointer */
    file->file_pos = regs.r[2];
    return file->file_pos;
#else
    return -1;
#endif
}

/*
** Get file size in bytes
*/
int riscos_vfs_filesize(riscos_file_t *file)
{
#ifdef RISCOS
    _kernel_swi_regs regs;
    _kernel_oserror *err;

    if (!file || file->handle == 0) return -1;

    /* Call OS_Args 2 (read file extent) */
    regs.r[0] = 2;
    regs.r[1] = file->handle;

    err = _kernel_swi(OS_Args, &regs, &regs);

    if (err) return -1;

    /* r[2] contains file extent (size) */
    file->file_size = regs.r[2];
    return file->file_size;
#else
    return -1;
#endif
}

/*
** Synchronize file with disk (flush buffers)
** RISC OS 3.1 doesn't guarantee this will work, but we try
*/
int riscos_vfs_sync(riscos_file_t *file)
{
#ifdef RISCOS
    _kernel_swi_regs regs;
    _kernel_oserror *err;

    if (!file || file->handle == 0) return -1;

    /* Call OS_Args 255 (ensure file, if available) */
    regs.r[0] = 255;
    regs.r[1] = file->handle;

    err = _kernel_swi(OS_Args, &regs, &regs);

    /* Don't fail if this SWI is not available in RISC OS 3.1 */
    return 0;
#else
    return -1;
#endif
}

/*
** Delete a file
** Returns: 0 on success, -1 on error
*/
int riscos_vfs_delete(const char *path)
{
#ifdef RISCOS
    _kernel_swi_regs regs;
    _kernel_oserror *err;
    char *riscos_path;

    if (!path) return -1;

    riscos_path = riscos_translate_path(path);
    if (!riscos_path) return -1;

    /* Call OS_File 6 (delete file) */
    regs.r[0] = 6;
    regs.r[1] = (int)riscos_path;

    err = _kernel_swi(OS_File, &regs, &regs);

    riscos_free_path(riscos_path);

    return err ? -1 : 0;
#else
    return -1;
#endif
}
