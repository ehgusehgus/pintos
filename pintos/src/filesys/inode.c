#include "filesys/inode.h"
#include <list.h>
#include <debug.h>
#include <round.h>
#include <string.h>
#include "filesys/filesys.h"
#include "filesys/free-map.h"
#include "threads/malloc.h"
#include "filesys/cache.h"

/* Identifies an inode. */
#define INODE_MAGIC 0x494e4f44

/* On-disk inode.
   Must be exactly BLOCK_SECTOR_SIZE bytes long. */
struct inode_disk
  {
    //block_sector_t start;               /* First data sector. */
    off_t length;                       /* File size in bytes. */
    int is_dir;
    block_sector_t direct[123];            
    block_sector_t indirect;
    block_sector_t doubly_indirect; 
    unsigned magic;                     /* Magic number. */
  };

/* Returns the number of sectors to allocate for an inode SIZE
   bytes long. */
static inline size_t
bytes_to_sectors (off_t size)
{
  return DIV_ROUND_UP (size, BLOCK_SECTOR_SIZE);
}

/* In-memory inode. */
struct inode 
  {
    struct list_elem elem;              /* Element in inode list. */
    block_sector_t sector;              /* Sector number of disk location. */
    int open_cnt;                       /* Number of openers. */
    bool removed;                       /* True if deleted, false otherwise. */
    int deny_write_cnt;                 /* 0: writes ok, >0: deny writes. */
    struct inode_disk data;             /* Inode content. */
  };

/* Returns the block device sector that contains byte offset POS
   within INODE.
   Returns -1 if INODE does not contain data for a byte at offset
   POS. */
static block_sector_t
byte_to_sector (const struct inode *inode, off_t pos) 
{
  ASSERT (inode != NULL);
  if (pos < inode->data.length){
    if(pos/BLOCK_SECTOR_SIZE<123){
      return inode->data.direct[pos/BLOCK_SECTOR_SIZE];
    }
    else if (pos/BLOCK_SECTOR_SIZE<251){ //123 + 128 
      block_sector_t indirect[128];
      cache_read(inode->data.indirect, indirect);
      block_sector_t index = indirect[pos/BLOCK_SECTOR_SIZE-123];
      return index;
    }
    else if (pos/BLOCK_SECTOR_SIZE <16635){ //128*128 + 252
      int first_index = (pos/BLOCK_SECTOR_SIZE - 251)/128;
      int second_index = (pos/BLOCK_SECTOR_SIZE - 251)%128;

      block_sector_t indirect[128];
      cache_read(inode->data.doubly_indirect, indirect);
      cache_read(indirect[first_index], indirect);

      block_sector_t index = indirect[second_index];
      return index;
    }
    else
      return -1;
  }
  else{
    return -1;
  }
}

static
bool index_allocate(int sectors,struct inode_disk * disk_inode){
  int i;
  static char zeros[BLOCK_SECTOR_SIZE];
  if(sectors<123){//direct
    for(i=0;i<sectors;i++){
      if(disk_inode->direct[i] == 0){
        free_map_allocate(1, &disk_inode->direct[i]);
        cache_write(disk_inode->direct[i],zeros);
      }
    }
  }
  else{
    for(i=0;i<123;i++){
      if(disk_inode->direct[i] == 0){
        free_map_allocate(1, &disk_inode->direct[i]);
        cache_write(disk_inode->direct[i],zeros);
      }
    }
  }
  sectors = sectors-123;

  if(sectors<=0){
    return true;
  }
  if(disk_inode->indirect ==0){
    free_map_allocate(1, &disk_inode->indirect);
    cache_write(disk_inode->indirect, zeros);
  }

  block_sector_t indirect_block[128];
  cache_read(disk_inode->indirect, indirect_block);
  if(sectors<128){//indirect
    for(i=0;i<sectors;i++){
      if(indirect_block[i] ==0){
        free_map_allocate(1, &indirect_block[i]);
        cache_write(indirect_block[i], zeros);
      }
    }
  }
  else{
    for(i=0;i<128;i++){
      if(indirect_block[i]==0){
        free_map_allocate(1, &indirect_block[i]);
        cache_write(indirect_block[i], zeros);
      }
    }
  }
  cache_write(disk_inode->indirect,indirect_block);  

  sectors = sectors-128;

  if(sectors<=0)
    return true;

  if(disk_inode->doubly_indirect ==0){
    free_map_allocate(1, &disk_inode->doubly_indirect);
    cache_write(disk_inode->doubly_indirect, zeros);
  }

  block_sector_t doubly_indirect_block[128];
  cache_read(disk_inode->doubly_indirect, doubly_indirect_block);

  int first_index = sectors/128;
  int second_index = sectors%128;

  if(first_index>=128){
    PANIC("Two large file");
    return false;
  }

  for(i=0;i<first_index;i++){
    if(doubly_indirect_block[i] ==0){ 
      free_map_allocate(1, &doubly_indirect_block[i]);
      cache_write(doubly_indirect_block[i], zeros);
    }

    block_sector_t doubly_indirect_block_inter[128];
    cache_read(doubly_indirect_block[i], doubly_indirect_block_inter);
    int j;
    for(j=0;j<128;j++){
      if(doubly_indirect_block_inter[i] == 0){
        free_map_allocate(1, &doubly_indirect_block_inter[j]);
        cache_write(doubly_indirect_block_inter[j], zeros);
      }
    }
    cache_write(doubly_indirect_block[i], doubly_indirect_block_inter);
  }
  if(doubly_indirect_block[first_index] == 0){
    free_map_allocate(1, &doubly_indirect_block[first_index]);
    cache_write(doubly_indirect_block[first_index], zeros);
  }

  block_sector_t doubly_indirect_block_inter[128];
  cache_read(doubly_indirect_block[first_index], doubly_indirect_block_inter);
  int j;
  for(j=0;j<second_index;j++){
    if(doubly_indirect_block_inter[j] == 0){
      free_map_allocate(1, &doubly_indirect_block_inter[j]);
      cache_write(doubly_indirect_block_inter[j], zeros);
    }
  }
  cache_write(doubly_indirect_block[first_index], doubly_indirect_block_inter);  
  
  cache_write(disk_inode->doubly_indirect, doubly_indirect_block);
  return true;
}

