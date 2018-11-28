#ifndef FILESYS_CACHE_H
#define FILESYS_CACHE_H

#include "devices/block.h"
#include "threads/synch.h"
#include "filesys/cache.h"
#include "filesys/filesys.h"

void cache_init(void);
void cache_remove(void);
struct cache_entry * cache_find(block_sector_t sector_index);
void cache_read(block_sector_t sector_index, void * dest);
void cache_write(block_sector_t sector_index, void *ori);
void flush_func(void);
void flush_once(void);


#endif
