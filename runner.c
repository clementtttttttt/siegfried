#include "runner.h"
#include "klib.h"
#include "diskman.h"
#include "draw.h"


int runner_spawn_from_file_at_root(unsigned long disk_inode, char *path){

    diskman_ent *d;
    if(!(d = diskman_find_ent(disk_inode))){
        draw_string("invalid inode number");
        return 0;
    }
    unsigned long in = extfs_find_finode_from_dir(diskman_find_ent(disk_inode),EXTFS_ROOTDIR_INODE, "init.sfe");

    draw_string("INODE: ");
    draw_hex(in);


    return 0;
}
