#include "diskman.h"
#include "draw.h"
#include "obj_heap.h"
#include "klib.h"

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

        //reads INODE ENTRY in INODE TABLE not BLOCK GROUP DESC

        d->read_func(d->inode,

        bd->blk_inode_tab*sz_s
        + ((((inode-1) % inf->inodes_per_grp) * inf->inode_struct_sz_b) / 512)

        ,1, inode_tab);

        k_obj_free(bd16x);

        return (extfs_inode *)((unsigned long)((unsigned long)inode_tab + inf->inode_struct_sz_b * ((inode - 1 ) % (512/inf->inode_struct_sz_b))));

}

extfs_blk_list *extfs_new_pair(extfs_blk_list **root, unsigned long num, unsigned long off, unsigned long blks_f_off){

    extfs_blk_list *in = *root;

    if(in == 0){

        in = *root = k_obj_alloc(sizeof(extfs_blk_list));

    }
    else{
        while(in->next){
            in = in->next;
        }
        in->next = k_obj_alloc(sizeof(extfs_blk_list));
        in=in->next;
    }

    in -> blks_off = off;
    in -> blks_f_off = blks_f_off;
    in -> num_blks = num;


    return in;

}

extfs_blk_list *extfs_parse_extent_tree(diskman_ent *d, extfs_extent_head *head){


    extfs_blk_list *root = 0;

    draw_string("DEPTH=");
    draw_hex(head->depth);

    if(head->depth == 0){


        extfs_extent_end *end = (extfs_extent_end*)((unsigned long)head + sizeof(extfs_extent_head));

        for(unsigned long i=0; i<head->ents; ++i){

            extfs_new_pair(&root, end->num_blks, (unsigned long)end->blk_dat | ((unsigned long)end->blk_dat_h << 32), end->blk_f_off);

            end = (extfs_extent_end*)((unsigned long)end + sizeof(extfs_extent_end));
        }





    }else{


    }

    return root;

}

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

        while(root_dirents->inode){

            draw_string("FNAME=");

            draw_string_w_sz(root_dirents->name, root_dirents->namelen_l);

            draw_string(" INODE=");

            draw_hex(root_dirents->inode);

            draw_string(" SZ=");

            draw_hex(root_dirents->ent_sz);

            if(mem_cmp("words.txt", root_dirents->name, str_len("words.txt"))){
                    extfs_inode *inode_tab2 = extfs_read_inode_struct(d, 0xd);

                    if(sb->req_flags & 0x40){
                        extfs_blk_list *blks = extfs_parse_extent_tree(d, (extfs_extent_head*)inode_tab2->blk_data_ptrs);



                            char *buf = k_obj_alloc(0x2000);


                            d->read_func(d->inode, blks->blks_off * inf->blksz_bytes / 512,0x10,buf);

                           draw_string_w_sz(buf,0x2000);

                    }

            }

            //  draw_string_w_sz(root_dirents->name,root_dirents->ent_sz);
            root_dirents = (void*)(((unsigned long)root_dirents) + root_dirents->ent_sz);


        }

    }

    k_obj_free(chkbuf);


}

