#include "lib/kernel/list.h"
#include "vm/frame.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/palloc.h"
#include "threads/malloc.h"
<<<<<<< HEAD
#include "lib/kernel/hash.h"
#include "vm/page.h"

static struct list_elem * clock;
=======

static struct list frame_table;
static struct lock frame_lock;
>>>>>>> a7411f174dcd283c0088ab0e1fe146a560c06410

void frame_table_init(void) {
  list_init(&frame_table);
  lock_init(&frame_lock);
<<<<<<< HEAD
  clock = NULL;
}

void * frame_alloc(enum palloc_flags flags, void * upage) {
  lock_acquire(&frame_lock);
  void * page = palloc_get_page(flags);
  if (page == NULL) {
    // page allocation failed. Need to swap frame to allocate memory to such page
    page = frame_table_entry_evict(flags);
    //page = palloc_get_page(flags);
  }
 
  frame_table_entry_insert(page, upage);
  lock_release(&frame_lock);
  return page;
}

void frame_table_entry_insert(void * page, void * upage){
  struct frame_table_entry *frame_table_entry = malloc(sizeof(struct frame_table_entry));
  frame_table_entry->kernel_virtual_address = page;
  frame_table_entry->user_virtual_address = upage;
  frame_table_entry->pinning = true;
  frame_table_entry->thread = thread_current();
  //printf("%x\n", upage);

  //lock_acquire(&frame_lock);
  list_push_back(&frame_table, &frame_table_entry->elem);
  //lock_release(&frame_lock);
=======
}

void * frame_alloc(enum palloc_flags flags) {
  void * page = palloc_get_page(flags);
  if (page == NULL) {
    // page allocation failed. Need to swap frame to allocate memory to such page
    return NULL;
    //frame_table_entry_evict();
  }
  frame_table_entry_insert(page);
  return page;
}

void frame_table_entry_insert(void * page){
  struct frame_table_entry *frame_table_entry = malloc(sizeof(struct frame_table_entry));
  frame_table_entry->kernel_virtual_address = page;
  frame_table_entry->physical_address = vtop(page);
  
  lock_acquire(&frame_lock);
  list_push_front(&frame_table, &frame_table_entry->elem);
  lock_release(&frame_lock);
>>>>>>> a7411f174dcd283c0088ab0e1fe146a560c06410
  return;
}

void frame_table_entry_free(void * page){
  struct list_elem *e;
<<<<<<< HEAD
  lock_acquire(&frame_lock);
  for (e = list_begin (&frame_table); e != list_end (&frame_table);
       e = list_next (e))
    {
      struct frame_table_entry * fte  = list_entry (e, struct frame_table_entry, elem);
      if(fte->kernel_virtual_address == page){
          list_remove(&fte->elem);
          palloc_free_page(fte->kernel_virtual_address);
          free(fte);
          lock_release(&frame_lock);
          return;
      }
    }
  lock_release(&frame_lock);
}

void only_frame_table_entry_free(void * page){
  struct list_elem *e;
  lock_acquire(&frame_lock);
=======

>>>>>>> a7411f174dcd283c0088ab0e1fe146a560c06410
  for (e = list_begin (&frame_table); e != list_end (&frame_table);
       e = list_next (e))
    {
      struct frame_table_entry * fte  = list_entry (e, struct frame_table_entry, elem);
      if(fte->kernel_virtual_address == page){
<<<<<<< HEAD
          list_remove(&fte->elem);
=======
          lock_acquire(&frame_lock);
          list_remove(&fte->elem);
          palloc_free_page(fte->kernel_virtual_address);
>>>>>>> a7411f174dcd283c0088ab0e1fe146a560c06410
          free(fte);
          lock_release(&frame_lock);
          return;
      }
    }
<<<<<<< HEAD
  lock_release(&frame_lock);
}

