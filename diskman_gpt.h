
void diskman_gpt_enum(diskman_ent *e);

typedef struct gpt_header{ //LBA 1 of gpt

    char magic[8];
    unsigned int rev;
    unsigned int sz;
    unsigned int crc32;
    unsigned int rsvd;
    unsigned long lba_this;
    unsigned long lba_alt;
    unsigned long first_usable;
    unsigned long last_usable;
    char guid [16];
    unsigned long lba_part_ent;
    unsigned int num_parts;
    unsigned int parts_ent_sz;
    unsigned int parts_ents_crc32;

}__attribute__((packed))gpt_header;

typedef struct gpt_partent{

    char type_guid[16];
    __int128 part_guid;
    unsigned long lba_start;
    unsigned long lba_end;
    unsigned long attr;
    char partname[72];

}__attribute__((packed))gpt_partent;

typedef struct gpt_partlist_ent{

    diskman_ent *disk;
    unsigned long lba_start, lba_end;
    unsigned long attr;
    unsigned long inode;
    struct gpt_partlist_ent *next;

}gpt_partlist_ent;
