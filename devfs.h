void devfs_setup();
typedef struct devfs_ent{
		struct devfs_ent *next;
		struct devfs_ent *child;
		char name[20];
		unsigned char *buffer;
		unsigned long size;
}devfs_ent;
