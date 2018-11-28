#ifndef VM_FRAME_H
#define VM_FRAME_H

#include "lib/kernel/list.h"
#include "threads/palloc.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/malloc.h"

<<<<<<< HEAD
struct list frame_table;
struct lock frame_lock;

struct frame_table_entry{
	void *kernel_virtual_address;
	void *user_virtual_address;
	struct thread * thread;
  	bool pinning;
=======
struct frame_table_entry{
	void *kernel_virtual_address;
  	void *physical_address;
>>>>>>> a7411f174dcd283c0088ab0e1fe146a560c06410
  	struct list_elem elem;
};

void frame_table_init(void);
<<<<<<< HEAD
void * frame_alloc(enum palloc_flags flags, void * upage);
void frame_table_entry_insert(void * page, void * upage);
void frame_table_entry_free(void * page);
void * frame_table_entry_evict(enum palloc_flags flags);
void only_frame_table_entry_free(void * page);

void frame_table_entry_set_pin_true(void * kpage);
void frame_table_entry_set_pin_false(void * kpage);
=======
void * frame_alloc(enum palloc_flags flags);
void frame_table_entry_insert(void * page);
void frame_table_entry_free(void * page);
void frame_table_entry_evict(void);
>>>>>>> a7411f174dcd283c0088ab0e1fe146a560c06410

#endif

