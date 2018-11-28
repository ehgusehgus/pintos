#include "lib/kernel/hash.h"
#include "vm/page.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/palloc.h"
#include "threads/malloc.h"
<<<<<<< HEAD
#include "vm/frame.h"
#include "userprog/syscall.h"
=======
>>>>>>> a7411f174dcd283c0088ab0e1fe146a560c06410

struct lock page_lock;

unsigned spt_hash_func(struct hash_elem *hash_elem, void *aux UNUSED){
  struct supplement_page_table_entry *spte = hash_entry(hash_elem, struct supplement_page_table_entry, elem);
  return hash_int(spte->upage);
}

bool spt_less_func(struct hash_elem *a_, struct hash_elem *b_, void *aux UNUSED){
  struct supplement_page_table_entry *a = hash_entry(a_, struct supplement_page_table_entry, elem);
  struct supplement_page_table_entry *b = hash_entry(b_, struct supplement_page_table_entry, elem);

  return a->upage < b->upage;
}

void spt_action_function(struct hash_elem * elem, void *aux UNUSED){
  struct supplement_page_table_entry * spte = hash_entry(elem, struct supplement_page_table_entry, elem);
<<<<<<< HEAD
  //if (pagedir_get_page(thread_current()->pagedir, spte->upage) != NULL){
  //  frame_table_entry_free(pagedir_get_page(thread_current()->pagedir, spte->upage));
  //}
  if(spte->on_frame == true)
    only_frame_table_entry_free(spte->kpage);
  else{
    if(spte->status == 2)
      swap_free(spte->swap_index);
  }
=======
>>>>>>> a7411f174dcd283c0088ab0e1fe146a560c06410
  free(spte);
}

void supplement_page_table_init(void){
  hash_init(&thread_current()->supplement_page_table,spt_hash_func,spt_less_func, NULL);
  lock_init(&page_lock);
}

void supplement_page_table_destroy(void){
  hash_destroy(&thread_current()->supplement_page_table, spt_action_function);
}

<<<<<<< HEAD
bool supplement_page_table_has_entry(void *upage){
  struct supplement_page_table_entry spte;
  spte.upage = pg_round_down(upage);
  struct hash_elem *elem = hash_find(&thread_current()->supplement_page_table, &spte.elem);
  if (elem == NULL){
    return false;
  } else {
    return true;
  }
}

