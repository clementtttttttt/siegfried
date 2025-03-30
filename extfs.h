#include "types.h"

typedef struct extfs_superblock{

    unsigned int num_inodes;
    unsigned int num_blks;

    unsigned int num_rsvd_blks;

    unsigned int num_free_blks;
    unsigned int num_free_inodes;

    unsigned int sb_blknum;
    unsigned int blksz; //NOTE: shift 1024 by this to get sz
    unsigned int fragsz;

    unsigned int num_blks_grp;
    unsigned int num_frags_grp;
    unsigned int num_inodes_grp;

    unsigned int t_lastmount;
    unsigned int t_lastwrite;
    unsigned short times_mounted_since_fsck;
    unsigned short times_mounted_til_fsck;

    unsigned short magic; //NOTE: its' 0xef53

    unsigned short state;
    unsigned short err_handle_type;

    unsigned short ver_minor;
    unsigned int t_last_fsck;
    unsigned int int_between_fskc;

    unsigned int os_id;
    unsigned int ver_major;

    unsigned short uid_of_rsvd_blk;
    unsigned short gid_of_rsvd_blk;

    unsigned int first_unrsvd_inode;

    unsigned short inode_struct_sz_b;
    unsigned short sb_blkgrp;

    unsigned int opt_flags;
    unsigned int req_flags;

    unsigned int ro_flags;

    __uint128_t uuid;
    char name[16];

    char last_path[64];

    unsigned int comp_algos;
    unsigned char blk_prealloc_file;
    unsigned char blk_prealloc_dir;

    unsigned short rsvd;

    __uint128_t j_uuid;
    unsigned int j_inode;
    unsigned int j_dev;
    unsigned int head_orphan_inode_list;

    unsigned int htree_hash_seed[4];
    unsigned char dir_hash_algo;
    unsigned char j_blk_ha_inode_blk_n_sz;

    unsigned short bgd_sz_b;

    unsigned int mnt_flags;
    unsigned int blk_first_meta_grp;
    unsigned int t_creation;

    unsigned int j_inode_backup[17];

    unsigned int num_blks_h;
    unsigned int num_rsvd_blks_h;
    unsigned int num_free_blks_h;
}__attribute__((packed))extfs_superblock;

typedef struct extfs_bgrp_desc{

    unsigned int blk_blkbmp;
    unsigned int blk_inodebmp;

    unsigned int blk_inode_tab;

    unsigned short free_blks_in_grp;
    unsigned short free_inodes_in_grp;
    unsigned short dirs_in_grp;

	unsigned short flags;
	
	unsigned int exclude_bmp;
	unsigned short chksum_lo;
	unsigned short inode_chksum_lo;
	
	unsigned short bg_tab_lo;
	unsigned short chksum;
	
    
    unsigned int blk_blkbmp_h;
    unsigned int blk_inodebmp_h;
    unsigned int blk_inode_tab_h;
    
    unsigned short free_blks_in_grp_h;
    unsigned short free_inodes_in_grp_h;
    unsigned short dirs_in_grp_h;
    
    unsigned short flags_h;
    
    unsigned int exclude_bmp_h;
    
    unsigned short blk_blk_use_bmp_h;
    unsigned short blk_inode_use_bmp_h;
    
    unsigned int rsvd;

}__attribute__((packed))extfs_bgrp_desc;

typedef struct extfs_disk_info{

    size_t blksz_bytes;
    size_t blksz_sects;

    size_t inodes_per_grp;
    size_t blk_start;
    size_t bgdt_sz_b;

    size_t inode_struct_sz_b;

    unsigned int req_flags, opt_flags;


}extfs_disk_info;

typedef struct extfs_inode{

    unsigned short types_n_perm;

    unsigned short uid;

    unsigned int sz_in_bytes_l;
    unsigned int time_access;
    unsigned int time_create;
    unsigned int time_mod;
    unsigned int time_delete;

    unsigned short gid;

    unsigned short hard_links_count;
    unsigned int disk_sects_count;

    unsigned int flags;
    unsigned int custom;

    unsigned int blk_data_ptrs[12];
    unsigned int blk_ptrs_of_blk_data_ptrs;
    unsigned int blk_ptrs_to_blk_ptrs_of_blk_data_ptrs;
    unsigned int blk_ptrs_to_blk_ptrs_to_blk_ptrs_of_blk_data_ptrs;

    unsigned int gen_num;

    unsigned int blk_att;

    unsigned int sz_in_bytes_h;

    unsigned int blk_frag;

    char custom12[12];



}__attribute__((packed))extfs_inode;

typedef struct extfs_dirent{

    unsigned int inode;
    unsigned short ent_sz;
    unsigned char namelen_l;
    unsigned char type_or_namelen_h;
    char name[0];


}__attribute__((packed))extfs_dirent;

typedef struct extfs_hashdir_ent{
	unsigned int hash;
	unsigned int block;
}__attribute__((packed))extfs_hashdir_ent;

typedef struct extfs_hashdir_root{
	
	char onedot_and_twodot[0x1c];
	unsigned char hash_type;
	unsigned char info_len;
	unsigned char indir_levels;
	unsigned char rsvd;
	unsigned short limit_ents;
	unsigned short count_ents;
	unsigned int file_block;
	extfs_hashdir_ent ents[0];
	
}__attribute__((packed))extfs_hashdir_root;

typedef struct extfs_extent_head{

    unsigned short magic;
    unsigned short ents;

    unsigned short max_ents;

    unsigned short depth;
    unsigned int gen;

}__attribute__((packed))extfs_extent_head;

typedef struct extfs_extent_int{

    unsigned int blk_f_off;
    unsigned int blk_child_low;
    unsigned short blk_child_h;
    unsigned short rsvd;
}__attribute__((packed))extfs_extent_int;

typedef struct extfs_extent_end{

    unsigned int blk_f_off;

    unsigned short num_blks;
    unsigned short blk_dat_h;
    unsigned int blk_dat;

}__attribute__((packed))extfs_extent_end;

typedef struct extfs_blk_list{

    struct extfs_blk_list *next;
    unsigned long num_blks;
    unsigned long blks_off;
    unsigned long blks_f_off;

}extfs_blk_list;


#define EXTFS_REQF_EXTENT 0x40
#define EXTFS_REQF_64BIT 0x80
#define EXTFS_BGRP_DESC_SZ_32BITS 32
#define EXTFS_HASHED_IDX_FLAG 0x1000
void extfs_enum(diskman_ent *d);

extfs_bgrp_desc *extfs_read_blk_desc(diskman_ent *d, ino_t inode, extfs_bgrp_desc *descs_16x);
unsigned long extfs_find_finode_from_dir(diskman_ent *d, ino_t dir_inode,char *name);
unsigned long extfs_read_inode_contents(diskman_ent *d, ino_t in, void* buf, unsigned long count,unsigned long off);
#define EXTFS_ROOTDIR_INODE 2


