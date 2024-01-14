#include "runner.h"
#include "klib.h"
#include "diskman.h"
#include "draw.h"
#include "elf.h"
#include "obj_heap.h"
#include "page.h"
#include "tasks.h"

int runner_spawn_task(unsigned long disk_inode, char *name, char** argv, char** env){

    diskman_ent *d;
    if(!(d = diskman_find_ent(disk_inode))){
        draw_string("invalid inode number\n");
        return 0;
    }
    
    siegfried_file *f = d->fopen(disk_inode, name);
    
    draw_hex((unsigned long)f);
    
   // unsigned long in = extfs_find_finode_from_dir(diskman_find_ent(disk_inode),EXTFS_ROOTDIR_INODE, name);

    
    if((long)f <= 0){
			draw_string("runner: exec file not found\n");
			return 0;
    }

    elf_head *header = k_obj_alloc(sizeof(elf_head));
	
	
    //extfs_read_inode_contents(diskman_find_ent(disk_inode), in, header, 512, 0);
    d->fread(f, header, 0, sizeof(elf_head), 0); //load initial header
	
    if(mem_cmp((char[]){0x7f,'E','L','F'}, header->magic,4)){
		draw_string("invalid elf file");
		
		k_obj_free(header);
		k_obj_free(f);
		return 0;
	}
			    draw_string("number of loads: ");
	    draw_hex(header->prog_tab_num_ents);
	
	void *header2 = k_obj_alloc(sizeof(elf_head) + header->prog_tab_num_ents*header->prog_tab_ent_sz);
	mem_cpy(header2,header, sizeof(elf_head));
		k_obj_free(header);

	header = header2;
	header = k_obj_realloc(header,sizeof(elf_head) + header->prog_tab_num_ents * header->prog_tab_ent_sz);

	d->fread(f, (void*)((unsigned long)header + sizeof(elf_head)),sizeof(elf_head), header->prog_tab_ent_sz * header->prog_tab_num_ents, 0);

       task* t=task_start_func((void*)header->entry_addr);
	    t -> tf -> rip = (unsigned long) header->entry_addr;
	
	unsigned long argc=0;
	if(argv != 0){ //handle nullptr
		while(argv[argc]) ++argc;
	}
	else argc = 0;

	if(argv == 0){
		argv = k_obj_alloc(sizeof(char*));
		*argv = 0; //set to 0, REMINDER: DO NOT FORGET TO FREE THIS WHEN EXITS 
	}

	t->tf->rdi = argc; //argc first argument
	t->tf->rsi = (unsigned long) argv; //argv second argument

	if(env == 0){ //handle null env
		env = k_obj_alloc(sizeof(char*));
		*env = 0;
	}

	t->env = env;

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

			
					//extfs_read_inode_contents(diskman_find_ent(disk_inode), in, seg_addr, header->prog_tab[i].f_sz, header->prog_tab[i].dat_off);
					d->fread(f, seg_addr, header->prog_tab[i].dat_off, header->prog_tab[i].f_sz, 0);
					
					page_unmap_vaddr(seg_addr);
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
