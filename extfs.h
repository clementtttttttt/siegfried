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
}extfs_superblock;

typedef struct extfs_bgrp_desc{

    unsigned int blk_blkbmp;
    unsigned int blk_inodebmp;

    unsigned int blk_inode_tab;

    unsigned short free_blks_in_grp;
    unsigned short free_inodes_in_grp;
    unsigned short dirs_in_grp;

    char padding[14];

}__attribute__((packed))extfs_bgrp_desc;

typedef struct extfs_disk_info{

    unsigned int blksz_bytes;
    unsigned long inodes_per_grp;
    unsigned long blk_start;
    unsigned long bgdt_sz_b;


}extfs_disk_info;

typedef struct extfs_inode{

    unsigned short types_n_perm;

    unsigned short uid;

    unsigned int sz_in_b_l;
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
    unsigned int sz_in_b_h;

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

typedef struct extfs_extent_head{

    unsigned short magic;
    unsigned short ents;

    unsigned short max_ents;

    unsigned short depth;
    unsigned int gen;

}__attribute__((packed))extfs_extent_head;

typedef struct extfs_extent_id{

    unsigned int blk_f_off;
    unsigned int blk_child_low;
    unsigned short blk_child_h;
    unsigned short rsvd;
}__attribute__((packed))extfs_extent_id;

typedef struct extfs_extent{

    unsigned int blk_f_off;

    unsigned short num_blks;
    unsigned short blk_dat_h;
    unsigned short blk_dat;

}__attribute__((packed))extfs_extent;

void extfs_enum(diskman_ent *d);

void extfs_read_blk_desc(diskman_ent *d, unsigned long inode, extfs_bgrp_desc *descs_16x);
