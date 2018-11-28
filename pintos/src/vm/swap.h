#ifndef VM_SWAP_H
#define VM_SWAP_H

<<<<<<< HEAD
#include "lib/stddef.h"

void swap_table_init(void);

size_t swap_out(void *kpage);

void swap_in(size_t swap_index, void *kpage);

void swap_free (int swap_index);

=======
void swap_table_init(void);
size_t swap_out(void *kpage);
void swap_in(size_t swap_index, void *kpage);

>>>>>>> a7411f174dcd283c0088ab0e1fe146a560c06410
#endif