#include "runner.h"
#include "klib.h"
#include "diskman.h"
#include "draw.h"
#include "elf.h"
#include "obj_heap.h"

int runner_spawn_from_file_at_root(unsigned long disk_inode, char *name){

    diskman_ent *d;
    if(!(d = diskman_find_ent(disk_inode))){
        draw_string("invalid inode number\n");
        return 0;
    }
    unsigned long in = extfs_find_finode_from_dir(diskman_find_ent(disk_inode),EXTFS_ROOTDIR_INODE, name);

    
    if(in == 0){
			draw_string("sfinit not found\n");
			return 0;
    }

    elf_head *header = k_obj_alloc(512);
	
	draw_string("INIT INODE: ");
	draw_hex(in);
    extfs_read_inode_contents(diskman_find_ent(disk_inode), in, header, 512);
	
    if(!mem_cmp("\x7f\ELF", header->magic,4)){
		draw_string("invalid elf file");
		return 0;
	}
	
	draw_string("RUNNER: program header #0 vaddr = ");
	draw_hex(header->prog_tab[0].vaddr);
	
	
		


    return 0;
}
