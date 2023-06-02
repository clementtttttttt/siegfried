#include "diskman.h"
#include "obj_heap.h"

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

    return ret;
}

void diskman_setup(){

}
