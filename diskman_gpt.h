
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
