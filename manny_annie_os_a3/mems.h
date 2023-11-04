#ifndef MEMS_H
#define MEMS_H

#include <stddef.h>

void mems_init();
void mems_finish();
void* mems_malloc(size_t size);
void mems_free(void* ptr);
void mems_print_stats();
void* mems_get(void* v_ptr);

#endif  // MEMS_H