bool supplement_page_table_insert(void *upage,void *kpage ,int status ,struct file * file, int ofs, int read_bytes, int zero_bytes, bool writeable){
=======
bool supplement_page_table_insert(void *upage, int status ,struct file * file, int ofs, int read_bytes, int zero_bytes, bool writeable){
>>>>>>> a7411f174dcd283c0088ab0e1fe146a560c06410
  struct supplement_page_table_entry *spte = (struct supplement_page_table_entry *) malloc(sizeof(struct supplement_page_table_entry));
  if (spte == NULL){
    return false;
  }
<<<<<<< HEAD
  spte->upage = upage;
  spte->kpage = kpage;
=======
  //spte->status = ON_FRAME;
  spte->upage = upage;
>>>>>>> a7411f174dcd283c0088ab0e1fe146a560c06410
  spte->status = status;
  spte->file = file;
  spte->ofs = ofs;
  spte->read_bytes = read_bytes;
  spte->zero_bytes = zero_bytes;
<<<<<<< HEAD
  if(upage == PHYS_BASE - PGSIZE){
    spte->on_frame = true;
  }
  else
      spte->on_frame = false;
  spte->writeable = writeable;
  spte->dirty = false;

  struct hash_elem *elem;
  //lock_acquire(&page_lock);
  elem = hash_insert(&thread_current()->supplement_page_table, &spte->elem);
  //lock_release(&page_lock);
=======
  spte->on_frame = false;
  spte->writeable = writeable;

  struct hash_elem *elem;
  lock_acquire(&page_lock);
  elem = hash_insert(&thread_current()->supplement_page_table, &spte->elem);
  lock_release(&page_lock);
>>>>>>> a7411f174dcd283c0088ab0e1fe146a560c06410
  if(elem != NULL){
    free(spte);
    return false;
  }
  return true;
}

<<<<<<< HEAD
bool load_from_supplement_page_table(void *upage, bool pinning){
  struct supplement_page_table_entry spte;
  spte.upage = pg_round_down(upage);
  struct hash_elem *elem = hash_find(&thread_current()->supplement_page_table, &spte.elem);

  if (elem == NULL){
    return false;
  }

  struct supplement_page_table_entry * spte2;
  spte2 = hash_entry (elem, struct supplement_page_table_entry, elem);

  if (spte2->status == 1){ //file
    /* Get a page of memory. */
    uint8_t *kpage = frame_alloc(PAL_USER, pg_round_down(upage));
=======
bool load_from_supplement_page_table(void *upage){
  struct supplement_page_table_entry spte;
  spte.upage = pg_round_down(upage);
  struct hash_elem *elem = hash_find(&thread_current()->supplement_page_table, &spte.elem);
  if (elem == NULL)
    return false;
  struct supplement_page_table_entry * spte2;
  spte2 = hash_entry (elem, struct supplement_page_table_entry, elem);
  if (spte2->status == 1){ //file
    /* Get a page of memory. */
    uint8_t *kpage = frame_alloc(PAL_USER);
>>>>>>> a7411f174dcd283c0088ab0e1fe146a560c06410
    if (kpage == NULL)
      return false;

    /* Load this page. */
<<<<<<< HEAD
    // if (syscall_lock.holder){
    //   printf("%d\n", syscall_lock.holder->tid);
    // }

    lock_acquire(&syscall_lock);

    if (file_read_at(spte2->file, kpage, spte2->read_bytes, spte2->ofs) != (int) spte2->read_bytes)
      {
        lock_release(&syscall_lock);
        frame_table_entry_free (kpage);
        return false; 
      }

    lock_release(&syscall_lock);

=======
    if (file_read_at(spte2->file, kpage, spte2->read_bytes, spte2->ofs) != (int) spte2->read_bytes)
      {
        frame_table_entry_free (kpage);
        return false; 
      }
>>>>>>> a7411f174dcd283c0088ab0e1fe146a560c06410
    memset (kpage + spte2->read_bytes, 0, spte2->zero_bytes);

    /* Add the page to the process's address space. */
    if (!install_page (spte2->upage, kpage, spte2->writeable)) 
      {
        frame_table_entry_free (kpage);
        return false; 
      }
    spte2->on_frame = true;
<<<<<<< HEAD
    spte2->kpage = kpage;
    if(!pinning)
      frame_table_entry_set_pin_false(kpage);
=======
>>>>>>> a7411f174dcd283c0088ab0e1fe146a560c06410
    return true;
  }
  else if (spte2->status == 2){
    //swap
<<<<<<< HEAD
    if(spte2->on_frame == true)
      return false;
    uint8_t *kpage = frame_alloc(PAL_USER, pg_round_down(upage));
    if (kpage == NULL)
      return false;
    swap_in(spte2->swap_index, kpage);
    if (!install_page (spte2->upage, kpage, spte2->writeable)) 
      {
        frame_table_entry_free (kpage);
        return false; 
      }
    spte2->on_frame = true;
    spte2->kpage = kpage;
    if(!pinning)
      frame_table_entry_set_pin_false(kpage);
    return true;
  }
  else if (spte2->status == 3){
    if(spte2->on_frame == true)
      return false;
    uint8_t *kpage = frame_alloc(PAL_USER |PAL_ZERO, pg_round_down(upage));
    if (kpage == NULL)
      return false;
    if (!install_page (spte2->upage, kpage, spte2->writeable)) 
      {
        frame_table_entry_free (kpage);
        return false; 
      }
    spte2->on_frame = true;
    spte2->kpage = kpage;
    if(!pinning)
      frame_table_entry_set_pin_false(kpage);
    return true;
  }
  return false;
}

void
supplement_page_table_mm_unmap(void *upage, struct file *f){
  struct supplement_page_table_entry spte;
  spte.upage = upage;
  struct hash_elem *elem = hash_find(&thread_current()->supplement_page_table, &spte.elem);

  struct supplement_page_table_entry *spte2;
  spte2 = hash_entry(elem, struct supplement_page_table_entry, elem);

  if(spte2->on_frame == true){
    if(pagedir_is_dirty(thread_current()->pagedir, spte2->upage)) {
      if(spte2->read_bytes != PGSIZE){
        file_write_at(f, spte2->upage, spte2->read_bytes, spte2->ofs);
      }
      else {
        file_write_at(f, spte2->upage, PGSIZE, spte2->ofs);
      }
    }

    frame_table_entry_free (spte2->kpage);
    pagedir_clear_page (thread_current()->pagedir, spte2->upage);
  }
  else if(spte2->status == 1){

  }
  else if(spte2->status == 2){
    if(spte2->dirty){
      void *inter = palloc_get_page(0);
      swap_in(spte2->swap_index, inter);
      file_write_at(f,inter,PGSIZE,spte2->ofs);
      palloc_free_page(inter);
    }
    else
      swap_free(spte2->swap_index);
  }

  hash_delete(&thread_current()->supplement_page_table, &spte2->elem);
}


=======
    return false;
  }
  else if (spte2->status == 3){
    //zeros 
    return false;
  }
  return false;
}
>>>>>>> a7411f174dcd283c0088ab0e1fe146a560c06410
