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
void syscall_new_handler_s();
void syscall_wrmsr(unsigned int addr, unsigned int high, unsigned int low){
	asm("wrmsr"::"c"(addr),"a"(low),"d"(high));
}


void syscall_setup(){
	syscall_wrmsr(0xc0000082, ((uint64_t)syscall_new_handler_s)>>32, ((uint32_t)(uint64_t)syscall_new_handler_s)&0xffffffff);
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

ino_t syscall_diskman_get_root(void){
	extern unsigned long krnl_init_inode;
		
		return krnl_init_inode; //just returns
}

pid_t syscall_get_tid(void){
		if(curr_task == 0) return 0; //krnl pid = 0;
		
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
	char *end = path;
	while(*(end++));
	--end;
	while(end > path && (*end != '/')){
		--end;
	}
	++end;
	return end;
}


char *sys_dirname(char *buf,char *path){
	char *end = path;
	while(*(end++));
	--end;
	while(end > path && (*end != '/')){
		--end;
	}
	++end;

	//todo: secure mempy
	(void)mem_cpy(buf, path, end - path);
	return end;
}

size_t sys_get_dir_name(char *buf, size_t s,diskman_ent *d, siegfried_dir* in){ //returns offset for basenaem
	
	if(in->inode == d->get_root_inode()){
			//FIXME: error checking for this
			buf[0] = '/';
			return 1;
	}
	
	siegfried_dir par;
	int ret = syscall_open_dir("..",&par);
	if(ret < 0){
		return ret;
	}
		
		siegfried_dir *old_cwd = curr_task->cwd;
		curr_task->cwd = &par;
	syscall_close_dir(&par);

	
	char self_name[NAME_MAX];
	mem_set(self_name, 0, NAME_MAX);
	d->get_name_from_parent(d, in->parent, in->inode, self_name);
	

	size_t path_off = sys_get_dir_name(buf, s, d, &par);

	

	if((s-path_off) > 0)
		(void)mem_cpy(buf+path_off, self_name, str_len(self_name));
	path_off += str_len(self_name);
	if(path_off == (s-1)){
		buf[path_off] = 0;
		return s;
	}
	else{
		buf[path_off++] = '/';
		buf[path_off] = 0;
	}

	
		curr_task->cwd = old_cwd;

	//return 0;
	return path_off;
}
char *syscall_getcwd(char *buf, size_t s){
	//TODO: implement getcwd with more robust 
	mem_set(buf, 0, s);

	(void)sys_get_dir_name(buf, s, diskman_find_ent(curr_task->dm_inode),curr_task->cwd);
	return buf;
	//return "";
}
/*
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
					curr_task->name[size] = 0;


	}
	mem_cpy(buf,curr_task->name, size);
		draw_string(buf);
		draw_string(" ");

		
	return buf;
}*/

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
	
	ino_t disk_inode = parse_path(&path);
	
	pid_t ret =  runner_spawn_task(disk_inode, path, argv, env, attrs);
	return ret;
	
}

void syscall_close_dir(siegfried_dir *in){
	diskman_find_ent(in->di)->fclosedir(in->di, in);
}

int syscall_chdir(char *path){
	siegfried_dir *d = k_obj_alloc(sizeof(siegfried_dir));
	int ret;
	if((ret=syscall_open_dir(path, d)) < 0){
		k_obj_free(d);
		return ret;
	}
	
	syscall_close_dir(curr_task->cwd);
	curr_task->cwd = d;
	
	

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

void syscall_gmsg(syscall_msg_t *dest){
	syscall_msg_t *m = task_msgqueue_pop();
	
	mem_cpy(dest, m, sizeof(syscall_msg_t));
	
	k_obj_free(m);

}

void syscall_pmsg_1(pid_t dest, pid_t src, syscall_msg_type_t t){
	syscall_msg_t *msg = task_nmsg(dest,src,t,0);
	task_msgqueue_push(msg);
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



void *syscall_table[200] = {syscall_exit, syscall_sleep, draw_string_w_sz, syscall_diskman_get_next_ent, syscall_diskman_read, syscall_diskman_write, syscall_read, syscall_write,syscall_open, syscall_spawn, syscall_diskman_get_root, syscall_get_tid, syscall_stat, syscall_close, syscall_open_dir, syscall_mmap, syscall_getcwd, syscall_read_dir, syscall_close_dir, syscall_chdir, syscall_reboot, syscall_gmsg};

long syscall_main(unsigned long func,unsigned long i1, unsigned long i2, unsigned long i3, unsigned long i4, unsigned long i5, unsigned long i6){
	
	 long retval = -ENOSYS;
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

