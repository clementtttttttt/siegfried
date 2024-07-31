typedef union pml4e{
    struct{
        unsigned char present : 1;
        unsigned char rw : 1;
        unsigned char isuser : 1;
        unsigned char write_through : 1;
        unsigned char nocache : 1;
        unsigned char isaccessed : 1;
        unsigned char is_krnl_pg : 1;
        unsigned char rsvd : 1;

        unsigned int custom4 : 4;
        unsigned int paddr : 28;
        unsigned char paddrU;

        unsigned short rsvd4 : 4;
        unsigned short custom11 : 11;
        unsigned short noexec : 1;


    } __attribute__((packed));

    unsigned long long raw;
} pml4e;

typedef union pdpte{
    struct{
        unsigned char present : 1;
        unsigned char rw : 1;
        unsigned char isuser : 1;
        unsigned char write_through : 1;
        unsigned char nocache : 1;
        unsigned char isaccessed : 1;
        unsigned char is_krnl_pg : 1;
        unsigned char ps : 1;

        unsigned int custom4 : 4;
        unsigned int paddr : 28;
        unsigned char paddrU;

        unsigned short rsvd4 : 4;
        unsigned short custom11 : 11;
        unsigned short noexec : 1;


    } __attribute__((packed));

    unsigned long long raw;
} pdpte;

typedef union pde{
    struct{
        unsigned char present : 1;
        unsigned char rw : 1;
        unsigned char isuser : 1;
        unsigned char write_through : 1;
        unsigned char nocache : 1;
        unsigned char isaccessed : 1;
        unsigned char dirty : 1;
        unsigned char ps : 1;

        unsigned int glob : 1;
        unsigned int custom3 : 3;

        unsigned int attr_tab_or_rsvd : 1;
        unsigned int rsvd8 : 8;
        unsigned int paddr : 19;
        unsigned char paddrU;

        unsigned short rsvd4 : 4;
        unsigned short avl6 : 6;
        unsigned short is_krnl_pg : 1;
        unsigned short pkey : 4;
        unsigned short noexec : 1;


    } __attribute__((packed));

    unsigned long long raw;
} pde;


typedef struct page_avail_mem_struct{
		unsigned long paddr, len;
		struct page_avail_mem_struct *next;
		
} page_available_mem_ent;
 pml4e *page_get_krnl_tab();
void page_unmark_phys_mem_map(unsigned long in_vaddr, unsigned long pgs);
void page_dump_pde(pde* in);
pde* page_lookup_pdei(pml4e *tab, void *in);
void page_clone_krnl_tab(pml4e *dest);
void page_clone_page_tab(pml4e *dest, pml4e *src);

void page_unmap_vaddr(void* vaddr);
void page_alloc_tab(pml4e *tab, void *phy, void *vir);
void* page_lookup_paddr_tab(pml4e *tab, void* in);
unsigned long page_virt_find_addr_user(pml4e *tab, unsigned long pgs);
void page_alloc(void* phy, void* vir);
void page_alloc_dev(void *phy, void *vir);
void page_flush();
void page_init_map();
void *page_find_and_alloc(unsigned long pgs);
void *page_find_and_alloc_user(pml4e *tab,unsigned long vaddr,  unsigned long pgs);
void page_free_found(unsigned long in_vaddr, unsigned long pgs);
void *page_map_paddr(unsigned long paddr,unsigned long pgs);
void *page_map_paddr_dev(unsigned long paddr,unsigned long pgs);
void *page_map_paddr_mmio(unsigned long paddr,unsigned long pgs);
void *page_lookup_paddr(void* vir);
void page_free_tab(pml4e *tab);
void page_mark_physmemmap(unsigned long paddr);
void page_mark_available_mem_range(unsigned long paddr, unsigned long len);

void page_switch_tab(pml4e *tab);
void page_switch_krnl_tab();
void page_free_found_user(pml4e *tab, unsigned long in_vaddr, unsigned long pgs);
pml4e *page_get_curr_tab();
