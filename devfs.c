#include "devfs.h"
#include "diskman.h"
#include "errno.h"

DISKMAN_READ_FUNC(devfs_read_stub){
		return -ENOSYS;
}

DISKMAN_WRITE_FUNC(devfs_write_stub){
		return -ENOSYS;
}
void devfs_setup(){
	diskman_ent *devd = diskman_new_ent();
	
	devd->fs_type = DISKMAN_FS_DEV;
	devd->read_func = devfs_read_stub;
	devd->write_func = devfs_write_stub;
}
