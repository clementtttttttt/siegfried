#include "diskman.h"
#include "obj_heap.h"
#include "draw.h"
#include "pageobj_heap.h"
#include "klib.h"

//handles partition schemes and disks, makes disk partition devices as well

diskman_ent *disks = 0;

unsigned long inode = 0;



diskman_ent *diskman_new_ent(){

    diskman_ent *ret;

    if(disks == 0){
        ret = disks = k_obj_calloc(sizeof(diskman_ent), 1);
    }
    else{
        ret = disks;
        while(ret->next){
            ret = ret->next;
        }

        ret->next = k_obj_calloc(sizeof(diskman_ent),1);

        ret = ret->next;
    }

    ret->inode = ++inode;
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

    char *detect_sect=k_obj_alloc( 1024);

    while(i){
        if(i->ispart){
            i=i->next;
            continue;
        }

        mem_set(detect_sect, 0, 1024);

        i->read_func(i->inode, 0, 2, detect_sect);


        //create partition drives

        if(mem_cmp(&detect_sect[512], "EFI PART", 8)){
            diskman_gpt_enum(i);
        }
        //draw_string_w_sz(detect_sect, 1024);

        i = i->next;
    }

    i=disks;

    //fs detection
    while(i){

        extfs_enum(i);

        i = i->next;
    }
}
