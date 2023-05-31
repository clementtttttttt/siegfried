#include "page.h"
#include "pageobj_heap.h"
#include "debug.h"

pml4e pml4_table[512] __attribute__ ((aligned (4096)));

inline void set_paddr(void* in, unsigned long inaddr){
    pml4e *t = in;

    inaddr >>= 12;

    t -> paddr = inaddr & 0xFFFFFFF;
    //FIXME: detect bits and avoid writing on reserved shit

    t->paddrU = 0;
    t -> paddrU = inaddr >> 28 & 0xff;

    return;
}

inline void set_paddr_pde(void* in, unsigned long inaddr){
    pde *t = in;

    inaddr >>= 21;

    t -> paddr = inaddr & 0x7FFFF;

    //FIXME: detect bits and avoid writing on reserved shit

    t->paddrU = 0;
    t -> paddrU = (inaddr >> 19) & 0xff;

    return;
}

inline unsigned long get_paddr(void* in){
    pml4e *t = in;

    return ((unsigned long)(t -> paddr) | (t -> paddrU << 28)) << 12;
}

unsigned char phys_mem_map[16777216];
    extern KHEAPSS page_heap;


void page_init_map(){
   // phys_mem_map = k_pageobj_alloc(&page_heap, );

    for(int i=0;i<16777216;++i){
        phys_mem_map[i] = 0;
    }
    for(int i=0;i<512;++i){
        pml4_table[i].raw = 0;
    }
}

unsigned long page_lookup_paddr(unsigned long vir){

    unsigned long off = vir % 2097152;

    unsigned long pml4i = (unsigned long)vir >> 39 & 0x1ff;
    unsigned long pdptei = (unsigned long)vir >> 30 & 0x1ff;
    unsigned long pdei = (unsigned long)vir >> 21 & 0x1ff;

    pdpte *pdpt_table = (pdpte*) get_paddr(&pml4_table[pml4i]);

    if(pdpt_table == 0){
        dbgconout("pdpt_table is 0");
        return vir;


    }

    pde *pdei_table = (pde*) get_paddr(&pdpt_table[pdptei]);

    if(pdei_table == 0){
            dbgconout("pdpt_table is 0");

        return vir;
    }

    return get_paddr(&pdei_table[pdei]) + off;

}

void page_alloc(void *phy, void *vir){
    unsigned long pml4i = (unsigned long)vir >> 39 & 0x1ff;
    unsigned long pdptei = (unsigned long)vir >> 30 & 0x1ff;
    unsigned long pdei = (unsigned long)vir >> 21 & 0x1ff;



    if(pml4_table[pml4i].present == 0){
        set_paddr(&pml4_table[pml4i], (unsigned long)k_pageobj_alloc(&page_heap, 4096));
    }
    pml4_table[pml4i].present = 1;
    pml4_table[pml4i].rw = 1;


    pdpte *pdpt_table = (pdpte*) get_paddr(&pml4_table[pml4i]);

    if(get_paddr(&pdpt_table[pdptei]) == 0 || pdpt_table[pdptei].present == 0){
        set_paddr(&pdpt_table[pdptei],(unsigned long) page_lookup_paddr((unsigned long)k_pageobj_alloc(&page_heap, 4096)));
    }
    pdpt_table[pdptei].present = 1;
    pdpt_table[pdptei].rw = 1;


    pde *pdei_table = (pde*) get_paddr(&pdpt_table[pdptei]);

    set_paddr_pde(&pdei_table[pdei], (unsigned long) phy);

    pdei_table[pdei].present = 1;
    pdei_table[pdei].rw = 1;
    pdei_table[pdei].ps = 1;
    pdei_table[pdei].attr_tab_or_rsvd = 0;

    //mark phys mem usage
    phys_mem_map[(unsigned long)phy / 2097152 / 8] |= (1 << (((unsigned long)phy/2097152)%8));


}

