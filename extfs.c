#include "diskman.h"
#include "draw.h"
#include "obj_heap.h"
#include "klib.h"
#include "rtc.h"
#include "errno.h"
#include "tasks.h"
#include "debug.h"
#define MIN(x, y) (((x) < (y)) ? (x) : (y))


#define ROUND(f, a, b, c, d, x, s)	\
	(a += f(b, c, d) + x, a = rol32(a, s))
#define K1 0
#define K2 013240474631UL
#define K3 015666365641UL


/* F, G and H are basic MD4 functions: selection, majority, parity */
#define F(x, y, z) ((z) ^ ((x) & ((y) ^ (z))))
#define G(x, y, z) (((x) & (y)) + (((x) ^ (y)) & (z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))

/**
 * rol32 - rotate a 32-bit value left
 * @word: value to rotate
 * @shift: bits to roll
 */
static inline uint32_t rol32(uint32_t word, unsigned int shift)
{
	return (word << (shift & 31)) | (word >> ((-shift) & 31));
}


/*
 * Basic cut-down MD4 transform.  Returns only 32 bits of result.
 */
static uint32_t half_md4_transform(uint32_t buf[4], uint32_t const in[8])
{
	uint32_t a = buf[0], b = buf[1], c = buf[2], d = buf[3];

	/* Round 1 */
	ROUND(F, a, b, c, d, in[0] + K1,  3);
	ROUND(F, d, a, b, c, in[1] + K1,  7);
	ROUND(F, c, d, a, b, in[2] + K1, 11);
	ROUND(F, b, c, d, a, in[3] + K1, 19);
	ROUND(F, a, b, c, d, in[4] + K1,  3);
	ROUND(F, d, a, b, c, in[5] + K1,  7);
	ROUND(F, c, d, a, b, in[6] + K1, 11);
	ROUND(F, b, c, d, a, in[7] + K1, 19);

	/* Round 2 */
	ROUND(G, a, b, c, d, in[1] + K2,  3);
	ROUND(G, d, a, b, c, in[3] + K2,  5);
	ROUND(G, c, d, a, b, in[5] + K2,  9);
	ROUND(G, b, c, d, a, in[7] + K2, 13);
	ROUND(G, a, b, c, d, in[0] + K2,  3);
	ROUND(G, d, a, b, c, in[2] + K2,  5);
	ROUND(G, c, d, a, b, in[4] + K2,  9);
	ROUND(G, b, c, d, a, in[6] + K2, 13);

	/* Round 3 */
	ROUND(H, a, b, c, d, in[3] + K3,  3);
	ROUND(H, d, a, b, c, in[7] + K3,  9);
	ROUND(H, c, d, a, b, in[2] + K3, 11);
	ROUND(H, b, c, d, a, in[6] + K3, 15);
	ROUND(H, a, b, c, d, in[1] + K3,  3);
	ROUND(H, d, a, b, c, in[5] + K3,  9);
	ROUND(H, c, d, a, b, in[0] + K3, 11);
	ROUND(H, b, c, d, a, in[4] + K3, 15);

	buf[0] += a;
	buf[1] += b;
	buf[2] += c;
	buf[3] += d;

	return buf[1]; /* "most hashed" word */
}
#undef ROUND
#undef K1
#undef K2
#undef K3
#undef F
#undef G
#undef H

static void str2hashbuf_signed(const char *msg, int len, uint32_t *buf, int num)
{       
	draw_hex(num);
        uint32_t   pad, val;
        int     i;
        const signed char *scp = (const signed char *) msg;
                                
        pad = (uint32_t)len | ((uint32_t)len << 8);
        pad |= pad << 16;
                
        val = pad;      
        if (len > num*4)
                len = num * 4;
        for (i = 0; i < len; i++) {
                val = ((int) scp[i]) + (val << 8); 
                if ((i % 4) == 3) {
                        *buf++ = val;
                        val = pad;
                        num--;
                }
        }       
        if (--num >= 0)
                *buf++ = val;
        while (--num >= 0)
                *buf++ = pad;
}            

 uint32_t extfs_dir_hash(const extfs_inode *d, const char *name, long len){
	uint32_t seed[4];
	     /* Initialize the default seed for the hash checksum functions */
        seed[0] = 0x67452301;
        seed[1] = 0xefcdab89;
        seed[2] = 0x98badcfe;
        seed[3] = 0x10325476;
        
        uint32_t hash_in[8];
        const char *backup = name;
        
        while(len>0){
			str2hashbuf_signed(backup, len, hash_in, 8);
			half_md4_transform(seed, hash_in);
			len -= 32;
			backup += 32;
		}
	return 0;
} 


void dump_inode(extfs_inode ino){
/*	draw_string("\nINODE DUMP!\n");
	
	#define D(name) {draw_string("ino->"#name" = ");draw_hex(ino.name);}
	

	D(time_access)
	D(disk_sects_count)
	D(types_n_perm)
	D(sz_in_bytes_l)
	D(sz_in_bytes_h)*/
}

