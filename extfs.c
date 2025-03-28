#include "diskman.h"
#include "draw.h"
#include "obj_heap.h"
#include "klib.h"
#include "rtc.h"
#include "errno.h"
#include "tasks.h"
#include "debug.h"
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
extfs_bgrp_desc* extfs_read_inodes_blk_desc(diskman_ent *d, unsigned long inode, extfs_bgrp_desc *descs_8x){

    extfs_disk_info *inf = d->fs_disk_info;
        unsigned long sz_s = inf -> blksz_bytes ;
        

    d->read_func(d->inode,
                    (((inode-1) / (((extfs_disk_info*)d->fs_disk_info) -> inodes_per_grp)))  * inf->bgdt_sz_b /*16 blk descs in 1 sf sect */
                     + ((extfs_disk_info*)d->fs_disk_info)->blk_start * sz_s /*sb blk addr*/
                     + sz_s/*1 block offset*/
                     , 512, descs_8x);


    return (extfs_bgrp_desc*)((unsigned long)descs_8x + (((inode-1) / ((extfs_disk_info*)d->fs_disk_info) -> inodes_per_grp)) % (512 / inf->bgdt_sz_b) * inf->bgdt_sz_b);
}

extfs_bgrp_desc* extfs_read_inodes_blk_desc_new(diskman_ent *d, unsigned long inode, extfs_bgrp_desc (*descs_8x)[]){

    extfs_disk_info *inf = d->fs_disk_info;
        unsigned long sz_s = inf -> blksz_bytes ;
        

    d->read_func(d->inode,
                    (((inode-1) / (((extfs_disk_info*)d->fs_disk_info) -> inodes_per_grp)))  * inf->bgdt_sz_b /*16 blk descs in 1 sf sect */
                     + ((extfs_disk_info*)d->fs_disk_info)->blk_start * sz_s /*sb blk addr*/
                     + sz_s/*1 block offset*/
                     , 512, descs_8x);


    return (extfs_bgrp_desc*)((unsigned long)descs_8x + (((inode-1) / ((extfs_disk_info*)d->fs_disk_info) -> inodes_per_grp)) % (512 / inf->bgdt_sz_b) * inf->bgdt_sz_b);
}

void extfs_read_inode_struct(extfs_inode * inode_tab,diskman_ent *d, unsigned long inode){

       	
       	extfs_bgrp_desc bd8x[8] ;


	extfs_bgrp_desc *bd = extfs_read_inodes_blk_desc_new(d, inode, &bd8x);


        extfs_disk_info *inf = d->fs_disk_info;

        unsigned long sz_s = inf -> blksz_bytes ;

	

        //reads INODE ENTRY in INODE TABLE not BLOCK GROUP DESC

        d->read_func(d->inode,

        bd->blk_inode_tab*sz_s
        + ((((inode-1) % inf->inodes_per_grp) * inf->inode_struct_sz_b) )

        ,sizeof(extfs_inode), inode_tab);



		
	
        //return (extfs_inode *)((unsigned long)((unsigned long)inode_tab + inf->inode_struct_sz_b * ((inode - 1 ) % (512/inf->inode_struct_sz_b))));
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
    in -> next = 0;


    return in;

}

