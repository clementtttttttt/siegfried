typedef unsigned long (*diskman_read_func_t) (unsigned long id, unsigned long off_sects, unsigned long num_sects, void* buf);
typedef unsigned long (*diskman_write_func_t) (unsigned long id, unsigned long off_sects, unsigned long num_sects, void* buf);

#define DISKMAN_READ_FUNC(name) unsigned long name (unsigned long id, unsigned long off_sects, unsigned long num_sects, void* buf)
#define DISKMAN_WRITE_FUNC(name) unsigned long name (unsigned long id, unsigned long off_sects, unsigned long num_sects, void* buf)

typedef struct diskman_ent{
    struct diskman_ent *next;

    char *uuid; //MBR/GPT IDENTIFIER
    unsigned long inode;

    diskman_read_func_t read_func;
    diskman_write_func_t write_func;


}diskman_ent;

diskman_ent *diskman_new_ent();
void diskman_setup();
