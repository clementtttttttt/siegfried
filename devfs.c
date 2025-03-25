#include "devfs.h"
#include "diskman.h"
#include "errno.h"
#include "draw.h"
#include "klib.h"

#include "obj_heap.h"
diskman_ent *devd;



DISKMAN_READ_FUNC(devfs_read_stub){
	
	return -ENOSYS;
}

DISKMAN_WRITE_FUNC(devfs_write_stub){
		return -ENOSYS;
}

DISKMAN_FREAD_FUNC(devfs_fread){
	devfs_ent *it = f->fs_dat;
	
	it->read_func(bytes,off, buf, attrs);

	return -EINVAL;
}


static devfs_ent *devfs_root = 0;

DISKMAN_FOPEN_FUNC(devfs_fopen){
	
	path += 1;
	
	devfs_ent *it = devfs_root;

	while(it){
		if(mem_cmp(it->name, path, str_len(it->name)) == 0){
			siegfried_file *f = k_obj_alloc(sizeof(siegfried_file));
			f->inode = it->inode;
			f->disk = devd;
			f->fs_dat = it;
			return f; 
		}
		it = it->next;
	}

	return (siegfried_file*)-EINVAL;
}

static int curr_inode = 0;


devfs_ent *devfs_make_ent(char *name){
	devfs_ent *it=devfs_root;

	if(it == 0){
		it = devfs_root = k_obj_alloc(sizeof(devfs_ent));
	}
	else{
		while(it->next){
			it=it->next;
		}
		it->next = k_obj_alloc(sizeof(devfs_ent));
		it = it->next;
	}
	mem_cpy(it->name, name, str_len(name));
	it->inode = ++curr_inode;
	return it;
}

void devfs_setup(){
	devd = diskman_new_ent();
	
	devd->fs_type = DISKMAN_FS_DEV;
	devd->read_func = devfs_read_stub;
	devd->write_func = devfs_write_stub;
	devd->fread = devfs_fread;
	devd->fopen = devfs_fopen;

}