extfs_blk_list *extfs_parse_extent_tree(diskman_ent *d, extfs_extent_head *head, extfs_blk_list **ptr_root){

    extfs_blk_list *new_ptr = 0;

    extfs_blk_list **root;
    if(ptr_root){
        root = ptr_root;
    }
    else root = &new_ptr;

    if(head->depth == 0){


        extfs_extent_end *end = (extfs_extent_end*)(head + 1);

        for(unsigned long i=0; i<head->ents; ++i){

            extfs_new_pair(root, end->num_blks, (unsigned long)end->blk_dat | ((unsigned long)end->blk_dat_h << 32), end->blk_f_off);

            ++end;
        }

    }else{


        extfs_extent_int *start = (extfs_extent_int*)(head + 1);
        extfs_extent_int *end = start + head->ents;




        extfs_extent_head *subh = k_obj_alloc(512);

        extfs_disk_info *inf = d->fs_disk_info;



        while(start < end){

        draw_string("INT_NODE ADDR=");
        draw_hex((unsigned long)start->blk_child_low | ((unsigned long)start->blk_child_h << 32));

            d->read_func(d->inode, ((unsigned long)start->blk_child_low | ((unsigned long)start->blk_child_h << 32)) * inf->blksz_bytes  , 512, subh);

            unsigned long sects_sz = (sizeof(extfs_extent_int) * subh->ents + sizeof(extfs_extent_head))  + 512;

            k_obj_free(subh);
            subh=k_obj_alloc(sects_sz * 512);


            d->read_func(d->inode, ((unsigned long)start->blk_child_low | ((unsigned long)start->blk_child_h << 32)) * inf->blksz_bytes  , sects_sz, subh);


            if(subh->magic != 0xf30a){
                draw_string("HEAD INVALID MAGIC! IS ");
                draw_hex(subh->magic);
                ++start;
                continue;
            }
            extfs_parse_extent_tree(d, subh, root);

            ++start;

        }

        k_obj_free(subh);



    }

    return *root;

}



DISKMAN_FSTAT_FUNC(extfs_fstat){
		extfs_inode in; 
		extfs_read_inode_struct(&in,f->disk, f->inode);
		
		stat->perms = in.types_n_perm;
		stat->inode = f->inode;
		stat->disk_inode = f->disk->inode;
		stat->links = in.hard_links_count;
		stat->uid = in.uid;
		stat->gid = in.gid;
		stat->size = in.sz_in_bytes_l;
		stat->atime_in_ms = in.time_access;
		stat->mtime_in_ms = in.time_mod;
		stat->ctime_in_ms = in.time_create;
		
		return 0;
}

DISKMAN_FREAD_FUNC(extfs_fread){

	return extfs_read_inode_contents(f->disk, f->inode, buf, bytes, off);
	 
}

DISKMAN_FCLOSE_FUNC(extfs_fclose){
		//cleanup stuff
		if(!f){
				return -EINVAL;
		}
		k_obj_free(f);
		return 0;
}

void dump_inode(extfs_inode ino){
	draw_string("\nINODE DUMP!\n");
	
	#define D(name) {draw_string("ino->"#name" = ");draw_hex(ino.name);}

	D(time_access)
	D(disk_sects_count)
	D(types_n_perm)
	D(sz_in_bytes_l)
	D(sz_in_bytes_h)
}