void * frame_table_entry_evict(enum palloc_flags flags){
  // struct list_elem *e;
  // lock_acquire(&frame_lock); 
  // e = list_begin(&frame_table);
  // struct frame_table_entry * fte  = list_entry (e, struct frame_table_entry, elem);
  // void * kpage = fte->kernel_virtual_address;
  // list_remove(e);

  // int swap_index = swap_out(kpage);
  // palloc_free_page(fte->kernel_virtual_address);
  // lock_release(&frame_lock);

  // struct supplement_page_table_entry spte;
  // spte.upage = pg_round_down(fte->user_virtual_address);
  // struct hash_elem *elem = hash_find(&thread_current()->supplement_page_table, &spte.elem);
  // if (elem == NULL)
  //   return false;
  // struct supplement_page_table_entry * spte2;
  // spte2 = hash_entry (elem, struct supplement_page_table_entry, elem); 
  // spte2->swap_index = swap_index;
  // spte2->status = 2;
  // spte2->on_frame = false;

  // pagedir_clear_page(thread_current()->pagedir, fte->user_virtual_address);

  // free(fte);
  //lock_acquire(&frame_lock);
  int i;
  struct frame_table_entry * fte;
  clock = list_begin(&frame_table);
  for (i=0; i<list_size(&frame_table);i++)
    {
      // if(clock ==NULL || clock == list_rbegin(&frame_table))
      //   clock = list_begin(&frame_table);
      // else
      //   clock = list_next(clock);
      fte = list_entry (clock, struct frame_table_entry, elem);
      //printf("%x\n", fte->kernel_virtual_address);
      if (fte->pinning){
        clock = list_next(clock);
        continue;
      }
      else
        break;
      // if(pagedir_get_page(thread_current()->pagedir, fte->user_virtual_address) != NULL){      
      //   if(pagedir_is_accessed(thread_current()->pagedir,fte->user_virtual_address)){
      //     pagedir_set_accessed(thread_current()->pagedir,fte->user_virtual_address,false);
      //     continue;
      //   }
      //   else
      //     break;
      // }
    }
  // if(clock == NULL || clock == list_rbegin(&frame_table))
  //   clock = list_begin(&frame_table);
  // else
  //   clock = list_next(clock);  
  //printf("ddddd\n");
  //printf("%x\n", fte->user_virtual_address);
  pagedir_clear_page(fte->thread->pagedir, fte->user_virtual_address);

  int swap_index = swap_out(fte->kernel_virtual_address);

  struct supplement_page_table_entry spte;
  spte.upage = pg_round_down(fte->user_virtual_address);
  struct hash_elem *elem = hash_find(&fte->thread->supplement_page_table, &spte.elem);
  if (elem == NULL)
    return false;
  struct supplement_page_table_entry * spte2;
  spte2 = hash_entry (elem, struct supplement_page_table_entry, elem); 
  spte2->swap_index = swap_index;
  spte2->status = 2;
  spte2->on_frame = false;
  spte2->kpage = NULL;

  if(pagedir_is_dirty(fte->thread->pagedir, spte2->upage))
    spte2->dirty = true;

  list_remove(&fte->elem);
  palloc_free_page(fte->kernel_virtual_address);
  void * kpage = palloc_get_page(flags);
  free(fte);

  //lock_release(&frame_lock);

  return kpage;
}


void frame_table_entry_set_pin_true(void * kpage){
  lock_acquire (&frame_lock);
  struct list_elem * e;
  for (e = list_begin (&frame_table); e != list_end (&frame_table);
       e = list_next (e))
    {
      struct frame_table_entry * fte  = list_entry (e, struct frame_table_entry, elem);
      if(fte->kernel_virtual_address == kpage){
          fte->pinning = true;
          lock_release (&frame_lock);
          return;
      }
    }
  lock_release (&frame_lock);
}

void frame_table_entry_set_pin_false(void * kpage){
  lock_acquire (&frame_lock);
  struct list_elem * e;
  for (e = list_begin (&frame_table); e != list_end (&frame_table);
       e = list_next (e))
    {
      struct frame_table_entry * fte  = list_entry (e, struct frame_table_entry, elem);
      if(fte->kernel_virtual_address == kpage){
          fte->pinning = false;
          lock_release (&frame_lock);
          return;
      }
    }
  lock_release (&frame_lock);
=======
    PANIC("dd");
}

void frame_table_entry_evict(void){
  PANIC("??");
  return;
>>>>>>> a7411f174dcd283c0088ab0e1fe146a560c06410
}