void dump_hashroot(extfs_hashdir_root *in){
		draw_string("\nHASH DUMP!\n");
	
	#define DE(name) {draw_string("in->"#name" = ");draw_hex(in->name);}
	DE(hash_type);
	DE(info_len);
	DE(indir_levels);
	DE(limit_ents);
	DE(count_ents);
	DE(file_block);
	DE(ents[0].hash);
	DE(ents[0].block);
	DE(ents[1].hash);
	DE(ents[1].block);
}

void extfs_read_inodes_blk_desc(diskman_ent *d, ino_t inode, extfs_bgrp_desc *in){

    extfs_disk_info *inf = d->fs_disk_info;
        unsigned long sz_s = inf -> blksz_bytes ;
        
       draw_string("block group=");
       draw_hex((((inode-1) / (((extfs_disk_info*)d->fs_disk_info) -> inodes_per_grp))) );

	draw_string("bgd blocks=");
	
	unsigned long idx =                           (((inode-1) / (((extfs_disk_info*)d->fs_disk_info) -> inodes_per_grp)))  * inf->bgdt_sz_b  
                     + ((extfs_disk_info*)d->fs_disk_info)->blk_start * sz_s /*sb blk addr*/
                     + sz_s;


    d->read_func(d->inode,
                    idx
                     , sizeof(extfs_bgrp_desc), in);


}



