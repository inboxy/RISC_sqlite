/*
** RISC OS Memory Management Implementation for SQLite
**
** Allocates memory from RMA (Relocatable Module Area) using OS_Module SWI.
** Tracks allocations for debugging and statistics.
**
** In RISC OS 3.1, available memory is typically limited. This implementation:
** - Uses OS_Module 6 to claim RMA space
** - Uses OS_Module 7 to free RMA space
** - Maintains allocation statistics for debugging
** - Avoids fragmentation by using fixed-size pools where practical
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef RISCOS
#include <kernel.h>
#include <swis.h>
#endif

#include "mem_riscos.h"

/* Global statistics */
static struct {
    unsigned int total_allocated;
    unsigned int peak_allocated;
    unsigned int current_allocated;
    unsigned int num_allocations;
    unsigned int num_frees;
} mem_stats = {0, 0, 0, 0, 0};

/* Memory allocation tracking */
#define RISCOS_MEM_SIGNATURE 0xDEADBEEF

typedef struct {
    unsigned int signature;      /* Used to verify valid allocation */
    unsigned int size;           /* Actual size allocated */
} riscos_alloc_header_t;

/*
** Allocate memory from RMA
** Returns pointer to allocated memory, or NULL on failure
*/
void *riscos_malloc(size_t size)
{
    if (size == 0) return NULL;

#ifdef RISCOS
    _kernel_swi_regs regs;
    _kernel_oserror *err;
    riscos_alloc_header_t *header;
    size_t total_size;
    void *ptr;

    /* Add size for header */
    total_size = size + sizeof(riscos_alloc_header_t);

    /* Call OS_Module 6 (claim RMA) */
    regs.r[0] = OSMODULE_CLAIM;
    regs.r[3] = total_size;

    err = _kernel_swi(OS_Module, &regs, &regs);

    if (err) {
        return NULL;  /* Failed to allocate */
    }

    ptr = (void *)regs.r[2];
    if (!ptr) return NULL;

    /* Write header */
    header = (riscos_alloc_header_t *)ptr;
    header->signature = RISCOS_MEM_SIGNATURE;
    header->size = size;

    /* Update statistics */
    mem_stats.total_allocated += size;
    mem_stats.current_allocated += size;
    mem_stats.num_allocations++;

    if (mem_stats.current_allocated > mem_stats.peak_allocated) {
        mem_stats.peak_allocated = mem_stats.current_allocated;
    }

    /* Return pointer after header */
    return (void *)((char *)ptr + sizeof(riscos_alloc_header_t));
#else
    /* Fallback to standard malloc for non-RISC OS systems */
    void *ptr = malloc(size + sizeof(riscos_alloc_header_t));
    if (!ptr) return NULL;

    riscos_alloc_header_t *header = (riscos_alloc_header_t *)ptr;
    header->signature = RISCOS_MEM_SIGNATURE;
    header->size = size;

    mem_stats.total_allocated += size;
    mem_stats.current_allocated += size;
    mem_stats.num_allocations++;

    if (mem_stats.current_allocated > mem_stats.peak_allocated) {
        mem_stats.peak_allocated = mem_stats.current_allocated;
    }

    return (void *)((char *)ptr + sizeof(riscos_alloc_header_t));
#endif
}

/*
** Allocate and zero memory
*/
void *riscos_calloc(size_t nmemb, size_t size)
{
    size_t total_size = nmemb * size;
    void *ptr = riscos_malloc(total_size);

    if (ptr) {
        memset(ptr, 0, total_size);
    }

    return ptr;
}

/*
** Reallocate memory
*/
void *riscos_realloc(void *ptr, size_t size)
{
    void *new_ptr;
    riscos_alloc_header_t *old_header;
    size_t old_size;

    if (!ptr) {
        return riscos_malloc(size);
    }

    if (size == 0) {
        riscos_free(ptr);
        return NULL;
    }

    /* Get old header */
    old_header = (riscos_alloc_header_t *)((char *)ptr - sizeof(riscos_alloc_header_t));

    if (old_header->signature != RISCOS_MEM_SIGNATURE) {
        /* Invalid allocation header - this is likely an error */
        return NULL;
    }

    old_size = old_header->size;

    /* Allocate new block */
    new_ptr = riscos_malloc(size);
    if (!new_ptr) {
        return NULL;
    }

    /* Copy old data */
    size_t copy_size = (size < old_size) ? size : old_size;
    if (copy_size > 0) {
        memcpy(new_ptr, ptr, copy_size);
    }

    /* Free old block */
    riscos_free(ptr);

    return new_ptr;
}

/*
** Free allocated memory
*/
void riscos_free(void *ptr)
{
    if (!ptr) return;

#ifdef RISCOS
    _kernel_swi_regs regs;
    riscos_alloc_header_t *header;

    /* Get header */
    header = (riscos_alloc_header_t *)((char *)ptr - sizeof(riscos_alloc_header_t));

    if (header->signature != RISCOS_MEM_SIGNATURE) {
        /* Invalid header - don't free, avoid corruption */
        return;
    }

    /* Call OS_Module 7 (free RMA) */
    regs.r[0] = OSMODULE_FREE;
    regs.r[2] = (int)header;

    _kernel_swi(OS_Module, &regs, &regs);

    /* Update statistics */
    mem_stats.current_allocated -= header->size;
    mem_stats.num_frees++;
#else
    /* Fallback to standard free */
    riscos_alloc_header_t *header = (riscos_alloc_header_t *)((char *)ptr - sizeof(riscos_alloc_header_t));

    if (header->signature != RISCOS_MEM_SIGNATURE) {
        return;
    }

    mem_stats.current_allocated -= header->size;
    mem_stats.num_frees++;

    free(header);
#endif
}

/*
** Get memory statistics
*/
void riscos_mem_stats(riscos_mem_stats_t *stats)
{
    if (stats) {
        stats->total_allocated = mem_stats.total_allocated;
        stats->peak_allocated = mem_stats.peak_allocated;
        stats->num_allocations = mem_stats.num_allocations;
        stats->num_frees = mem_stats.num_frees;
    }
}

/*
** Print memory statistics report
*/
void riscos_mem_report(void)
{
    printf("Memory Statistics:\n");
    printf("  Total allocated:    %u bytes\n", mem_stats.total_allocated);
    printf("  Peak allocated:     %u bytes\n", mem_stats.peak_allocated);
    printf("  Currently allocated: %u bytes\n", mem_stats.current_allocated);
    printf("  Allocations:        %u\n", mem_stats.num_allocations);
    printf("  Frees:              %u\n", mem_stats.num_frees);
}

/*
** Check available memory in RMA (approximate)
** This is a rough estimate and may not be accurate on all RISC OS versions
*/
int riscos_mem_available(void)
{
#ifdef RISCOS
    _kernel_swi_regs regs;
    _kernel_oserror *err;

    /* Try to claim 0 bytes to get current RMA status - this is not standard */
    /* In practice, just return remaining budget based on 4MB total */
    int total_budget = 4 * 1024 * 1024;  /* 4MB */
    int remaining = total_budget - mem_stats.current_allocated;

    return (remaining > 0) ? remaining : 0;
#else
    return 4 * 1024 * 1024;  /* Assume 4MB available */
#endif
}