static
bool index_deallocate(int sectors, struct inode_disk * disk_inode){

  int i;
  if(sectors<123){//direct
    for(i=0;i<sectors;i++){
      free_map_release(disk_inode->direct[i],1);
    }
  }
  else{
    for(i=0;i<123;i++){
      free_map_release(disk_inode->direct[i],1);
    }
  }
  sectors = sectors-123;
  if(sectors<=0)
    return true;

  //free_map_allocate(1, &disk_inode->indirect);

  block_sector_t indirect_block[128];
  cache_read(disk_inode->indirect, indirect_block);

  if(sectors<128){//indirect
    for(i=0;i<sectors;i++){
      free_map_release(indirect_block[i], 1);
    }
  }
  else{
    for(i=0;i<128;i++){
      free_map_release(indirect_block[i], 1);
    }
  }
  free_map_release(disk_inode->indirect,1);

  sectors = sectors-128;
  if(sectors<=0)
    return true;

  //free_map_allocate(1, &disk_inode->doubly_indirect);
  block_sector_t doubly_indirect_block[128];
  cache_read(disk_inode->doubly_indirect, doubly_indirect_block);

  int first_index = sectors/128;
  int second_index = sectors%128;

  if(first_index>=128){
    PANIC("Two large file");
    return false;
  }

  for(i=0;i<first_index;i++){ 
    //free_map_allocate(1, &doubly_indirect_block[i]);

    block_sector_t doubly_indirect_block_inter[128];
    cache_read(doubly_indirect_block[i], doubly_indirect_block_inter);

    int j;
    for(j=0;j<128;j++){
      free_map_release(doubly_indirect_block_inter[j],1);
    }
    free_map_release(doubly_indirect_block[i],1);
  }
  //free_map_allocate(1, &doubly_indirect_block[first_index]);

  block_sector_t doubly_indirect_block_inter[128];
  cache_read(doubly_indirect_block[first_index], doubly_indirect_block_inter);
  int j;

  for(j=0;j<second_index;j++){
    free_map_release(doubly_indirect_block_inter[j],1);
  }
  free_map_release(doubly_indirect_block[first_index],1);
  
  free_map_release(disk_inode->doubly_indirect,1);

  return true;

}

/* List of open inodes, so that opening a single inode twice
   returns the same `struct inode'. */
static struct list open_inodes;

/* Initializes the inode module. */
void
inode_init (void) 
{
  list_init (&open_inodes);
}

/* Initializes an inode with LENGTH bytes of data and
   writes the new inode to sector SECTOR on the file system
   device.
   Returns true if successful.
   Returns false if memory or disk allocation fails. */
