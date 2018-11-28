#include "devices/block.h"
#include "threads/synch.h"
#include "filesys/cache.h"
#include "filesys/filesys.h"
#include "threads/thread.h"
#include "devices/timer.h"

struct cache_entry{
	block_sector_t sector_index;
	bool dirty;
	uint8_t buffer[BLOCK_SECTOR_SIZE];
	bool used;
	struct lock cache_lock;
};

static struct cache_entry * cache = NULL;
//static struct lock cache_lock;
static int clock=0;

void cache_init(void){
	//lock_init(&cache_lock);
	cache = (struct cache_entry *) malloc(sizeof(struct cache_entry)*64);
	int i;
	for(i=0;i<64;i++){
		(cache+i)->sector_index = -1;
		(cache+i)->used = false;
		lock_init(&(cache+i)->cache_lock);
	}
	struct thread * flusher = thread_create("flusher", PRI_DEFAULT, flush_func, NULL); 
}

void flush_func(void){
	while(cache != NULL){
		timer_sleep(1);
		int i;
		//lock_acquire(&cache_lock);
		for(i=0;i<64;i++){
			lock_acquire(&(cache+i)->cache_lock);
			if((cache+i)->used == true){
				if((cache+i)->dirty == true){
					block_write (fs_device, (cache+i)->sector_index, (cache+i)->buffer);
					(cache+i)->dirty = false;
					(cache+i)->used = false;
				}
			}
			lock_release(&(cache+i)->cache_lock);
		}
		//lock_release(&cache_lock);
	}
	thread_exit();
}

void flush_once(void){
	int i;
	for(i=0;i<64;i++){
		lock_acquire(&(cache+i)->cache_lock);
		if((cache+i)->used == true){
			if((cache+i)->dirty == true){
				block_write (fs_device, (cache+i)->sector_index, (cache+i)->buffer);
				(cache+i)->dirty = false;
				(cache+i)->used = false;
			}
		}
		lock_release(&(cache+i)->cache_lock);
	}
}



void cache_remove(void){
	int i;
	for(i=0;i<64;i++){
		if((cache+i)->used == true){
			if((cache+i)->dirty == true){
				block_write (fs_device, (cache+i)->sector_index, (cache+i)->buffer);
			}
		}
	}
	free(cache);
	cache = NULL;
}

struct cache_entry * cache_find(block_sector_t sector_index){
	int i;
	for(i=0; i<64; i++){
		lock_acquire(&(cache+i)->cache_lock);
		if((cache+i)->sector_index == sector_index){
			lock_release(&(cache+i)->cache_lock);
			return (cache+i);
		}
		lock_release(&(cache+i)->cache_lock);
	}

	return NULL;
}

void cache_read(block_sector_t sector_index, void * dest){
	//lock_acquire(&cache_lock);
	struct cache_entry * cache_entry = cache_find(sector_index);
	if(cache_entry == NULL){
		int i;
		for(i=0;i<64;i++){
			lock_acquire(&(cache+i)->cache_lock);
			if((cache+i)->used == false){
				(cache+i)->used = true;
				(cache+i)->dirty = false;
				(cache+i)->sector_index = sector_index;
				block_read(fs_device, sector_index, (cache+i)->buffer);
				memcpy(dest, (cache+i)->buffer, BLOCK_SECTOR_SIZE);
				lock_release(&(cache+i)->cache_lock);
				//lock_release(&cache_lock);
				return;
			}
			lock_release(&(cache+i)->cache_lock);
		}
		// eviction policy popopopopopopopo
		lock_acquire(&(cache+clock)->cache_lock);
		if((cache+clock)->dirty == true){
			block_write (fs_device, (cache+clock)->sector_index, (cache+clock)->buffer);
		}
		(cache+clock)->used = true;
		(cache+clock)->dirty = false;
		(cache+clock)->sector_index = sector_index;
		block_read(fs_device, sector_index, (cache+clock)->buffer);
		memcpy(dest, (cache+clock)->buffer, BLOCK_SECTOR_SIZE);
		lock_release(&(cache+clock)->cache_lock);
		clock = (clock+1)%64;
	}
	else{
		lock_acquire(&cache_entry->cache_lock);
		memcpy(dest, cache_entry->buffer, BLOCK_SECTOR_SIZE);
		lock_release(&cache_entry->cache_lock);
	}
	//lock_release(&cache_lock);
}

void cache_write(block_sector_t sector_index, void *ori){
	//lock_acquire(&cache_lock);
	struct cache_entry * cache_entry = cache_find(sector_index);
	if(cache_entry == NULL){
		int i;
		for(i=0;i<64;i++){
			lock_acquire(&(cache+i)->cache_lock);
			if((cache+i)->used == false){
				(cache+i)->used = true;
				(cache+i)->dirty = true;
				(cache+i)->sector_index = sector_index;
				//block_read(fs_device, sector_index, (cache+i)->buffer);
				memcpy((cache+i)->buffer, ori, BLOCK_SECTOR_SIZE);
				lock_release(&(cache+i)->cache_lock);
				//lock_release(&cache_lock);
				return;
			}
			lock_release(&(cache+i)->cache_lock);

		}
		// eviction policy popopopopopopopo
		lock_acquire(&(cache+clock)->cache_lock);
		if((cache+clock)->dirty == true){
			block_write (fs_device, (cache+clock)->sector_index, (cache+clock)->buffer);
		}

		(cache+clock)->used = true;
		(cache+clock)->dirty = true;
		(cache+clock)->sector_index = sector_index;
		//block_read(fs_device, sector_index, (cache+clock)->buffer);
		memcpy((cache+clock)->buffer, ori ,BLOCK_SECTOR_SIZE);
		lock_release(&(cache+clock)->cache_lock);
		clock = (clock+1)%64;
	}
	else{
		lock_acquire(&cache_entry->cache_lock);
		cache_entry->used = true;
		cache_entry->dirty = true;
		memcpy(cache_entry->buffer,ori,BLOCK_SECTOR_SIZE);
		lock_release(&cache_entry->cache_lock);
	}
	//lock_release(&cache_lock);
}
