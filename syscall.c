#include "idt.h"
#include "draw.h"
#include "rtc.h"
#include "timer.h"
#include "tasks.h"
#include "diskman.h"
#include "draw.h"
#include "page.h"
#include "klib.h"
#include "syscall.h"
#include "runner.h"
#include "obj_heap.h"
#include "acpiman.h"

#include "types.h"
#include "errno.h"
void idt_syscall_handler_s();

void syscall_setup(){

    idt_set_irq_ent(0xf0, idt_syscall_handler_s);
    idt_flush();

}


void syscall_sleep(unsigned long in1){



    unsigned long count = rtc_get_count() + in1;

    while(rtc_get_count() < count){

		task_yield();
    }


}

extern diskman_ent *disks;

unsigned long syscall_diskman_get_root(){
	extern unsigned long krnl_init_inode;
		
		return krnl_init_inode; //just returns
}

unsigned long syscall_get_tid(){
		
		return curr_task->tid; //just returns
}



void syscall_diskman_read(unsigned long disk_inode, unsigned long off_sects, unsigned long num_sects, void* buf){

    diskman_ent *e = diskman_find_ent(disk_inode);

    if(e != 0){
    e -> read_func(disk_inode, off_sects, num_sects, page_lookup_paddr_tab(curr_task -> page_tab, buf));
    }

}

void syscall_diskman_write(unsigned long disk_inode, unsigned long off_sects, unsigned long num_sects, void* buf){

    diskman_ent *e = diskman_find_ent(disk_inode);

    if(e != 0){
    e -> write_func(disk_inode, off_sects, num_sects, page_lookup_paddr_tab(curr_task -> page_tab, buf));
    }

}

unsigned long parse_path(char** path){
		unsigned long disk_inode = syscall_diskman_get_root();
		
		if(mem_cmp(*path, "//", 2) == 0){
			*path += 2;
			disk_inode = atoi_w_sz(*path, find_num_len(*path));
			*path += find_num_len(*path);
			
		}

		return disk_inode;
}

siegfried_file *syscall_open(char* path){
	unsigned long disk_inode = parse_path(&path);

    diskman_ent *e = diskman_find_ent(disk_inode);

   
    if(e != 0){

	return e -> fopen (disk_inode, path);
    }
    return (siegfried_file*)-EINVAL;
    

}

int syscall_open_dir(char* path, siegfried_dir *in){
	unsigned long disk_inode = parse_path(&path);

    diskman_ent *e = diskman_find_ent(disk_inode);

    if(e != 0){
		return e -> fopendir (disk_inode, path,0, in);
    }
    return -EINVAL;
    

}

siegfried_dirnames_t *syscall_read_dir(siegfried_dir *in, siegfried_dirnames_t *names){

    diskman_ent *e = diskman_find_ent(in->di);

    if(e != 0){
		return e -> freaddir (in, names);
    }
    return (siegfried_dirnames_t*)-EINVAL;
}

int syscall_close(siegfried_file *f){
	if(!f) return -EINVAL;
	
	diskman_ent *e = f->disk;
	if(e != 0){
		return e -> fclose (f);
    }
    return -EINVAL;
	
}

int syscall_stat(char* path, siegfried_stat *stat){

    siegfried_file *f = syscall_open(path);

    if((long)f > 0){
		int ret= f->disk -> fstat (f, stat);
		syscall_close(f);
		return ret;
    }
    return -EINVAL;
    

}

char *sys_basename(char *path){
	char *end = curr_task->name;
	while(*(end++));
	--end;
	while(end > curr_task->name && (*end != '/')){
		--end;
	}
	++end;
	return end;
}

char *syscall_getcwd(char *buf, size_t size){
	
	//get path without filename only directory
	char *end = curr_task->name;
	
	while(*(end++));
	--end;
	
	while(end > curr_task->name && (*end != '/')){
			--end;
	}
	
	size_t sz2 = end - curr_task->name;
	if(size > sz2) size = sz2;
	
	if(size == 0){ //FIXME: hack for root dir
		++size;
	}
	
	if(curr_task->name[size-1] != '/'){
		curr_task->name[size] = '/';
			++size;

	}
	mem_cpy(buf,curr_task->name, size);
		
	return buf;
}

long syscall_read(siegfried_file *f, void *buf, unsigned long sz_bytes, unsigned long attrs){

	
    if(f != 0){
		return f->disk -> fread (f,buf, f->off, sz_bytes, attrs);
    }
    return -EINVAL;
    

}