long extfs_find_inode_from_name_and_set_name(char *path, unsigned long disk_id,char* new_name){
			str_tok_result res = {0,0};

		ino_t curr_inode;
		switch(path[0]){
			case '/':
				++path;
				curr_inode = EXTFS_ROOTDIR_INODE;
				break;
			default:
				char buf[PATH_MAX] = {0};
				extern char *syscall_getcwd(char *buf, size_t size);
				syscall_getcwd(buf, PATH_MAX);
				curr_inode = extfs_find_inode_from_name_and_set_name(buf,disk_id, new_name);
				//TODO: open app dir
				if(curr_inode< 0){
				//	draw_hex(curr_inode);
						draw_string("ERR! curr_inode=");
		draw_hex(curr_inode);
		
					return curr_inode;
				}
				break;
				
			
		}
				
		str_tok(path, '/', &res);
		
		
		
		str_tok_result name_res = {0,0};
		char name[256];
		

		while(res.sz != 0){

						draw_string("curr_inode=");
		draw_hex(curr_inode);
		
				mem_set(name, 0, 256);
				mem_cpy(name , path+res.off, res.sz);
			
				extfs_inode f_info = {0};
				extfs_read_inode_struct(&f_info, diskman_find_ent(disk_id), curr_inode);
				
				if(!(f_info.types_n_perm & 0x4000)){ //not dir
					draw_string("f_info.types_n_perm = ");
					draw_hex(f_info.types_n_perm);
					dump_inode(f_info);
					return -ENOTDIR;
				}
				
				
				if((curr_inode = extfs_find_finode_from_dir(diskman_find_ent(disk_id),curr_inode, name)) == 0){
					
					return -ENOENT;//not dir
				}
			
			name_res = res;
			str_tok(path, '/', &res);
		
		}
				mem_cpy(new_name, path+name_res.off, str_len(path+name_res.off));

		return curr_inode;
}
long
 extfs_read_dir_dirents(diskman_ent *d, unsigned long dir_ino, extfs_dirent *parent){

		long ret;
		extfs_disk_info *inf = d->fs_disk_info;
		
		
		extfs_inode inode_tab;
		extfs_read_inode_struct(&inode_tab,d, dir_ino);
	
	if(!(inode_tab.types_n_perm & 0x4000)){ //not a dir        
			
			return 0;
		}
		

		
        if(!(inf->req_flags & EXTFS_REQF_EXTENT)){

            ret= d->read_func(d->inode, inode_tab.blk_data_ptrs[0]*(inf->blksz_bytes), 512*2, parent);
        }
        else{
            extfs_extent_head *chk2 =  (extfs_extent_head*)inode_tab.blk_data_ptrs;

            extfs_extent_end *ex =(extfs_extent_end *)( (unsigned long)chk2 + sizeof(extfs_extent_head));

            ret=d->read_func(d->inode, (ex->blk_dat)*(inf->blksz_bytes), 512*2, parent);

        }
 

	return ret;
	
}

DISKMAN_READ_DIR_FUNC(extfs_freaddir){
	
			extfs_inode dir_ino;
		extfs_read_inode_struct(&dir_ino, diskman_find_ent(in->di), in->inode);	
			char dir_dirents_mem[dir_ino.sz_in_bytes_l];
        extfs_dirent * dir_dirents = (extfs_dirent*)dir_dirents_mem;


        extfs_read_dir_dirents(diskman_find_ent(in->di), in->inode,  dir_dirents);
        
        unsigned long idx =0 ;
        while(dir_dirents->inode){

            mem_cpy(names[idx++], dir_dirents->name, dir_dirents->namelen_l);
             
            dir_dirents = (void*)(((unsigned long)dir_dirents) + dir_dirents->ent_sz);


        }
		return names;
} 

