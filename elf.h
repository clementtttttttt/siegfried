
typedef struct elf_prog_head_ent{

    unsigned int seg_type;
    unsigned int flags;
    unsigned long dat_off;
    unsigned long vaddr;
    unsigned long ign;
    unsigned long f_sz;
    unsigned long mem_sz;
    unsigned long alignment;

}__attribute__((packed)) elf_prog_head_ent;



typedef struct elf_head{
    unsigned char magic[4]; //"\7fELF"
    unsigned char bits;
    unsigned char endian; //1 = little
    unsigned char head_ver;
    unsigned char abi;
    unsigned long pad;
    unsigned short type;
    unsigned short arch; //0x3e = x86_64
    unsigned int elf_ver;
    unsigned long entry_addr;
    unsigned long prog_tab_addr;
    unsigned long sect_tab_addr;
    unsigned int flags;
    unsigned short head_sz;
    unsigned short prog_tab_ent_sz;
    unsigned short prog_tab_num_ents;
    unsigned short sect_tab_ent_sz;
    unsigned short sect_tab_num_ents;
    unsigned short sect_tab_name_idx;
	
	elf_prog_head_ent prog_tab[0];

}__attribute__((packed)) elf_head;

enum elfsegtypes{
	ELF_NULL, ELF_LOAD, ELF_DYNAMIC, ELF_INTERP, ELF_NOTE
	
};
