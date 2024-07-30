#include "runner.h"
#include "klib.h"
#include "diskman.h"
#include "draw.h"
#include "elf.h"
#include "obj_heap.h"
#include "page.h"
#include "tasks.h"
#include "errno.h"
#include "debug.h"



int  runner_spawn_task(unsigned long disk_inode, char *name, char** argv, char** env, unsigned long attrs){
	
	pml4e *old = page_get_curr_tab();

	
    diskman_ent *d;
    if(!(d = diskman_find_ent(disk_inode))){
        draw_string("invalid inode number\n");
        return 0;
    }
 


    siegfried_file *f = d->fopen(disk_inode, name);
    page_switch_krnl_tab(); //krnl stuff
    
   // unsigned long in = extfs_find_finode_from_dir(diskman_find_ent(disk_inode),EXTFS_ROOTDIR_INODE, name);
    
    if((long)f <= 0){
			draw_string("runner: exec file not found\n");
			//if(curr_task)
			//	curr_task->errno = ENOENT;	
			return 0;
    }

    elf_head *header = k_obj_alloc(sizeof(elf_head));
	
	
    //extfs_read_inode_contents(diskman_find_ent(disk_inode), in, header, 512, 0);
    d->fread(f, header, 0, sizeof(elf_head), 0); //load initial header
	
    if(mem_cmp((char[]){0x7f,'E','L','F'}, header->magic,4)){
		draw_string("invalid elf file\n");
		
		k_obj_free(header);
		k_obj_free(f);
		return 0;
	}
			    draw_string("number of loads: ");
	    draw_hex(header->prog_tab_num_ents);


	
	header = k_obj_realloc(header,sizeof(elf_head) + header->prog_tab_num_ents * header->prog_tab_ent_sz);

	dbg_enable_breakpoint(0,0,BREAK_ON_WRITE, &header, BP_LEN_8);


	draw_string("header addr=");
	draw_hex((unsigned long)header);
	d->fread(f, (void*)((unsigned long)header + sizeof(elf_head)),sizeof(elf_head), header->prog_tab_ent_sz * header->prog_tab_num_ents, 0);


	dbg_disable_breakpoint(0,0);
    
    if(mem_cmp((char[]){0x7f,'E','L','F'}, header->magic,4)){
		draw_string("second elf header check invalid? something's wrong\n");
		draw_hex((unsigned long)header);
		k_obj_free(header);
		k_obj_free(f);
		return 0;
	}
       task* t=task_start_func((void*)header->entry_addr);
	    t -> tf -> rip = (unsigned long) header->entry_addr;
	
	dbg_enable_breakpoint(0,0,BREAK_ON_WRITE, &t->page_tab, BP_LEN_8);

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
	


	draw_string("RUNNER: program page table = ");
	draw_hex((unsigned long) t->page_tab);
	
	draw_string("RUNNER: START OF ELF LOAD\n");
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
			

					
					void *tab = page_get_curr_tab();
					void *seg_addr = page_find_and_alloc_user(t->page_tab, header->prog_tab[i].vaddr,  1);
					
					
					page_switch_tab(t->page_tab);
					
					
						mem_set(seg_addr, 0x5a, header->prog_tab[i].mem_sz);
			
					draw_string("TEST: BEFORE=");
					draw_hex((unsigned long)t->page_tab);
					//extfs_read_inode_contents(diskman_find_ent(disk_inode), in, seg_addr, header->prog_tab[i].f_sz, header->prog_tab[i].dat_off);
					d->fread(f, seg_addr, header->prog_tab[i].dat_off, header->prog_tab[i].f_sz, 0);
					
					page_switch_tab(tab);
			draw_string("RUNNER: successfully loaded prog head #");
			draw_hex(i);
								draw_hex((unsigned long)t->page_tab);

				
				
				
				}
				break;
				default:
			//		draw_string("RUNNER: WARN: unknown seg type ");
				//	draw_hex(header->prog_tab[i].seg_type);
				break;
			}
			
	}

		page_switch_tab(old);

	
	d->fclose(f);

		draw_string("TID=");
		draw_hex(t->tid);
		dbg_disable_breakpoint(0,0);

		
    return t->tid;
}
