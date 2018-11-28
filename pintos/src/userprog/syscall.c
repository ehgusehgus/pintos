#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/palloc.h"
#include "userprog/process.h"
#include "devices/shutdown.h"
#include "threads/synch.h"
#include "filesys/filesys.h"
<<<<<<< HEAD
#include "filesys/file.h"
#include "vm/frame.h"
#include "vm/page.h"
#include "threads/malloc.h"

static void syscall_handler (struct intr_frame *);

static struct lock fd_lock;

void
hold_load(void * buffer, size_t size){
  //enum intr_level old_level;
  void * upage;
  //old_level = intr_disable();
  for (upage = pg_round_down(buffer); buffer+size > upage; upage+=PGSIZE){
    //old_level = intr_disable();
    uint8_t * esp = thread_current()->cur_esp;
    bool is_stack_fault, in_stack;
    in_stack= (upage < PHYS_BASE && PHYS_BASE - 8*1024*1024 <= upage);
    is_stack_fault= (upage == esp - 4 || upage == esp - 32) || upage >= esp ;
    if (in_stack&& is_stack_fault) {
      supplement_page_table_insert(pg_round_down(upage), NULL,3, NULL, NULL, NULL, NULL,true);
    }

    if(load_from_supplement_page_table(upage,true)){
    }

    //lock_acquire (&frame_lock);
    
    // struct list_elem * e;
    // for (e = list_begin (&frame_table); e != list_end (&frame_table);
    //      e = list_next (e))
    //   {
    //     struct frame_table_entry * fte  = list_entry (e, struct frame_table_entry, elem);
    //     if(fte->user_virtual_address == upage && fte->thread == thread_current()){
    //         fte->pinning = true;
    //         break;
    //     }
    //   }
    //lock_release (&frame_lock);
    //intr_set_level(old_level);
  }
  //intr_set_level(old_level);
}

void
release_hold(void *buffer, int size){
  enum intr_level old_level;
  void * upage;
  //old_level = intr_disable();
  for (upage = pg_round_down(buffer); buffer+size > upage; upage+=PGSIZE){
    //old_level = intr_disable();
    lock_acquire (&frame_lock);
    struct list_elem * e;
    for (e = list_begin (&frame_table); e != list_end (&frame_table);
         e = list_next (e))
      {
        struct frame_table_entry * fte  = list_entry (e, struct frame_table_entry, elem);
        if(fte->user_virtual_address == upage && fte->thread == thread_current()){
            fte->pinning = false;
            break;
        }
      }
    lock_release (&frame_lock);
    //intr_set_level(old_level);
  }
  //intr_set_level(old_level);
}
=======

static void syscall_handler (struct intr_frame *);

static struct lock syscall_lock;
static struct lock fd_lock;

>>>>>>> a7411f174dcd283c0088ab0e1fe146a560c06410

