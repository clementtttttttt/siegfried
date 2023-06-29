typedef unsigned long (*diskman_read_func_t) (unsigned long inode, unsigned long off_sects, unsigned long num_sects, void* buf);
typedef unsigned long (*diskman_write_func_t) (unsigned long inode, unsigned long off_sects, unsigned long num_sects, void* buf);

#define DISKMAN_READ_FUNC(name) unsigned long name (unsigned long id, unsigned long off_sects, unsigned long num_sects, void* buf)
#define DISKMAN_WRITE_FUNC(name) unsigned long name (unsigned long id, unsigned long off_sects, unsigned long num_sects, void* buf)

typedef struct diskman_inode{

    unsigned int block_ptr[12];

    unsigned int ptr_to_block_ptrs; //BLOCK_SZ_IN_BS / 4

    unsigned int ptr_to_ptrs_to_block_ptrs;

    unsigned int ptr_to_ptrs_to_ptrs_to_block_ptrs;


}diskman_inode;

typedef struct siegfried_diskman_dirent{

    char name[256];
    unsigned long fs_inode;
    unsigned long parent_dir_inode;

}siegfried_diskman_dirent;

typedef struct siegfried_file_ent{

    diskman_inode *inode;
    char name[256];

}siegfried_file_ent;

typedef unsigned long (*diskman_open_func_t) (unsigned long dm_inode, char *path, unsigned long attrs);
typedef unsigned long (*diskman_readfile_func_t) (unsigned long dm_inode, char *path, unsigned long attrs);

#define DISKMAN_OPEN_DIR_FUNC(name) unsigned long name (unsigned long dm_inode , char *path, siegfried_diskman_dirent *ent);
#define DISKMAN_GET_NEXT_IN_DIR_FUNC(name) unsigned long name (unsigned long dm_inode, siegfried_diskman_dirent *ent);
#define DISKMAN_OPEN(name) unsigned long name(unsigned long dm_inode, char *path, unsigned long attrs)


typedef struct diskman_fs_driver_funcs{



}diskman_fs_driver_funcs;

typedef struct diskman_ent{
    struct diskman_ent *next;

    char *uuid; //MBR/GPT IDENTIFIER
    int uuid_len;

    unsigned long inode;

    diskman_read_func_t read_func;
    diskman_write_func_t write_func;

    diskman_fs_driver_funcs *fs_driver;

    void *fs_disk_info;

    unsigned long fs_type;

    char ispart;

}diskman_ent;

diskman_ent *diskman_new_ent();
void diskman_setup();

void diskman_gpt_enum(diskman_ent *in);

diskman_ent *diskman_find_ent(unsigned long inode);


enum fs_types{
    DISKMAN_FS_NULL, DISKMAN_FS_EXTFS, DISKMAN_FS_FAT
};


#include "diskman_gpt.h"

#include "extfs.h"
