#include "diskman.h"
#include "obj_heap.h"
#include "draw.h"
#include "pageobj_heap.h"
#include "klib.h"
#include "devfs.h"

//handles partition schemes and disks, makes disk partition devices as well

diskman_ent *disks = 0;

unsigned long inode = 0; //inode start at one
#include "idt.h"


diskman_ent *diskman_new_ent(){
    diskman_ent *ret;

    if(disks == 0){
        ret = disks = k_obj_alloc(sizeof(diskman_ent));
    }
    else{
        ret = disks;
        while(ret->next){
            ret = ret->next;
        }

        ret->next = k_obj_alloc(sizeof(diskman_ent));

        ret = ret->next;
    }

    ret->ispart = 0;
    ret->inode = ++inode;
    ret->fs_type = DISKMAN_FS_NULL;
    ret->next = 0;

    return ret;
}

diskman_ent *diskman_find_ent(unsigned long inode){
    diskman_ent *it = disks;
    while(it){

        if(it->inode == inode) return it;

        it = it->next;
    }


    return (diskman_ent*)0;
}

char detect_sect[1024];

void diskman_setup(){


    diskman_ent *i = disks;

    char detect_sect[1024];
   	
   

	
    while(i){
        if(i->ispart){
            i=i->next;
            continue;
        }

	
        mem_set(detect_sect, 0, 1024);

        i->read_func(i->inode, 0, 1024, detect_sect);


        //create partition drives			

        if(!mem_cmp(&detect_sect[512], "EFI PART", 8)){
            diskman_gpt_enum(i);
        }
        //draw_string_w_sz(detect_sect, 1024);

        i = i->next;
    }

    i=disks;

    //fs detection
    while(i){
	
	if(i->fs_type == DISKMAN_FS_NULL){
		extfs_enum(i);
	}

        i = i->next;
    }
    
}
