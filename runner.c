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
#include "syscall.h"



pid_t  runner_spawn_task(unsigned long disk_inode, char *name, char** argv, char** env, unsigned long attrs){
	
	pml4e *old = page_get_curr_tab();

	
    diskman_ent *d;
    if(!(d = diskman_find_ent(disk_inode))){
        draw_string("invalid inode number\n");
        return -EINVAL;
    }
    
    char dname[PATH_MAX];
    mem_set(dname, 0, PATH_MAX);
    sys_dirname(dname, name);
    
	char name_krnl[str_len(name)+4]; //for two dots purposes
	mem_set(name_krnl, 0 , str_len(name)+2); 
	mem_cpy(name_krnl, name, str_len(name));
    page_switch_krnl_tab(); //krnl stuff



	
    siegfried_file *f = d->fopen(disk_inode, name_krnl);
    

   // unsigned long in = extfs_find_finode_from_dir(diskman_find_ent(disk_inode),EXTFS_ROOTDIR_INODE, name);

    if((long)f < 0){

			//if(curr_task)
			//	curr_task->errno = ENOENT;	
			page_switch_tab(old);
			return (pid_t)f;
    }


    elf_head *header = k_obj_alloc(sizeof(elf_head));


    //extfs_read_inode_contents(diskman_find_ent(disk_inode), in, header, 512, 0);
    long read = d->fread(f, header, 0, sizeof(elf_head), 0); //load initial header

		if(read<0){
				k_obj_free(header);
		k_obj_free(f);
				page_switch_tab(old);
		return read;
	}
    if(mem_cmp((char[]){0x7f,'E','L','F'}, header->magic,4)){
		
		k_obj_free(header);
		k_obj_free(f);
		page_switch_tab(old);
		return (pid_t) -ENOEXEC;
	}
			    

	
	header = k_obj_realloc(header,sizeof(elf_head) + header->prog_tab_num_ents * header->prog_tab_ent_sz);


	
	 read = d->fread(f, (void*)((unsigned long)header + sizeof(elf_head)),sizeof(elf_head), header->prog_tab_ent_sz * header->prog_tab_num_ents, 0);

	if(read<0){
				k_obj_free(header);
		k_obj_free(f);
				page_switch_tab(old);
		return read;
	}
    
    if(mem_cmp((char[]){0x7f,'E','L','F'}, header->magic,4)){
		draw_string("second elf header check invalid? something's wrong\n");
		draw_hex((unsigned long)header);
		k_obj_free(header);
		k_obj_free(f);
					page_switch_tab(old);

		return -ENOEXEC;
	}
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

//TODO: there's probably a better a way to do this

	t->name = k_obj_alloc(str_len(name_krnl) + 2);
	char *c = sys_basename(name_krnl);
	(void) mem_cpy (t->name, c, str_len(c));
	
	if(env == 0){ //handle null env
		env = k_obj_alloc(sizeof(char*));
		*env = 0;
	}

	t->env = env;
	
	t->dm_inode = disk_inode;
	t->cwd = k_obj_alloc(sizeof(siegfried_dir));
	int r = syscall_open_dir(dname, t->cwd);
	if(r < 0){
		return -ENOENT;
	}
	//d->fopendir(disk_inode, dname, 0, t->cwd);
	
	t->task_page_ents = header->prog_tab_num_ents;
	t->task_page_ptr = k_obj_alloc(sizeof( struct task_page_ent)*t->task_page_ents);
	
	t->creator = syscall_get_tid();
	
    for(int i=0; i < header->prog_tab_num_ents; ++i){
			switch(header->prog_tab[i].seg_type){
			
				case ELF_LOAD:
				{
				//	draw_string("RUNNER: loading prog head #");
					draw_hex(i);
					draw_string("RUNNER: program header #0 vaddr = ");
					draw_hex(header->prog_tab[i].vaddr);

					draw_string("RUNNER: program herader #0 memsz = ");
					draw_hex(header->prog_tab[i].mem_sz);
					draw_string("foffset=");
					draw_hex(header->prog_tab[i].dat_off);

					
					void *tab = page_get_curr_tab();
					void *seg_addr = page_find_and_alloc_user(t->page_tab, header->prog_tab[i].vaddr,  1);
			
			
					t->task_page_ptr[i].addr = page_lookup_paddr_tab(t->page_tab,(void*)header->prog_tab[i].vaddr);
					t->task_page_ptr[i].pages = 1;
					draw_hex((unsigned long)seg_addr);
					draw_string("RUNNER: program header #0 paddr = ");
					draw_hex((unsigned long)t->task_page_ptr[i].addr);


					
					
					page_switch_tab(t->page_tab);
					
					
					mem_set(seg_addr, 0x5a, header->prog_tab[i].mem_sz);
	
			
					//extfs_read_inode_contents(diskman_find_ent(disk_inode), in, seg_addr, header->prog_tab[i].f_sz, header->prog_tab[i].dat_off);
					d->fread(f, seg_addr, header->prog_tab[i].dat_off, header->prog_tab[i].f_sz, 0);
					
					
					
					page_switch_tab(tab);

				
				
				
				}
				break;
				default:
			//		draw_string("RUNNER: WARN: unknown seg type ");
				//	draw_hex(header->prog_tab[i].seg_type);
				break;
			}
			
	}
	k_obj_free(header);


	
	d->fclose(f);
	
	volatile pid_t ret = t->tid;
	page_switch_tab(old);

	
    return ret;
}