long syscall_write(siegfried_file *f, void *buf, unsigned long sz_bytes, unsigned long attrs){


    if(f != 0){
		return f->disk -> fwrite (f,buf, f->off, sz_bytes, attrs);
    }
    return -EINVAL;
    

}


pid_t syscall_spawn(char *path, char** argv, char** env, unsigned long attrs){
	//parse path
	
	unsigned long disk_inode = parse_path(&path);
	
	pid_t ret =  runner_spawn_task(disk_inode, path, argv, env, attrs);
	return ret;
	
}

void syscall_close_dir(siegfried_dir *in){
	diskman_find_ent(in->di)->fclosedir(in->di, in);
}

int syscall_chdir(char *path){
	siegfried_dir d;
	int ret;
	if((ret=syscall_open_dir(path, &d)) < 0){
		return ret;
	}
	
	char *exec_name = sys_basename(curr_task->name);
	
	unsigned long sz = str_len(path) + str_len(curr_task->name) + 4;
	char buf[sz];
	mem_set(buf, 0, sz);
	
	mem_cpy(buf, path, str_len(path));
	
	char *first_end;
	if(buf[str_len(path)-1] != '/'){
		buf[str_len(path) ] = '/';
		first_end = &buf[str_len(path) + 1];
	}
	else{
		first_end = &buf[str_len(path)];
	}
	

	mem_cpy(first_end, exec_name, str_len(exec_name));

	k_obj_free(curr_task->name);
	curr_task->name = k_obj_alloc(sz+1);
	mem_cpy(curr_task->name, buf, sz+1);
	
	
	syscall_close_dir(&d);
	return 0;
}

void syscall_diskman_get_next_ent(syscall_disk_ent *e){
    if(e == 0){
        return;
    }
    register diskman_ent *d = e -> diskman_ent;

    if(d == 0) d = disks;
    else d = d->next;

    //check after getting next
    if(d != 0){
        e -> inode = d -> inode;

        e -> uuid_len = d -> uuid_len;

        mem_cpy(e -> uuid, d -> uuid, d -> uuid_len);
    }

     e -> diskman_ent = d;
}


void syscall_exit(unsigned long code){
		//draw_string("task exits with code");
		//draw_hex(code);
		task_exit(code);
}

void *syscall_mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset){
	void *ret;
	if(addr == 0 || 
	(page_lookup_pdei_tab(curr_task->page_tab, (void*)addr)->present
         && page_lookup_pdei_tab(curr_task->page_tab, (void*)addr)->isuser
	 )){
		addr = (void*)page_virt_find_addr_user(curr_task->page_tab, len / 0x200000); 
	}
	// TODO: prot and flags for mmap
	ret = page_find_and_alloc_user(curr_task->page_tab, (unsigned long)addr, len/0x200000);

	return ret;
}



int syscall_reboot(int arg, unsigned long magic, unsigned long magic2){
	if(magic != SYSCALL_REBOOT_MAGIC && magic!=SYSCALL_REBOOT_MAGIC2){
		
		return -EINVAL;
	}
	
	acpiman_try_reboot();
	//TODO: permission authorise before reboot, ACPI reboot
	asm volatile("lidt 0;int $69");
	
	return 0;
}



void *syscall_table[200] = {syscall_exit, syscall_sleep, draw_string_w_sz, syscall_diskman_get_next_ent, syscall_diskman_read, syscall_diskman_write, syscall_read, syscall_write,syscall_open, syscall_spawn, syscall_diskman_get_root, syscall_get_tid, syscall_stat, syscall_close, syscall_open_dir, syscall_mmap, syscall_getcwd, syscall_read_dir, syscall_close_dir, syscall_chdir, syscall_reboot};

unsigned long syscall_main(unsigned long func,unsigned long i1, unsigned long i2, unsigned long i3, unsigned long i4, unsigned long i5, unsigned long i6){
	
	unsigned long retval = -ENOSYS;
    if(syscall_table[func]){
    	unsigned long i5_r8 = i5;
	unsigned long i6_r9 = i6;
        asm("cli; mov %5, %%r8; mov %6, %%r9; callq *%0; ":"=a"(retval):"r"(syscall_table[func]), "D"(i1), "S"(i2), "d"(i3), "c"(i4), "m"(i5_r8), "m"(i6_r9): "rbx", "r8", "r9");

	
	}
    else{
        draw_string("UNKNOWN SYSCALL ");
        draw_hex (func);
    }
    return retval;
}

