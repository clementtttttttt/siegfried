void devfs_setup();


typedef int(*devfs_read)(int len,int off, char *buf,int attrs);
#define DEVFS_READ_FUNC(in) int in(int len, int off, char * buf,int attrs)

typedef struct devfs_ent{
		struct devfs_ent *next;
		struct devfs_ent *child;
		char name[20];
		devfs_read read_func;
		unsigned long size;
		int inode;
		
}devfs_ent;

devfs_ent *devfs_make_ent(char *name);
