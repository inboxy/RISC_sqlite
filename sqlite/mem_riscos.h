/*
** RISC OS Memory Management for SQLite
** Uses OS_Module SWI for heap allocation in RMA (Relocatable Module Area)
*/

#ifndef _MEM_RISCOS_H_
#define _MEM_RISCOS_H_

#include <stdlib.h>

/* OS_Module reason codes */
#define OSMODULE_CLAIM      6   /* Claim RMA space */
#define OSMODULE_FREE       7   /* Free RMA space */

/* Statistics structure */
typedef struct {
    unsigned int total_allocated;
    unsigned int peak_allocated;
    unsigned int num_allocations;
    unsigned int num_frees;
} riscos_mem_stats_t;

/* Memory management functions */
void *riscos_malloc(size_t size);
void *riscos_calloc(size_t nmemb, size_t size);
void *riscos_realloc(void *ptr, size_t size);
void riscos_free(void *ptr);

/* Statistics and debugging */
void riscos_mem_stats(riscos_mem_stats_t *stats);
void riscos_mem_report(void);
int riscos_mem_available(void);

#endif /* _MEM_RISCOS_H_ */
