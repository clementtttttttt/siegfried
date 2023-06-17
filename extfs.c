#include "diskman.h"
#include "draw.h"
#include "obj_heap.h"

extfs_bgrp_desc* extfs_read_inodes_blk_desc(diskman_ent *d, unsigned long inode, extfs_bgrp_desc *descs_16x){

    extfs_disk_info *inf = d->fs_disk_info;
        unsigned long sz_s = inf -> blksz_bytes / 512;

    d->read_func(d->inode,
                    (((inode-1) / (((extfs_disk_info*)d->fs_disk_info) -> inodes_per_grp)))  * inf->bgdt_sz_b / 512 /*16 blk descs in 1 sf sect */
                     + ((extfs_disk_info*)d->fs_disk_info)->blk_start * sz_s /*sb blk addr*/
                     + sz_s/*1 block offset*/
                     , 1, descs_16x);


    return (extfs_bgrp_desc*)((unsigned long)descs_16x + (((inode-1) / ((extfs_disk_info*)d->fs_disk_info) -> inodes_per_grp)) % (512 / inf->bgdt_sz_b) * inf->bgdt_sz_b);
}

extfs_inode *extfs_read_inode_struct(diskman_ent *d, unsigned long inode){

        extfs_inode *inode_tab = k_obj_alloc(512);
        extfs_bgrp_desc *bd16x = k_obj_alloc(512);

        extfs_bgrp_desc *bd = extfs_read_inodes_blk_desc(d, inode, bd16x);


        extfs_disk_info *inf = d->fs_disk_info;

        unsigned long sz_s = inf -> blksz_bytes / 512;

        draw_string("INODE TAB PTR=");

        draw_hex(  bd->blk_inode_tab*sz_s
        + ((((inode-1) % inf->inodes_per_grp) * inf->inode_struct_sz_b) / 512));

        d->read_func(d->inode,

        bd->blk_inode_tab*sz_s
        + ((((inode-1) % inf->inodes_per_grp) * inf->inode_struct_sz_b) / 512)

        ,1, inode_tab);


        return (extfs_inode *)((unsigned long)((unsigned long)inode_tab + inf->inode_struct_sz_b * ((inode - 1 ) % inf->inodes_per_grp)));

}

void extfs_read_inode_contents(diskman_ent *d, unsigned long inode, void* buf){}

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
        inf -> inode_struct_sz_b = sb->inode_struct_sz_b;


        if(sb->req_flags & 0x80){
            inf->bgdt_sz_b = sb->bgd_sz_b;
            draw_hex(sb->bgd_sz_b);
        }
        else{
            inf->bgdt_sz_b = sizeof(extfs_bgrp_desc);
        }


        extfs_bgrp_desc * bd = k_obj_alloc(512);

         extfs_read_inodes_blk_desc(d, EXTFS_ROOTDIR_INODE, bd); //root inode = 2


        extfs_inode *inode_tab = extfs_read_inode_struct(d, EXTFS_ROOTDIR_INODE);


        draw_string("INODE BLK PTR 0=");
        draw_hex(inode_tab->blk_data_ptrs[0]);

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


        draw_string("INODE SZ=");
        draw_hex(sb->inode_struct_sz_b);


        extfs_dirent * root_dirents = k_obj_alloc(1024);

        if(!(sb->req_flags & 0x40)){

            d->read_func(d->inode, ((extfs_inode*)((unsigned long)inode_tab))->blk_data_ptrs[0]*(inf->blksz_bytes/512), 2, root_dirents);

        }
        else{
            extfs_extent_head *chk2 =  (extfs_extent_head*)((extfs_inode*)((unsigned long)inode_tab))->blk_data_ptrs;

            extfs_extent_end *ex =(extfs_extent_end *)( (unsigned long)chk2 + sizeof(extfs_extent_head));

            draw_hex(ex->blk_dat);

            d->read_func(d->inode, (ex->blk_dat)*(inf->blksz_bytes/512), 2, root_dirents);

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