void
syscall_init (void) 
{
  lock_init(&syscall_lock);
  lock_init(&fd_lock);
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

///////////// helper functions to access user memory
static int
get_user(const uint8_t *uaddr){
  if(is_user_vaddr(uaddr)){
    int result;
    asm("movl $1f, %0; movzbl %1, %0; 1:" : "=&a" (result) : "m" (*uaddr));
    return result;
  }
  return -1;
}

static int
get_user_many(const uint8_t *start, int how_many, void *destination){
  int cnt;

  for(cnt=0; cnt<how_many; cnt++){
    int result = get_user(start+cnt);
    if(result == -1){
      our_exit(-1);
    }
    *((char *)destination + cnt) = result;
  }
  return 0;
}

static bool
put_user (uint8_t *udst, uint8_t byte)
{
  if(is_user_vaddr(udst)){
    int error_code;
    asm ("movl $1f, %0; movb %b2, %1; 1:"
         : "=&a" (error_code), "=m" (*udst) : "q" (byte));
    return error_code != -1;
  }
  return -1;
}

int
allocate_fd (void) 
{
  static int next_fd = 2;
  int fd;
  lock_acquire (&fd_lock);
  fd = next_fd++;
  lock_release (&fd_lock);
  return fd;
}

struct file *
find_file_using_fd(int input_fd) {
  struct list_elem *e;
  struct thread * cur = thread_current();
  for (e = list_begin (&cur->open_file_list); e != list_end (&cur->open_file_list);
       e = list_next (e))
    {
      struct file_descriptor * f = list_entry (e, struct file_descriptor, elem);
      if(f->fd == input_fd){
        return f->file;
      }
    }
    return NULL;
}

struct file_descriptor *
find_descriptor_using_fd(int input_fd) {
  struct list_elem *e;
  struct thread * cur = thread_current();
  for (e = list_begin (&cur->open_file_list); e != list_end (&cur->open_file_list);
       e = list_next (e))
    {
      struct file_descriptor * f = list_entry (e, struct file_descriptor, elem);
      if(f->fd == input_fd){
        return f;
      }
    }
    return NULL;
}

<<<<<<< HEAD
struct mmap_descriptor *
find_mmap_descriptor(int mmap_id){
  struct list_elem *e;
  if (!list_empty(&thread_current()->mmap_descriptor_list)) {
    for(e = list_begin(&thread_current()->mmap_descriptor_list);
        e != list_end(&thread_current()->mmap_descriptor_list); e = list_next(e))
    {
      struct mmap_descriptor *mmap_descriptor = list_entry(e, struct mmap_descriptor, elem);
      if(mmap_descriptor->id == mmap_id) {
        return mmap_descriptor;
      }
    }
  }
}

=======
>>>>>>> a7411f174dcd283c0088ab0e1fe146a560c06410
///////////////////////////////////////////////////

void
our_halt(){
  shutdown_power_off();
}

void
our_exit(int status){
  if(lock_held_by_current_thread(&syscall_lock))
    lock_release(&syscall_lock);

  printf("%s: exit(%d)\n", thread_current()->name, status);

  lock_acquire(&syscall_lock);
  if(thread_current()->exec_file != NULL){
    file_close(thread_current()->exec_file);
  }
  thread_current()->exit_status = status;
<<<<<<< HEAD

=======
>>>>>>> a7411f174dcd283c0088ab0e1fe146a560c06410
  lock_release(&syscall_lock);
  thread_exit();
}

int
our_wait(tid_t tid){
  //lock_acquire(&syscall_lock);
  int x = process_wait(tid);
  //lock_release(&syscall_lock);
  return x;
}

tid_t
our_exec(const char *file){
  lock_acquire(&syscall_lock);
  tid_t x = process_execute(file);
  lock_release(&syscall_lock);
  return x;
}

bool
our_create(const char *file, unsigned initial_size){
  lock_acquire(&syscall_lock);
<<<<<<< HEAD
  bool success = filesys_create(file, initial_size, false);
=======
  bool success = filesys_create(file, initial_size);
>>>>>>> a7411f174dcd283c0088ab0e1fe146a560c06410
  lock_release(&syscall_lock);
  return success;
}

bool
our_remove(const char *file){
  lock_acquire(&syscall_lock);
  bool success = filesys_remove(file);
  lock_release(&syscall_lock);
  return success;
}

int
our_open(const char * file){
  struct file * file_opened;
  int fd;
  struct file_descriptor * file_desc;
  lock_acquire(&syscall_lock);
  file_opened = filesys_open(file);
  if (file_opened == NULL){
    lock_release(&syscall_lock);
    return -1;
  }
  file_desc = palloc_get_page(0);
  if(file_desc == NULL){
    lock_release(&syscall_lock);
    return -1;
  }
  file_desc->fd = allocate_fd();
  file_desc->file = file_opened;
<<<<<<< HEAD
  if(inode_get_isdir(file_get_inode(file_opened)))
    file_desc->dir = dir_open(inode_reopen(file_get_inode(file_opened)));
  else
    file_desc->dir = NULL;

=======
>>>>>>> a7411f174dcd283c0088ab0e1fe146a560c06410
  list_push_front(&thread_current()->open_file_list, &file_desc->elem);
  lock_release(&syscall_lock);
  return file_desc->fd;
}

int
our_filesize(int fd){
  struct file * file_opened;
  file_opened = find_file_using_fd(fd);
  if (file_opened == NULL)
    return -1;
  else
  {
    int len;
    lock_acquire(&syscall_lock);
    len = file_length(file_opened);
    lock_release(&syscall_lock);
    return len;
  }
}

int
our_read(int fd, const void *buffer, unsigned size){
  struct file * file_opened;
  file_opened = find_file_using_fd(fd);
  if (file_opened == NULL)
    return -1;
  int len;
<<<<<<< HEAD
//#ifdef VM
  hold_load(buffer,size);
//#endif
  lock_acquire(&syscall_lock);
  len = file_read(file_opened, buffer, size);
  lock_release(&syscall_lock);
//#ifdef VM
  release_hold(buffer,size);
//#endif
=======
  lock_acquire(&syscall_lock);
  len = file_read(file_opened, buffer, size);
  lock_release(&syscall_lock);
>>>>>>> a7411f174dcd283c0088ab0e1fe146a560c06410
  return len;
}

int
our_write(int fd, const void *buffer, unsigned size){
  if (fd ==1){
    putbuf(buffer,size);
    return size;
  }
  else{
    struct file * file_opened;
    file_opened = find_file_using_fd(fd);
    if (file_opened == NULL)
      return -1;
<<<<<<< HEAD
    struct file_descriptor * descriptor = find_descriptor_using_fd(fd);
    if(descriptor->dir != NULL){
      return -1;
    }
    int len;
//#ifdef VM
    hold_load(buffer,size);
//#endif
    lock_acquire(&syscall_lock);
    len = file_write(file_opened, buffer, size);
    lock_release(&syscall_lock);
//#ifdef VM
    release_hold(buffer,size);
//#endif    
=======
    int len;
    lock_acquire(&syscall_lock);
    len = file_write(file_opened, buffer, size);
    lock_release(&syscall_lock);
>>>>>>> a7411f174dcd283c0088ab0e1fe146a560c06410
    return len;
  }
}

void
our_seek(int fd, unsigned position)
{
  struct file * file_opened;
  file_opened = find_file_using_fd(fd);
  if (file_opened == NULL)
    our_exit(-1);
  else
  {
    lock_acquire(&syscall_lock);
    file_seek(file_opened,position);
    lock_release(&syscall_lock);
  }
}
unsigned
our_tell(int fd)
{
  struct file * file_opened;
  file_opened = find_file_using_fd(fd);
  if (file_opened == NULL)
    return -1;
  else
  {
    int len;
    lock_acquire(&syscall_lock);
    len = file_tell(file_opened);
    lock_release(&syscall_lock);
    return len;
  }
}

void
our_close(int fd)
{
  struct file * file_opened;
  struct file_descriptor * descriptor = find_descriptor_using_fd(fd);
  file_opened = find_file_using_fd(fd);
  if (file_opened == NULL)
    our_exit(-1);
  else
  {
    lock_acquire(&syscall_lock);
    file_close(file_opened);
<<<<<<< HEAD
    if(descriptor->dir != NULL)
      dir_close(descriptor->dir);
=======
>>>>>>> a7411f174dcd283c0088ab0e1fe146a560c06410
    list_remove(&descriptor->elem);
    palloc_free_page(descriptor);
    lock_release(&syscall_lock);
  }
}

<<<<<<< HEAD

int
our_mmap(int fd, void *addr)
{
  if(fd <= 1){
    return -1;
  }
  if(addr == 0){
    return -1;
  }
  if(pg_ofs(addr) != 0){
    return -1;
  }

  lock_acquire(&syscall_lock);

  struct file_descriptor *file_descriptor = find_descriptor_using_fd(fd);
  struct file *file = find_file_using_fd(fd);
  struct file *f = NULL;
  if(file_descriptor){
    if(file){
      f = file_reopen(file);
    }
  }
  if(f == NULL){
    lock_release(&syscall_lock);
    return -1;
  }
  int file_size = file_length(f);
  if(file_size == 0){
    lock_release(&syscall_lock);
    return -1;
  }


  int i;
  for (i = 0; i < file_size; i += PGSIZE) {
    if (supplement_page_table_has_entry(addr + i)){
      lock_release(&syscall_lock);
      return -1;
    }
  }
  for (i = 0; i < file_size; i += PGSIZE) {
    int read_bytes = (i + PGSIZE < file_size ? PGSIZE : file_size - i);
    int zero_bytes = PGSIZE - read_bytes;
    supplement_page_table_insert(addr + i, NULL,1, f, i, read_bytes, zero_bytes, /*writable*/true);
  }


  struct mmap_descriptor *mmap_descriptor = (struct mmap_descriptor *) malloc(sizeof(struct mmap_descriptor));
  mmap_descriptor->addr = addr;
  mmap_descriptor->file = f;
  mmap_descriptor->size = file_size;
  if (list_empty(&thread_current()->mmap_descriptor_list)){
    mmap_descriptor->id = 1;
  } else {
    mmap_descriptor->id = list_entry(list_front(&thread_current()->mmap_descriptor_list), struct mmap_descriptor, elem)->id + 1;
  }

  list_push_front(&thread_current()->mmap_descriptor_list, &mmap_descriptor->elem);

  int mmap_id = mmap_descriptor->id;
  lock_release (&syscall_lock);
  return mmap_id;
}


void
our_munmap(int mmap_id){
  lock_acquire(&syscall_lock);

  struct mmap_descriptor *mmap_descriptor = find_mmap_descriptor(mmap_id);
  int file_size = mmap_descriptor->size;
  int i;
  for(i = 0; i < file_size; i += PGSIZE) {
    supplement_page_table_mm_unmap(mmap_descriptor->addr + i, mmap_descriptor->file);
  }
  list_remove(&mmap_descriptor->elem);
  file_close(mmap_descriptor->file);
  free(mmap_descriptor);

  lock_release(&syscall_lock);
}

bool
our_chdir(const char *dir){
  lock_acquire(&syscall_lock);
  bool success = filesys_chdir(dir);
  lock_release(&syscall_lock);
  return success;
}
bool
our_mkdir(const char *dir){
  lock_acquire(&syscall_lock);
  bool success = filesys_create(dir, 0, true);
  lock_release(&syscall_lock);
  return success;
}
bool
our_readdir(int fd, char *name){
  lock_acquire(&syscall_lock);
  struct file_descriptor * descriptor = find_descriptor_using_fd(fd);
  if(descriptor == NULL){
    lock_release(&syscall_lock);
    return false;
  }
  if(inode_get_isdir(file_get_inode(descriptor->file)) == false){
    lock_release(&syscall_lock);
    return false;
  }
  int success = dir_readdir(descriptor->dir,name);
  lock_release(&syscall_lock);
  return success;
}
bool
our_isdir(int fd){
  lock_acquire(&syscall_lock);
  struct file * file = find_file_using_fd(fd);
  if(file == NULL){
    lock_release(&syscall_lock);
    return false;
  }
  int success = inode_get_isdir(file_get_inode(file));
  lock_release(&syscall_lock);
  return success;
}
int
our_inumber(int fd){
  lock_acquire(&syscall_lock);
  struct file *file = find_file_using_fd(fd);
  if(file == NULL){
    lock_release(&syscall_lock);
    return false;
  }
  int ret = inode_get_inumber(file_get_inode(file));
  lock_release(&syscall_lock);
  return ret;
}
 
=======
>>>>>>> a7411f174dcd283c0088ab0e1fe146a560c06410
static void
syscall_handler (struct intr_frame *f) 
{
  int syscallnumber;
  get_user_many(f->esp, 4, &syscallnumber);
<<<<<<< HEAD
  thread_current()->cur_esp = f->esp;
=======

>>>>>>> a7411f174dcd283c0088ab0e1fe146a560c06410
  switch(syscallnumber){
    case SYS_HALT: // 0
    {                   /* Halt the operating system. */
      our_halt();
      break;
    }
    case SYS_EXIT: // 1
    {                   /* Terminate this process. */
      int status;
      get_user_many(f->esp+4, 4, &status);             
      our_exit(status);
      break;
    }
    case SYS_EXEC: // 2
    {                   /* Start another process. */
      char *file;
      get_user_many(f->esp+4, 4, &file);
      if(get_user(file) == -1){
        our_exit(-1);
      }
      else{
        f->eax = (uint32_t)our_exec(file);
      }

      break;
    }
    case SYS_WAIT: // 3
    {
      tid_t tid;
      get_user_many(f->esp+4, 4, &tid);
      f->eax = (uint32_t)our_wait(tid);
      break;
    }
    case SYS_CREATE: // 4
    {
      char *file;
      unsigned initial_size;
      get_user_many(f->esp+4, 4, &file);
      get_user_many(f->esp+8, 4, &initial_size);
      if(get_user(file) == -1){
        our_exit(-1);
      }
      else {
        f->eax = our_create(file, initial_size);
      }
      break;
    }
    case SYS_REMOVE: // 5
    {
      const char *file;
      get_user_many(f->esp+4, 4, &file);
      if(get_user(file) == -1){
        our_exit(-1);
      }
      else {
        f->eax = our_remove(file);
      }
      break;
    }
    case SYS_OPEN: // 6
    {
      const char *file;
      get_user_many(f->esp+4, 4, &file);
      if(get_user(file) == -1){
        our_exit(-1);
      }
      else {
        f->eax = our_open(file);
      }
      break;
    }
    case SYS_FILESIZE: // 7
    {
      int fd;
      get_user_many(f->esp+4, 4, &fd);
      f->eax = our_filesize(fd);
      break;
    }
    case SYS_READ:
    {
      int fd;
      void *buffer;
      unsigned size;
      get_user_many(f->esp+4, 4, &fd);
      get_user_many(f->esp+8, 4, &buffer);
      get_user_many(f->esp+12, 4, &size);
      if(get_user(buffer) == -1){
        our_exit(-1);
      }
      else if(buffer == NULL)
      {
        our_exit(-1);
      }
      else{
        f->eax = (uint32_t)our_read(fd, buffer, size);
      }
      break;      
    }
    case SYS_WRITE: // 9
    {
      int fd;
      void *buffer;
      unsigned size;
      get_user_many(f->esp+4, 4, &fd);
      get_user_many(f->esp+8, 4, &buffer);
      get_user_many(f->esp+12, 4, &size);

      if(get_user(buffer) == -1){
        our_exit(-1);
      }
      else{
        f->eax = our_write(fd, buffer, size);
      }
      break;
    }
    case SYS_SEEK:
    {
      int fd;
      unsigned position;
      get_user_many(f->esp+4, 4, &fd);
      get_user_many(f->esp+8, 4, &position);
      our_seek(fd, position);
      break;
    }
    case SYS_TELL:
    {
      int fd;
      get_user_many(f->esp+4, 4, &fd);
      f->eax = our_tell(fd);
      break;
    }
    case SYS_CLOSE:
    {
      int fd;
      get_user_many(f->esp+4, 4, &fd);
      our_close(fd);
      break;
    }
<<<<<<< HEAD
    case SYS_MMAP:
    {
      int fd;
      void *addr;
      get_user_many(f->esp + 4, 4, &fd);
      get_user_many(f->esp + 8, 4, &addr);
      f->eax = our_mmap(fd, addr);
      break;
    }
    case SYS_MUNMAP:
    {
      int mmap_id;
      get_user_many(f->esp + 4, 4, &mmap_id);
      our_munmap(mmap_id);
      break;
    }
    case SYS_CHDIR:
    {
      void *dir;
      get_user_many(f->esp + 4, 4, &dir);
      f->eax = our_chdir(dir);
      break;
    }                  
    case SYS_MKDIR:
    {
      void *dir;
      get_user_many(f->esp + 4, 4, &dir);
      f->eax = our_mkdir(dir);
      break;
    }                  
    case SYS_READDIR:
    {
      int fd;
      void *name;
      get_user_many(f->esp + 4, 4, &fd);
      get_user_many(f->esp + 8, 4, &name);
      f->eax = our_readdir(fd, name);
      break;
    }                
    case SYS_ISDIR:
    {
      int fd;
      get_user_many(f->esp+4, 4, &fd);
      f->eax = our_isdir(fd);
      break;
    }                  
    case SYS_INUMBER:
    {
      int fd;
      get_user_many(f->esp+4, 4, &fd);
      f->eax = our_inumber(fd);
      break;
    }                 
=======
>>>>>>> a7411f174dcd283c0088ab0e1fe146a560c06410
    default:
      break;
  }
}
