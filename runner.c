#include "runner.h"
#include "klib.h"
#include "diskman.h"
#include "draw.h"
#include "elf.h"
#include "obj_heap.h"
#include "page.h"
#include "tasks.h"

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
    extfs_read_inode_contents(diskman_find_ent(disk_inode), in, header, 512, 0);
	
    if(!mem_cmp("\x7f\ELF", header->magic,4)){
		draw_string("invalid elf file");
		return 0;
	}

		

       task* t=task_start_func((void*)header->entry_addr);
	    t -> tf -> rip = (unsigned long) header->entry_addr;


    for(int i=0; i < header->prog_tab_num_ents; ++i){
			switch(header->prog_tab[i].seg_type){
			
				case ELF_LOAD:
				{
					draw_string("RUNNER: loading prog head #");
					draw_hex(i);
					draw_string("RUNNER: program header #0 vaddr = ");
					draw_hex(header->prog_tab[i].vaddr);
					draw_string("RUNNER: program herader #0 memsz = ");
					draw_hex(header->prog_tab[i].mem_sz);
			
					draw_string("RUNNER: prog head type = ");
					draw_hex(header->prog_tab[i].seg_type);
			
					draw_string("RUNNER: prog head f_sz = ");
					draw_hex(header->prog_tab[i].f_sz);
			
					void *seg_addr = page_map_paddr((unsigned long )page_lookup_paddr_tab(t->page_tab,page_find_and_alloc_user(t->page_tab, header->prog_tab[i].vaddr,  1)), 1);
					
						mem_set(seg_addr, 0, header->prog_tab[i].mem_sz);

			
					extfs_read_inode_contents(diskman_find_ent(disk_inode), in, seg_addr, header->prog_tab[i].f_sz, header->prog_tab[i].dat_off);
	
					
					page_free_found((unsigned long)seg_addr,1);
			draw_string("RUNNER: successfully loaded prog head #");
			draw_hex(i);
				
				
				
				}
				break;
				default:
					draw_string("RUNNER: WARN: unknown seg type ");
					draw_hex(header->prog_tab[i].seg_type);
				break;
			}
			
	}

    return t->tid;
}