bool
inode_create (block_sector_t sector, off_t length, int is_dir)
{
  struct inode_disk *disk_inode = NULL;
  bool success = false;

  ASSERT (length >= 0);

  /* If this assertion fails, the inode structure is not exactly
     one sector in size, and you should fix that. */
  ASSERT (sizeof *disk_inode == BLOCK_SECTOR_SIZE);

  disk_inode = calloc (1, sizeof *disk_inode);
  if (disk_inode != NULL)
    {
      size_t sectors = bytes_to_sectors (length);
      disk_inode->length = length;
      disk_inode->magic = INODE_MAGIC;
      disk_inode->is_dir = is_dir;
      if (index_allocate (sectors, disk_inode)) 
        {
          //block_write (fs_device, sector, disk_inode);
          cache_write(sector, disk_inode);
          // if (sectors > 0) 
          //   {
          //     static char zeros[BLOCK_SECTOR_SIZE];
          //     size_t i;
              
          //     for (i = 0; i < sectors; i++) 
          //       //block_write (fs_device, disk_inode->start + i, zeros);
          //       cache_write(disk_inode->start + i, zeros);
          //   }
          success = true; 
        } 
      free (disk_inode);
    }
  return success;
}

/* Reads an inode from SECTOR
   and returns a `struct inode' that contains it.
   Returns a null pointer if memory allocation fails. */
struct inode *
inode_open (block_sector_t sector)
{
  struct list_elem *e;
  struct inode *inode;
  /* Check whether this inode is already open. */
  for (e = list_begin (&open_inodes); e != list_end (&open_inodes);
       e = list_next (e)) 
    {
      inode = list_entry (e, struct inode, elem);
      if (inode->sector == sector) 
        {
          inode_reopen (inode);
          return inode; 
        }
    }

  /* Allocate memory. */
  inode = malloc (sizeof *inode);
  if (inode == NULL)
    return NULL;

  /* Initialize. */
  list_push_front (&open_inodes, &inode->elem);
  inode->sector = sector;
  inode->open_cnt = 1;
  inode->deny_write_cnt = 0;
  inode->removed = false;
  
<<<<<<< HEAD
  //block_read (fs_device, inode->sector, &inode->data);
  cache_read(inode->sector, &inode->data);
=======
  block_read (fs_device, inode->sector, &inode->data);
>>>>>>> a7411f174dcd283c0088ab0e1fe146a560c06410
  
  
  return inode;
}

/* Reopens and returns INODE. */
struct inode *
inode_reopen (struct inode *inode)
{
  if (inode != NULL)
    inode->open_cnt++;
  return inode;
}

/* Returns INODE's inode number. */
block_sector_t
inode_get_inumber (const struct inode *inode)
{
  return inode->sector;
}
int 
inode_get_isdir(const struct inode *inode){
  return inode->data.is_dir;
}
bool
inode_get_removed(const struct inode *inode){
  return inode->removed;
}


/* Closes INODE and writes it to disk.
   If this was the last reference to INODE, frees its memory.
   If INODE was also a removed inode, frees its blocks. */
void
inode_close (struct inode *inode) 
{
  //printf("%d\n", inode_get_inumber(inode));
  /* Ignore null pointer. */
  if (inode == NULL)
    return;
  /* Release resources if this was the last opener. */
  if (--inode->open_cnt == 0)
    {
      /* Remove from inode list and release lock. */
      list_remove (&inode->elem);
      /* Deallocate blocks if removed. */
      if (inode->removed) 
        {
          index_deallocate(bytes_to_sectors (inode->data.length), &inode->data );          
          free_map_release (inode->sector, 1);
          //free_map_release (inode->data.start,
          //                  bytes_to_sectors (inode->data.length)); 
        }
      free (inode); 
    }
}

/* Marks INODE to be deleted when it is closed by the last caller who
   has it open. */
void
inode_remove (struct inode *inode) 
{
  ASSERT (inode != NULL);
  inode->removed = true;
}

/* Reads SIZE bytes from INODE into BUFFER, starting at position OFFSET.
   Returns the number of bytes actually read, which may be less
   than SIZE if an error occurs or end of file is reached. */