void page_alloc_dev(void *phy, void *vir){
    unsigned long pml4i = (unsigned long)vir >> 39 & 0x1ff;
    unsigned long pdptei = (unsigned long)vir >> 30 & 0x1ff;
    unsigned long pdei = (unsigned long)vir >> 21 & 0x1ff;



    if(pml4_table[pml4i].present == 0){
        set_paddr(&pml4_table[pml4i], (unsigned long)k_pageobj_alloc(&page_heap, 4096));
    }
    pml4_table[pml4i].present = 1;
    pml4_table[pml4i].rw = 1;
    pml4_table[pml4i].write_through = 1;
    pml4_table[pml4i].rsvd = 0;
    pml4_table[pml4i].rsvd4 = 0;


    pdpte *pdpt_table = (pdpte*) get_paddr(&pml4_table[pml4i]);

    if(pdpt_table[pdptei].present == 0){
        set_paddr(&pdpt_table[pdptei],(unsigned long) k_pageobj_alloc(&page_heap, 4096));
    }
    pdpt_table[pdptei].present = 1;
    pdpt_table[pdptei].rw = 1;
    pdpt_table[pdptei].write_through = 1;
    pdpt_table[pdptei].rsvd4 = 0;


    pde *pdei_table = (pde*) get_paddr(&pdpt_table[pdptei]);

    set_paddr(&pdei_table[pdei], (unsigned long) phy);

    pdei_table[pdei].present = 1;
    pdei_table[pdei].rw = 1;
    pdei_table[pdei].ps = 1;
    pdei_table[pdei].write_through = 1;
    pdei_table[pdei].rsvd8 = 0;
    pdei_table[pdei].rsvd4 = 0;

    if(vir == 0){
        dbgconout("ZERO ENTRY ADDR: ");
        dbgnumout_hex((unsigned long)&pdei_table[pdei]);
    }
    //mark phys mem usage
    phys_mem_map[((unsigned long)phy / 2097152 / 8)] |= (1 << ((unsigned long)phy/2097152)%8);

}



inline unsigned char page_physmemmap_is_used(unsigned long paddr){
    return phys_mem_map[paddr/2097152/8] & (1<<((paddr/2097152)%8));
}

unsigned long page_virt_find_addr(unsigned long pgs){

        for(unsigned long addr = 0; addr < 0xFFFFFFFFFFFF; addr += 2097152){

            int mem_not_found = 0;

            unsigned long targ2;

            for(unsigned long addr2 = 0; addr2 < pgs*2097152; addr2 += 2097152){

                targ2 = addr + addr2;

                unsigned long pml4i = targ2 >> 39 & 0x1ff;
                unsigned long pdptei = targ2 >> 30 & 0x1ff;
                unsigned long pdei = targ2 >> 21 & 0x1ff;

                if(pml4_table[pml4i].present == 0){
                    if(pgs < 512*512){
                        return addr;
                    }
                    else{
                    //  addr += 549755813888 - addr% (549755813888);
                        addr2 += 549755813888 - addr2 % (549755813888) - 2097152;
                        continue;
                    }
                }

                pdpte *pdpt_table = (pdpte*) get_paddr(&pml4_table[pml4i]);

                if(pdpt_table[pdptei].present == 0){
                    if(pgs < 512){

                        return addr;
                    }
                    else{
                        addr2 += 1073741824 - addr2%1073741824 - 2097152;
                        continue;
                    }
                }

                //skip to next pdpt
                if(pdpt_table[pdptei].ps == 1){
                    addr += 1073741824 - addr%1073741824 ;
                    continue;
                }



                pde *pdei_table = (pde*) get_paddr(&pdpt_table[pdptei]);



                if(pdei_table[pdei].present == 1){
                    mem_not_found = 1;
                    break;
                }

            }

            if(mem_not_found == 0){

                return addr;
            }

        }
        return 0xDEAD;
}

extern int _krnl_end;

void *page_find_and_alloc(unsigned long pgs){

    for(unsigned long i=0;i < 16777216*8 - pgs; ++i){

        if(page_physmemmap_is_used(i * 2097152)){

            continue;
        }


        unsigned long addr = page_virt_find_addr(pgs);

        for(unsigned long pi = 0; pi < pgs; ++pi){
            unsigned long off = i + pi;

            while(page_physmemmap_is_used(off*2097152)){
                ++i;
                off = i + pi;
            }

            page_alloc((void*)(pi * 2097152 + i*2097152),(void*) (addr + pi*2097152));
        }

    //    dbgconout("PFAA: RETURN ");
      //  dbgnumout_hex(addr);
      for(unsigned long i=0;i<(pgs*2097152/8);++i){
        ((unsigned long*)addr)[i] = 0;
    }

        return (void*)addr;
    }
    return (void*)0xDEAD;

}

void page_free_found(unsigned long in_vaddr, unsigned long pgs){
    for(unsigned long i=0;i<pgs;++i){
        unsigned long vir = in_vaddr + i*2097152;
        unsigned long pml4i = (unsigned long)vir >> 39 & 0x1ff;
        unsigned long pdptei = (unsigned long)vir >> 30 & 0x1ff;
        unsigned long pdei = (unsigned long)vir >> 21 & 0x1ff;

        pde *pdei_table = (pde*) get_paddr(&(((pdpte*)get_paddr(&pml4_table[pml4i]))[pdptei]));

        unsigned long pa = get_paddr(&pdei_table[pdei]);


        //FIXME: remove 48bit hard limit.
        pa &= 0xffffffffffff;


        phys_mem_map[pa/2097152/8] &= ~(1 << ((pa/2097152)%8));


        pdei_table[pdei].present = 0;

    }
    page_flush();
}

void page_flush(){
    asm inline("movq %0, %%rdx \n movq %%rdx, %%cr3"::"r" (&pml4_table):"rdx");

}
