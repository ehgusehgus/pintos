#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include "filesys/filesys.h"
#include "lib/kernel/list.h"
<<<<<<< HEAD
#include "filesys/directory.h"
=======
>>>>>>> a7411f174dcd283c0088ab0e1fe146a560c06410

struct file_descriptor
{
	int fd;
	struct file * file;
<<<<<<< HEAD
	struct dir * dir;
	struct list_elem elem;
};

struct mmap_descriptor {
  int id;
  void *addr;
  struct file* file;
  size_t size;
  struct list_elem elem;
};

struct lock syscall_lock;
=======
	struct list_elem elem;
};

>>>>>>> a7411f174dcd283c0088ab0e1fe146a560c06410

void syscall_init (void);
void our_munmap(int mmap_id);

#endif /* userprog/syscall.h */
