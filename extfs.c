#include "diskman.h"
#include "draw.h"
#include "obj_heap.h"

void extfs_enum(diskman_ent *d){

    void *chkbuf = k_obj_alloc(1024);

    d->read_func(d->inode, 2, 2, chkbuf); //superblock at 1024 bytes offset, which is 2 512sz sectors

    extfs_superblock *sb = chkbuf;

    //draw_hex(sb->gid_of_rsvd_blk );

    if(sb->magic == 0xef53){
        draw_string("FOUND EXTFS\n");

        draw_string("MAJ_VER=");
        draw_hex(sb->ver_major);

        draw_string("BLKSZ=");
        draw_hex(1024 << sb->blksz);

        draw_string("START BLK=");
        draw_hex(sb->sb_blknum);

        extfs_disk_info *inf = d->fs_disk_info = k_obj_alloc(sizeof(extfs_disk_info));
        inf -> inodes_per_grp = sb->num_inodes_grp;
        inf -> blk_start = sb -> sb_blknum;
        inf -> blksz_bytes = 1024 << sb->blksz;


        extfs_bgrp_desc * bd = k_obj_alloc(1024);

        d->read_func(d->inode,(inf->blksz_bytes/512)*(inf->blk_start+1)  , 2, bd);

            draw_string("INODE TAB PTR=");
        draw_hex( bd[0].blk_inode_tab*inf->blksz_bytes/512);

        extfs_inode *inode_tab = k_obj_alloc(1024);

        d->read_func(d->inode,  bd[0].blk_inode_tab*inf->blksz_bytes/512, 2, inode_tab);

        draw_string("INODE BLK PTR 0=");
        draw_hex(((extfs_inode*)((unsigned long)inode_tab + sb->inode_struct_sz_b))->blk_data_ptrs[0]);

        draw_string("NUM_BLKGRPS_INODES=");
        draw_hex(sb->num_inodes_grp);

        draw_string("FEATURES= ");

        char *extfs_featstr_tab[] = {"CMPR", "DIRENT_HA_TYPE", "NEED_J", "J_DEV", 0, 0, "EXTENT", "64BIT", "MMP", "FLEX", "EXTATT", 0, "DATA_DIRENT", "CHKSUM_IN_SB", "DIRSZ>4G", "INODE_DATA", "ENCRYPTION", "CASEFOLD"};

        for(int i=0;i < 32;++i){
            if(sb->req_flags & (1<<i)){
                draw_string(extfs_featstr_tab[i]);
                draw_string(" ");
            }
        }

        draw_string("\n");

        if(sb->req_flags & 0x80){
            inf->bgdt_sz_b = sb->bgd_sz_b;
        }
        else{
            inf->bgdt_sz_b = sizeof(extfs_bgrp_desc);
        }


        draw_string("INODE SZ=");
        draw_hex(sb->inode_struct_sz_b);

        extfs_dirent * root_dirents = k_obj_alloc(1024);

        if(!(sb->req_flags & 0x40)){
            d->read_func(d->inode, ((extfs_inode*)((unsigned long)inode_tab + sb->inode_struct_sz_b))->blk_data_ptrs[0]*(inf->blksz_bytes/512), 2, root_dirents);
        }
        else{
            extfs_extent_head *chk2 = k_obj_alloc(1024);
            d->read_func(d->inode, ((extfs_inode*)((unsigned long)inode_tab + sb->inode_struct_sz_b))->blk_data_ptrs[0]*(inf->blksz_bytes/512), 2, chk2);

            draw_hex(chk2->magic);

        }

        draw_string("====EXTFS DIRLIST TEST====\n");

        while(root_dirents->ent_sz){

            draw_string("FNAME=");

            draw_string_w_sz(root_dirents->name, root_dirents->namelen_l);
            //  draw_string_w_sz(root_dirents->name,root_dirents->ent_sz);
                        root_dirents = (void*)(((unsigned long)root_dirents) + root_dirents->ent_sz);

            draw_string("\n");
        }

    }

    k_obj_free(chkbuf);


}

