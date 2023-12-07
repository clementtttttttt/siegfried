#include "runner.h"
#include "klib.h"
#include "diskman.h"
#include "draw.h"
#include "elf.h"
#include "obj_heap.h"

int runner_spawn_from_file_at_root(unsigned long disk_inode, char *path){

    diskman_ent *d;
    if(!(d = diskman_find_ent(disk_inode))){
        draw_string("invalid inode number\n");
        return 0;
    }
    unsigned long in = extfs_find_finode_from_dir(diskman_find_ent(disk_inode),EXTFS_ROOTDIR_INODE, "sfinit");
    
    
    if(in == 0){
			draw_string("sfinit not found\n");
			return 0;
    }
    elf_head *header = k_obj_alloc(sizeof(elf_head));

    extfs_read_inode_contents(diskman_find_ent(disk_inode), in, header, sizeof(elf_head));

    if(mem_cmp("\7fELF", header->magic,4)){
		draw_string("invalid elf file\n");
		return 0;
	}

    draw_string("INIT INODE: ");
    draw_hex(in);


    return 0;
}
