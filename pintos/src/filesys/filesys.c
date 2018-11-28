#include "filesys/filesys.h"
#include <debug.h>
#include <stdio.h>
#include <string.h>
#include "filesys/file.h"
#include "filesys/free-map.h"
#include "filesys/inode.h"
#include "filesys/directory.h"
#include "filesys/cache.h"
#include "threads/thread.h"

/* Partition that contains the file system. */
struct block *fs_device;

static void do_format (void);

/* Initializes the file system module.
   If FORMAT is true, reformats the file system. */
void
filesys_init (bool format) 
{
  fs_device = block_get_role (BLOCK_FILESYS);
  if (fs_device == NULL)
    PANIC ("No file system device found, can't initialize file system.");
  cache_init();

  inode_init ();
  free_map_init ();

  if (format) 
    do_format ();

  free_map_open ();
}

/* Shuts down the file system module, writing any unwritten data
   to disk. */
void
filesys_done (void) 
{
  free_map_close ();
  //printf("%d\n", inode_get_isdir(inode_open(0)));
  cache_remove();
}


/* Creates a file named NAME with the given INITIAL_SIZE.
   Returns true if successful, false otherwise.
   Fails if a file named NAME already exists,
   or if internal memory allocation fails. */
bool
filesys_create (const char *name, off_t initial_size, int is_dir) 
{
  block_sector_t inode_sector = 0;
  char path[strlen(name)+1];
  char file_name[strlen(name)+1];
  parse_path(name, path, file_name);

  struct dir *dir = dir_open_sub(path);
  if(dir == NULL)
    return NULL;
  bool success = (dir != NULL
                  && free_map_allocate (1, &inode_sector)
                  && inode_create (inode_sector, initial_size, is_dir)
                  && dir_add (dir, file_name, inode_sector));

  if (!success && inode_sector != 0) 
    free_map_release (inode_sector, 1);
  dir_close (dir);

  return success;
}

/* Opens the file with the given NAME.
   Returns the new file if successful or a null pointer
   otherwise.
   Fails if no file named NAME exists,
   or if an internal memory allocation fails. */
struct file *
filesys_open (const char *name)
{
<<<<<<< HEAD
  char path[strlen(name)+1];
  char file_name[strlen(name)+1];
  parse_path(name, path, file_name);
  struct dir *dir = dir_open_sub(path);
  // printf("%s %s\n", path, file_name);
  // printf("%d\n",strcmp(file_name,"."));

  if(dir == NULL){
    return NULL;
  }
=======

  struct dir *dir = dir_open_root ();
>>>>>>> a7411f174dcd283c0088ab0e1fe146a560c06410
  struct inode *inode = NULL;

  // if(dir != NULL && strcmp(file_name,".") == 0){
  //   inode = dir_get_inode(dir);
  // }
  // if(dir != NULL && strcmp(file_name,"..") == 0){
  //   inode = dir_get_parent_inode(dir);
  // }
 
  if (dir != NULL && strlen(file_name)!=0){
    dir_lookup (dir, file_name, &inode);
  }
  else if(strlen(name)!=0){
    inode = dir_get_inode(dir);
  }
  dir_close(dir);

  if(inode == NULL || inode_get_removed(inode))
    return NULL;

  return file_open (inode);
}

/* Deletes the file named NAME.
   Returns true if successful, false on failure.
   Fails if no file named NAME exists,
   or if an internal memory allocation fails. */
bool
filesys_remove (const char *name) 
{
  char path[strlen(name)+1];
  char file_name[strlen(name)+1];
  parse_path(name, path, file_name);
  struct dir *dir = dir_open_sub(path);
  if(dir == NULL){
    return false;
  }
  bool success = dir != NULL && dir_remove (dir, file_name);
  dir_close (dir); 
  return success;
}

/* Formats the file system. */
static void
do_format (void)
{
  printf ("Formatting file system...");
  free_map_create ();
  if (!dir_create (ROOT_DIR_SECTOR, 16))
    PANIC ("root directory creation failed");
  free_map_close ();
  printf ("done.\n");
}


bool
filesys_chdir(const char *name){
  struct dir * dir = dir_open_sub(name);
  if(dir == NULL){
    return false;
  }
  dir_close(thread_current()->cur_dir);
  thread_current()->cur_dir = dir;
  return true;
}