DISKMAN_OPEN_DIR_FUNC(extfs_fopendir){
		char name[NAME_MAX];
		ino_t dir_inode = extfs_find_inode_from_name_and_set_name(path, dm_inode, name);
		if(dir_inode <= 0){
				return (int) dir_inode;
		}
		
		diskman_ent *d = diskman_find_ent(dm_inode);
		
		
		        extfs_bgrp_desc bd[8];

        extfs_read_inodes_blk_desc_new(d, dir_inode, &bd); //root inode = 2
		
		extfs_inode dir_ino;
		extfs_read_inode_struct(&dir_ino, d, dir_inode);	
		
				if(!(dir_ino.types_n_perm & 0x4000)){ //not dir
										draw_string("dir_ino.types_n_perm = ");
															dump_inode(dir_ino);

					draw_hex(dir_ino.types_n_perm);
					return -ENOTDIR;
				}
						

		char dir_dirents_mem[dir_ino.sz_in_bytes_l];
        extfs_dirent * dir_dirents = (extfs_dirent*)dir_dirents_mem;

		/*

        if(!(sb->req_flags & EXTFS_REQF_EXTENT)){

            d->read_func(d->inode, ((extfs_inode*)((unsigned long)inode_tab))->blk_data_ptrs[0]*(inf->blksz_bytes/512), 2, root_dirents);

        }
        else{
            extfs_extent_head *chk2 =  (extfs_extent_head*)((extfs_inode*)((unsigned long)inode_tab))->blk_data_ptrs;

            extfs_extent_end *ex =(extfs_extent_end *)( (unsigned long)chk2 + sizeof(extfs_extent_head));

            d->read_func(d->inode, (ex->blk_dat)*(inf->blksz_bytes/512), 2, root_dirents);


        }*/
        
        extfs_read_dir_dirents(d, dir_inode,  dir_dirents);
        
        int count = 0;
        
        void *dir_dirents_2 = dir_dirents;

        while(dir_dirents->inode){
/*
            draw_string("FNAME=");

            draw_string_w_sz(root_dirents->name, root_dirents->namelen_l);

            draw_string(" INODE=");

            draw_hex(root_dirents->inode);

            draw_string(" SZ=");

            draw_hex(root_dirents->ent_sz);*/


    
/*            if(mem_cmp("words.txt", root_dirents->name, str_len("words.txt"))){
                    extfs_inode *inode_tab2 = extfs_read_inode_struct(d, root_dirents->inode, &f_ptr);

                    if(sb->req_flags & 0x40){
                        extfs_blk_list *blks = extfs_parse_extent_tree(d, (extfs_extent_head*)inode_tab2->blk_data_ptrs,0);



                            char *buf5 = k_obj_alloc(0x3000);


                            d->read_func(d->inode, blks->blks_off * inf->blksz_bytes,0x10*512,buf5);
							draw_string_w_sz(buf5, 100);
							
							k_obj_free(buf5);

                    }
                    
			}
            */
			++count;
             
            dir_dirents = (void*)(((unsigned long)dir_dirents) + dir_dirents->ent_sz);


        }
        
        dir_dirents = dir_dirents_2;
        
        siegfried_dir *dir = in;

        
        
        dir->num_files = count;
        
        /*
        int idx = 0;
        while(dir_dirents->inode){

             mem_cpy(dir->filenames[idx], dir_dirents->name, dir_dirents->namelen_l);
             dir->filenames[idx][dir_dirents->namelen_l] = 0;
             
             ++idx;
             
            dir_dirents = (void*)(((unsigned long)dir_dirents) + dir_dirents->ent_sz);


        }
        */
		
		dir->inode = dir_inode;
		mem_cpy(dir->name, name, str_len(name)); 
		dir->di = dm_inode;
		
		return 0;
}

DISKMAN_FOPEN_FUNC(extfs_fopen){
		siegfried_file *f = k_obj_alloc(sizeof(siegfried_file));
		
		ino_t curr_inode = extfs_find_inode_from_name_and_set_name(path, disk_id,f->name);
		
		if(curr_inode <= 0){
								k_obj_free(f);
								return (siegfried_file*)curr_inode;

		}
		
		extfs_inode ins; 
		extfs_read_inode_struct(&ins,diskman_find_ent(disk_id), curr_inode);
		
		//f->inode = curr_task->dir.inode;
		f->inode = curr_inode;
		f->disk = diskman_find_ent(disk_id);
		
		return f;
}

void extfs_free_blk_list(extfs_blk_list *l){

    while(l){
        extfs_blk_list *next = l->next;

        k_obj_free(l);

        l = next;
    }

}

unsigned long extfs_read_inode_contents(diskman_ent *d, ino_t in, void* buf, unsigned long count, unsigned long off){

    extfs_inode inode_tab; 
    extfs_read_inode_struct(&inode_tab,d, in);
   
    extfs_disk_info *inf = d->fs_disk_info;
		unsigned long read;
    
    	if(!(inf->req_flags & EXTFS_REQF_EXTENT)){

            read =d->read_func(d->inode, ((extfs_inode*)((unsigned long)&inode_tab))->blk_data_ptrs[0]*(inf->blksz_bytes) + off, count, buf);
        }
        else{

	
		extfs_inode inode_tab_2;
		extfs_read_inode_struct(&inode_tab_2,d, in );

                        extfs_blk_list *blks = extfs_parse_extent_tree(d, (extfs_extent_head*)inode_tab_2.blk_data_ptrs,0);


			//TODO: not reading entire extent properly?

          read= d->read_func(d->inode, blks->blks_off * inf->blksz_bytes + off,count,buf);
                            
							

     }
        
       
        return read;
}


