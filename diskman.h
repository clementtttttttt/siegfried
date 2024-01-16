#ifndef _SYS_DISKMAN_H
#define _SYS_DISKMAN_H



typedef  long (*diskman_read_func_t) (unsigned long inode, unsigned long off_sects, unsigned long num_sects, void* buf);
typedef  long (*diskman_write_func_t) (unsigned long inode, unsigned long off_sects, unsigned long num_sects, void* buf);

#define DISKMAN_READ_FUNC(name)  long name (unsigned long id, unsigned long off_bytes, unsigned long num_bytes, void* buf)
#define DISKMAN_WRITE_FUNC(name)  long name (unsigned long id, unsigned long off_bytes, unsigned long num_bytes, void* buf)


typedef struct siegfried_dir{

    struct siegfried_file *files;
    unsigned long num_files;
	unsigned long inode;
	char name[256];

}siegfried_dir;

enum diskman_file_tees{
    DISKMAN_T_NULL, DISKMAN_T_REG, DISKMAN_T_DIR, DISKMAN_T_SPEC
};



typedef siegfried_dir* (*diskman_open_dir_t) (unsigned long dm_inode, char *path, unsigned long attrs);

typedef unsigned long (*diskman_fread_t) (struct siegfried_file *f, void *buf, unsigned long off, unsigned long bytes, unsigned long attrs);

typedef unsigned long (*diskman_fwrite_t) (struct siegfried_file *f, void *buf, unsigned long off, unsigned long bytes, unsigned long attrs);

typedef struct siegfried_file* (*diskman_fopen_t) (unsigned long disk_id, char *path);

#define DISKMAN_OPEN_DIR_FUNC(name) siegfried_dir* name (unsigned long dm_inode, char *path, unsigned long attrs)

#define DISKMAN_FWRITE_FUNC(name) unsigned long name (siegfried_file *f, void *buf, unsigned long off, unsigned long bytes, unsigned long attrs)
#define DISKMAN_FREAD_FUNC(name) unsigned long name (siegfried_file *f, void *buf, unsigned long off, unsigned long bytes, unsigned long attrs)
#define DISKMAN_FOPEN_FUNC(name) siegfried_file* name (unsigned long disk_id, char *path)

typedef struct siegfried_stat{
	
	unsigned long	perms;
	unsigned long 	inode;
	unsigned long	disk_inode;
	unsigned long	links;
	unsigned long	uid;
	unsigned long	gid;
	unsigned long	size;
	unsigned long	atime_in_ms;
	unsigned long	mtime_in_ms;
	unsigned long	ctime_in_ms;
} siegfried_stat;

typedef struct diskman_ent{
    struct diskman_ent *next;

    char *uuid; //MBR/GPT IDENTIFIER
    int uuid_len;

    unsigned long inode;

    diskman_read_func_t read_func;
    diskman_write_func_t write_func;

     diskman_open_dir_t fopendir;
    diskman_fread_t fread;
	diskman_fwrite_t fwrite;
	diskman_fopen_t fopen;

    void *fs_disk_info;

    unsigned long fs_type;

    char ispart;

}diskman_ent;


typedef struct siegfried_file{

	diskman_ent *disk;
    unsigned long inode;
    char name[256];

    unsigned short t; // file type(eg block device, char dev , dir , etc blab l

    void* fs_spec_dat;

}siegfried_file;


diskman_ent *diskman_new_ent();
void diskman_setup();

void diskman_gpt_enum(diskman_ent *in);

diskman_ent *diskman_find_ent(unsigned long inode);


enum fs_types{
    DISKMAN_FS_NULL, DISKMAN_FS_EXTFS, DISKMAN_FS_FAT
};


#include "diskman_gpt.h"

#include "extfs.h"

#endif