void extfs_read_inode_struct(extfs_inode * inode_tab,diskman_ent *d, ino_t inode){

       	


	extfs_bgrp_desc bd; 
	extfs_read_inodes_blk_desc(d, inode, &bd);
	


	//FIXME: 64bit extfs address
        extfs_disk_info *inf = d->fs_disk_info;

        size_t sz_s = inf -> blksz_bytes ;

	draw_string("inode=");
	draw_hex(inode);
	draw_string("off=");
		draw_hex(  bd.blk_inode_tab*sz_s
        + ((((inode-1) % inf->inodes_per_grp) * inf->inode_struct_sz_b) )
       );
        	draw_string("off2=");
		draw_hex(bd.blk_inode_tab);
        //reads INODE ENTRY in INODE TABLE not BLOCK GROUP DESC

        d->read_func(d->inode,

        bd.blk_inode_tab*sz_s
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
	extfs_inode check;
	extfs_read_inode_struct(&check, f->disk, f->inode);
	if(check.types_n_perm & EXTFS_DIR_TYPE){
		return -EISDIR;
	}

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




ino_t extfs_find_inode_from_name_and_set_name(char *path, unsigned long disk_id,char* new_name){
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
		char name[PATH_MAX];
		

		while(res.sz != 0){

		
				mem_set(name, 0, 256);
				mem_cpy(name , path+res.off, res.sz);
			
				extfs_inode f_info = {0};
				extfs_read_inode_struct(&f_info, diskman_find_ent(disk_id), curr_inode);
				
				draw_string("\n\n");
				draw_string(name);				draw_hex(f_info.types_n_perm);

				draw_string("\n\n");
				
				if(f_info.types_n_perm & EXTFS_SYMLINK_TYPE){
					
										char symlink_str[f_info.sz_in_bytes_l + 1];

					if(f_info.sz_in_bytes_l < 60){ //stored in extent stuff
						draw_string_w_sz((const char*)f_info.blk_data_ptrs, f_info.sz_in_bytes_l);
											mem_cpy(symlink_str, f_info.blk_data_ptrs, f_info.sz_in_bytes_l);

					}
			draw_string("\n\nA");
				draw_string(symlink_str);
				draw_string("A\n\n");
					
					curr_inode  = extfs_find_finode_from_dir(diskman_find_ent(disk_id), curr_inode, symlink_str);
				}	
				else if(!(f_info.types_n_perm & EXTFS_DIR_TYPE)){ //not dir
					draw_string("f_info.types_n_perm = ");
					draw_hex(f_info.types_n_perm);
					dump_inode(f_info);
					
					return -ENOTDIR;
				}
				
				
				if((curr_inode = extfs_find_finode_from_dir(diskman_find_ent(disk_id),curr_inode, name)) == 0){
					
					return -ENOENT;//not dir
				}

			str_tok(path, '/', &res);
		
		}


				mem_cpy(new_name, path+name_res.off, str_len(path+name_res.off));
			
		return curr_inode;
}
long
 extfs_read_dir_dirents(diskman_ent *d, unsigned long dir_ino, extfs_dirent *parent){

		long ret =0 ;
		extfs_disk_info *inf = d->fs_disk_info;
		
		
		extfs_inode inode_tab;
		extfs_read_inode_struct(&inode_tab,d, dir_ino);
	
		if(!(inode_tab.types_n_perm & 0x4000) ){ //not a dir        
			return -ENOTDIR;
		}
		

		
        if(!(inf->req_flags & EXTFS_REQF_EXTENT)){

            ret= d->read_func(d->inode, inode_tab.blk_data_ptrs[0]*(inf->blksz_bytes), 512*2, parent);
        }
        else{

            



             if(inode_tab.flags & EXTFS_HASHED_IDX_FLAG){
				//read in twodots and bullcrap
				//TODO: dirty hack that changes doubledot dirent
				((extfs_dirent*)((void*)parent+0xc))->ent_sz = 12;
             
				char hash_root_mem[sizeof(extfs_hashdir_root) + 160] = {0};
				extfs_hashdir_root *hash_root = (extfs_hashdir_root*)hash_root_mem;
				
				ret = extfs_read_inode_contents(d, dir_ino, hash_root,  sizeof(extfs_hashdir_root) + 160, 0);
				
				ret = extfs_read_inode_contents(d, dir_ino, ((char*)parent), 512,hash_root->file_block*inf->blksz_bytes);

				//FIXME: proper reading dirents 
				//size_t off2 = 0;
				for(int i=1;i<hash_root->count_ents-1;++i){
					
					// off2 += extfs_read_inode_contents(d, dir_ino, ((char*)parent)+0x18, 512,hash_root->ents[i].block*inf->blksz_bytes+off2);
				}
				
			
			 }
			 else{
				 ret = extfs_read_inode_contents(d, dir_ino, parent, 512*2,0);
		
			}

        }
 
	return ret;
	
}

DISKMAN_READ_DIR_FUNC(extfs_freaddir){
	//FIXME: proper dirent sizing
			char dir_dirents_mem[4096];
			mem_set(dir_dirents_mem, 0, 4096);
        extfs_dirent * dir_dirents = (extfs_dirent*)dir_dirents_mem;


        extfs_read_dir_dirents(diskman_find_ent(in->di), in->inode,  dir_dirents);
        
        unsigned long idx =0 ;
        while(dir_dirents->ent_sz && dir_dirents->inode){

            mem_cpy(names[idx++], dir_dirents->name, dir_dirents->namelen_l);
             
            dir_dirents = (void*)(((unsigned long)dir_dirents) + dir_dirents->ent_sz);


        }
		return names;
} 

DISKMAN_OPEN_DIR_FUNC(extfs_fopendir){
		char name[NAME_MAX];
		ino_t dir_inode = extfs_find_inode_from_name_and_set_name(path, dm_inode, name);
		if(dir_inode <= 0){
				return  dir_inode;
		}
		
		
		diskman_ent *d = diskman_find_ent(dm_inode);
		

						

		char dir_dirents_mem[4096];//FIXME: proper dirent size
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
        
        mem_set(dir_dirents_mem, 0, 4096);
        
        long ret = extfs_read_dir_dirents(d, dir_inode,  dir_dirents);
        
   
        if(ret <= 0){

			return ret;
		}
        
        int count = 0;
        
        void *dir_dirents_2 = dir_dirents;

        while(dir_dirents->ent_sz&& dir_dirents->inode){
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

size_t extfs_read_inode_contents(diskman_ent *d, ino_t in, void* buf, long count, unsigned long off){

    extfs_inode inode_tab; 
    extfs_read_inode_struct(&inode_tab,d, in);
   
    extfs_disk_info *inf = d->fs_disk_info;
		unsigned long read=0;
    
    	if(!(inf->req_flags & EXTFS_REQF_EXTENT)){

            read =d->read_func(d->inode, ((extfs_inode*)((unsigned long)&inode_tab))->blk_data_ptrs[0]*(inf->blksz_bytes) + off, count, buf);
        }
        else{

	
	
			extfs_blk_list *blks = extfs_parse_extent_tree(d, (extfs_extent_head*)inode_tab.blk_data_ptrs,0);
			dump_inode(inode_tab);
			//FIXME: not reading entire extent properly?
			
		  do{
			  long count2 = MIN((long)(blks->num_blks * inf->blksz_bytes), count);
				draw_string("OFF");
				draw_hex( blks->blks_off );
				read += d->read_func(d->inode, blks->blks_off * inf->blksz_bytes + off,count2,buf);
				off += count2;
				buf += count2;
				count -= blks->num_blks * inf->blksz_bytes;
				blks = blks->next;
	      }
          while(count>0 && blks);                
          
          extfs_free_blk_list(blks);
		  

     }
        
       
        return read;
}


void extfs_fclosedir(ino_t dm_inode, siegfried_dir *in){
		
}


ino_t extfs_find_finode_from_dir(diskman_ent *d, ino_t dir_inode,char *name){


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
        long ret;
        if((ret=extfs_read_dir_dirents(d, dir_inode,  root_dirents) )<=0){
						draw_string("ERR!");
			draw_hex(-ret);
			return ret;
		}
        
       

        while(root_dirents->inode && root_dirents->ent_sz){

            if(str_len(name) == root_dirents->namelen_l && !mem_cmp(name, root_dirents->name, MIN(str_len(name),root_dirents->namelen_l))){
					
			return root_dirents->inode;

            }

            //  draw_string_w_sz(root_dirents->name,root_dirents->ent_sz);
            root_dirents = (void*)(((unsigned long)root_dirents) + root_dirents->ent_sz);


        }

 
        return -ENOENT;

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

        if(sb->req_flags & EXTFS_REQF_64BIT){
            inf->bgdt_sz_b = sb->bgd_sz_b;
        }
        else{
            inf->bgdt_sz_b = EXTFS_BGRP_DESC_SZ_32BITS;
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