off_t
inode_read_at (struct inode *inode, void *buffer_, off_t size, off_t offset) 
{
  uint8_t *buffer = buffer_;
  off_t bytes_read = 0;
  uint8_t *bounce = NULL;

  while (size > 0) 
    {
      /* Disk sector to read, starting byte offset within sector. */
      block_sector_t sector_idx = byte_to_sector (inode, offset);
      int sector_ofs = offset % BLOCK_SECTOR_SIZE;

      /* Bytes left in inode, bytes left in sector, lesser of the two. */
      off_t inode_left = inode_length (inode) - offset;
      int sector_left = BLOCK_SECTOR_SIZE - sector_ofs;
      int min_left = inode_left < sector_left ? inode_left : sector_left;

      /* Number of bytes to actually copy out of this sector. */
      int chunk_size = size < min_left ? size : min_left;
      if (chunk_size <= 0){
        break;
      }

      if (sector_ofs == 0 && chunk_size == BLOCK_SECTOR_SIZE)
        {
          /* Read full sector directly into caller's buffer. */
          //block_read (fs_device, sector_idx, buffer + bytes_read);
          cache_read(sector_idx, buffer + bytes_read);
        }
      else 
        {
          /* Read sector into bounce buffer, then partially copy
             into caller's buffer. */
          if (bounce == NULL) 
            {
              bounce = malloc (BLOCK_SECTOR_SIZE);
              if (bounce == NULL)
                break;
            }
          //block_read (fs_device, sector_idx, bounce);
          cache_read(sector_idx, bounce);
          memcpy (buffer + bytes_read, bounce + sector_ofs, chunk_size);
        }
      
      /* Advance. */
      size -= chunk_size;
      offset += chunk_size;
      bytes_read += chunk_size;

    }
  free (bounce);

  return bytes_read;
}

/* Writes SIZE bytes from BUFFER into INODE, starting at OFFSET.
   Returns the number of bytes actually written, which may be
   less than SIZE if end of file is reached or an error occurs.
   (Normally a write at end of file would extend the inode, but
   growth is not yet implemented.) */
off_t
inode_write_at (struct inode *inode, const void *buffer_, off_t size,
                off_t offset) 
{
  const uint8_t *buffer = buffer_;
  off_t bytes_written = 0;
  uint8_t *bounce = NULL;

  if (inode->deny_write_cnt)
    return 0;

  if(byte_to_sector (inode, offset+size-1) == -1){
    index_allocate(bytes_to_sectors(offset+size) ,&inode->data);
    inode->data.length = offset+size;
    cache_write(inode->sector, &inode->data);
  }

  while (size > 0) 
    {
      /* Sector to write, starting byte offset within sector. */
      block_sector_t sector_idx = byte_to_sector (inode, offset);
      int sector_ofs = offset % BLOCK_SECTOR_SIZE;

      /* Bytes left in inode, bytes left in sector, lesser of the two. */
      off_t inode_left = inode_length (inode) - offset;
      int sector_left = BLOCK_SECTOR_SIZE - sector_ofs;
      int min_left = inode_left < sector_left ? inode_left : sector_left;

      /* Number of bytes to actually write into this sector. */
      int chunk_size = size < min_left ? size : min_left;
      if (chunk_size <= 0)
        break;

      if (sector_ofs == 0 && chunk_size == BLOCK_SECTOR_SIZE)
        {
          /* Write full sector directly to disk. */
          //block_write (fs_device, sector_idx, buffer + bytes_written);
          cache_write(sector_idx, buffer + bytes_written);
        }
      else 
        {
          /* We need a bounce buffer. */
          if (bounce == NULL) 
            {
              bounce = malloc (BLOCK_SECTOR_SIZE);
              if (bounce == NULL)
                break;
            }

          /* If the sector contains data before or after the chunk
             we're writing, then we need to read in the sector
             first.  Otherwise we start with a sector of all zeros. */
          if (sector_ofs > 0 || chunk_size < sector_left) 
            //block_read (fs_device, sector_idx, bounce);
            cache_read(sector_idx, bounce);
          else
            memset (bounce, 0, BLOCK_SECTOR_SIZE);
          memcpy (bounce + sector_ofs, buffer + bytes_written, chunk_size);
          //block_write (fs_device, sector_idx, bounce);
          cache_write(sector_idx, bounce);
        }

      /* Advance. */
      size -= chunk_size;
      offset += chunk_size;
      bytes_written += chunk_size;
    }
  free (bounce);

  return bytes_written;
}

/* Disables writes to INODE.
   May be called at most once per inode opener. */
void
inode_deny_write (struct inode *inode) 
{
  inode->deny_write_cnt++;
  ASSERT (inode->deny_write_cnt <= inode->open_cnt);
}

/* Re-enables writes to INODE.
   Must be called once by each inode opener who has called
   inode_deny_write() on the inode, before closing the inode. */
void
inode_allow_write (struct inode *inode) 
{
  ASSERT (inode->deny_write_cnt > 0);
  ASSERT (inode->deny_write_cnt <= inode->open_cnt);
  inode->deny_write_cnt--;
}

/* Returns the length, in bytes, of INODE's data. */
off_t
inode_length (const struct inode *inode)
{
  return inode->data.length;
}