void extfs_fclosedir(ino_t dm_inode, siegfried_dir *in){
		
}


unsigned long extfs_find_finode_from_dir(diskman_ent *d, ino_t dir_inode,char *name){


    if(d->fs_type != DISKMAN_FS_EXTFS){
        return 0;
    }     
    char root_dirents_mem[1024];
    extfs_dirent * root_dirents = (extfs_dirent*)root_dirents_mem
    ;

		/*

        if(!(sb->req_flags & EXTFS_REQF_EXTENT)){

            d->read_func(d->inode, ((extfs_inode*)((unsigned long)inode_tab))->blk_data_ptrs[0]*(inf->blksz_bytes/512), 2, root_dirents);

        }
        else{
            extfs_extent_head *chk2 =  (extfs_extent_head*)((extfs_inode*)((unsigned long)inode_tab))->blk_data_ptrs;

            extfs_extent_end *ex =(extfs_extent_end *)( (unsigned long)chk2 + sizeof(extfs_extent_head));

            d->read_func(d->inode, (ex->blk_dat)*(inf->blksz_bytes/512), 2, root_dirents);


        }*/
        if(extfs_read_dir_dirents(d, dir_inode,  root_dirents) == 0){
			return 0;
		}
        
       

        while(root_dirents->inode){

            if(str_len(name) == root_dirents->namelen_l && !mem_cmp(name, root_dirents->name, MIN(str_len(name),root_dirents->namelen_l))){
					
			return root_dirents->inode;

            }

            //  draw_string_w_sz(root_dirents->name,root_dirents->ent_sz);
            root_dirents = (void*)(((unsigned long)root_dirents) + root_dirents->ent_sz);


        }

 
        return 0;

}
   

void extfs_enum(diskman_ent *d){
       

	char chkbuf[1024];

    d->read_func(d->inode, 1024, 1024, chkbuf); //superblock at 1024 bytes offset, which is 2 512sz sectors
    

    extfs_superblock *sb = (extfs_superblock*)chkbuf;

    //draw_hex(sb->gid_of_rsvd_blk );

    if(sb->magic == 0xef53){

        draw_string("FOUND EXTFS\n");

        draw_string("DM_INODE=");
        draw_hex(d->inode);

        draw_string("MAJ_VER=");
        draw_hex(sb->ver_major);

        draw_string("BLKSZ=");
        draw_hex(1024 << sb->blksz);

        draw_string("START BLK=");
        draw_hex(sb->sb_blknum);

        d->fs_type = DISKMAN_FS_EXTFS;

	


        extfs_disk_info *inf = d->fs_disk_info = k_obj_alloc(sizeof(extfs_disk_info));
        inf -> inodes_per_grp = sb->num_inodes_grp;
        inf -> blk_start = sb -> sb_blknum;

        inf -> blksz_bytes = 1024 << sb->blksz;
        inf -> blksz_sects = inf->blksz_bytes / 512;
        inf -> inode_struct_sz_b = sb->inode_struct_sz_b;

        inf->req_flags = sb->req_flags;

        if(sb->req_flags & 0x80){
            inf->bgdt_sz_b = sb->bgd_sz_b;
            draw_hex(sb->bgd_sz_b);
        }
        else{
            inf->bgdt_sz_b = sizeof(extfs_bgrp_desc);
        }

			//set rw functions in struct
        d->fopen = extfs_fopen;
		d->fread = extfs_fread;
		d->fstat = extfs_fstat;
		d->fclose = extfs_fclose;
		d->fopendir = extfs_fopendir;
		d->freaddir = extfs_freaddir;
		d->fclosedir = extfs_fclosedir;



    }

	


}

