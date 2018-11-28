#ifndef VM_PAGE_H
#define VM_PAGE_H

#include "lib/kernel/hash.h"

struct supplement_page_table_entry {
  struct hash_elem elem;
  void *upage;
<<<<<<< HEAD
  void *kpage;
=======
>>>>>>> a7411f174dcd283c0088ab0e1fe146a560c06410
  int status;
  int on_frame;
  struct file * file;
  int ofs;
  int read_bytes;
  int zero_bytes;
  int writeable;
<<<<<<< HEAD

  int swap_index;

  bool dirty;
=======
>>>>>>> a7411f174dcd283c0088ab0e1fe146a560c06410
};

unsigned spt_hash_func(struct hash_elem *hash_elem, void *aux);
bool spt_less_func(struct hash_elem *a_, struct hash_elem *b_, void *aux);
void spt_action_function(struct hash_elem * elem, void *aux);
void supplement_page_table_init(void);
void supplement_page_table_destroy(void);
<<<<<<< HEAD
bool supplement_page_table_insert(void *upage,void * kpage , int status, struct file * file, int ofs, int read_bytes, int zero_bytes, bool writeable);
bool load_from_supplement_page_table(void *upage, bool pinning);
void supplement_page_table_mm_unmap(void *upage, struct file *f);
bool supplement_page_table_has_entry(void *upage);
=======
bool supplement_page_table_insert(void *upage, int status, struct file * file, int ofs, int read_bytes, int zero_bytes, bool writeable);
bool load_from_supplement_page_table(void *upage);
>>>>>>> a7411f174dcd283c0088ab0e1fe146a560c06410
#endif