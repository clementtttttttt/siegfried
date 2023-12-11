#include "page.h"
#include "pageobj_heap.h"
#include "debug.h"
#include "draw.h"
#include "klib.h"

pml4e pml4_table[512] __attribute__ ((aligned (4096)));

void page_switch_krnl_tab(){
		page_switch_tab(pml4_table);
	
}

static inline void set_paddr(void* in, unsigned long inaddr){
    pml4e *t = in;

    inaddr >>= 12;

    t -> paddr = inaddr & 0xFFFFFFF;
    //FIXME: detect bits and avoid writing on reserved shit

    t->paddrU = 0;
    t -> paddrU = inaddr >> 28 & 0xff;

    return;
}


static inline void set_paddr_pde(void* in, unsigned long inaddr){
    pde *t = in;

    inaddr >>= 21;

    t -> paddr = inaddr & 0x7FFFF;

    //FIXME: detect bits and avoid writing on reserved shit

    t->paddrU = 0;
    t -> paddrU = (inaddr >> 19) & 0xff;

    return;
}

static inline void* get_paddr(void* in){
    pml4e *t = in;

    return (void*)(((unsigned long)(t -> paddr) | (t -> paddrU << 28)) << 12);
}

unsigned char phys_mem_map[16777216];
    extern KHEAPSS page_heap;

void page_clone_page_tab(pml4e *dest, pml4e *src){

    //each pml4i ent represents 512gib
    for(unsigned long pml4i = 0; pml4i < 512; ++pml4i){
        dest[pml4i].raw = src[pml4i].raw;
        dest[pml4i].isuser = 1; //just in case

        if(dest[pml4i].present){

            pdpte *pdpt_table_dest = k_pageobj_alloc(&page_heap, 4096);
            set_paddr(&dest[pml4i],(unsigned long) pdpt_table_dest);

            pdpte *pdpt_table_src = (pdpte*)get_paddr(&src[pml4i]);

            for(unsigned long pdpti = 0; pdpti < 512; ++pdpti){
                pdpt_table_dest[pdpti] = pdpt_table_src[pdpti];
                pdpt_table_dest[pdpti].isuser = 1;

                if(pdpt_table_dest[pdpti].present){
                    pde *pdei_table_src = get_paddr(&pdpt_table_src[pdpti]);
                    pde *pdei_table_dest = k_pageobj_alloc(&page_heap, 4096);
                    set_paddr(&pdpt_table_dest[pdpti], (unsigned long)pdei_table_dest);
                    for(unsigned long pdei = 0; pdei < 512; ++pdei){
                        pdei_table_dest[pdei] = pdei_table_src[pdei];
                            pdei_table_dest[pdei].isuser = 0;


                    }

                }
            }

        }

    }

}

void page_free_tab(pml4e *tab){
	
    //each pml4i ent represents 512gib
    for(unsigned long pml4i = 0; pml4i < 512; ++pml4i){

        if(tab[pml4i].present){

            pdpte *pdpt_table = (pdpte*)get_paddr(&tab[pml4i]);

            for(unsigned long pdpti = 0; pdpti < 512; ++pdpti){


                if(pdpt_table[pdpti].present){
					   pde *pdei_table_src = get_paddr(&pdpt_table[pdpti]);
					   			   
					   k_pageobj_free(&page_heap, pdei_table_src);
                }
            }
            
            k_pageobj_free(&page_heap, pdpt_table);

        }

    }
	
}

void page_clone_krnl_tab(pml4e *dest){
    page_clone_page_tab(dest, pml4_table);
}


void page_init_map(){
   // phys_mem_map = k_pageobj_alloc(&page_heap, );

    for(int i=0;i<16777216;++i){
        phys_mem_map[i] = 0;
    }
    for(int i=0;i<512;++i){
        pml4_table[i].raw = 0;
    }
}

void* page_lookup_paddr_tab(pml4e *tab, void* in){

    unsigned long vir = (unsigned long) in;
    unsigned long off = vir & 0x1fffff;

    unsigned long pml4i = (unsigned long)vir >> 39 & 0x1ff;
    unsigned long pdptei = (unsigned long)vir >> 30 & 0x1ff;
    unsigned long pdei = (unsigned long)vir >> 21 & 0x1ff;

    pdpte *pdpt_table = (pdpte*) get_paddr(&tab[pml4i]);

    if(pdpt_table == 0){
        dbgconout("pdpt_table is 0");
        return (void*)vir;


    }

    pde *pdei_table = (pde*) get_paddr(&pdpt_table[pdptei]);

    if(pdei_table == 0){
            dbgconout("pdpt_table is 0");

        return (void*)vir;
    }

    return get_paddr(&pdei_table[pdei]) + off;
}


pde* page_lookup_pdei(pml4e *tab, void *in){
	
    unsigned long vir = (unsigned long) in;

    unsigned long pml4i = (unsigned long)vir >> 39 & 0x1ff;
    unsigned long pdptei = (unsigned long)vir >> 30 & 0x1ff;
    unsigned long pdei = (unsigned long)vir >> 21 & 0x1ff;

    pdpte *pdpt_table = (pdpte*) get_paddr(&pml4_table[pml4i]);

    if(pdpt_table == 0){
        dbgconout("pdpt_table is 0");
        return (void*)0;


    }

    pde *pdei_table = (pde*) get_paddr(&pdpt_table[pdptei]);

    if(pdei_table == 0){
            dbgconout("pdpt_table is 0");

        return (void*)0;
    }

	return &pdei_table[pdei];
}

void* page_lookup_paddr(void* in){

    unsigned long vir = (unsigned long) in;
    unsigned long off = vir & 0x1fffff;

    unsigned long pml4i = (unsigned long)vir >> 39 & 0x1ff;
    unsigned long pdptei = (unsigned long)vir >> 30 & 0x1ff;
    unsigned long pdei = (unsigned long)vir >> 21 & 0x1ff;

    pdpte *pdpt_table = (pdpte*) get_paddr(&pml4_table[pml4i]);

    if(pdpt_table == 0){
        dbgconout("pdpt_table is 0");
        return (void*)vir;


    }

    pde *pdei_table = (pde*) get_paddr(&pdpt_table[pdptei]);

    if(pdei_table == 0){
            dbgconout("pdpt_table is 0");

        return (void*)vir;
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
    pml4_table[pml4i].isuser = 1;
    pml4_table[pml4i].is_krnl_pg = 1;



    pdpte *pdpt_table = (pdpte*) get_paddr(&pml4_table[pml4i]);

    if(get_paddr(&pdpt_table[pdptei]) == 0 || pdpt_table[pdptei].present == 0){
        set_paddr(&pdpt_table[pdptei],(unsigned long) page_lookup_paddr(k_pageobj_alloc(&page_heap, 4096)));
    }
    pdpt_table[pdptei].present = 1;
    pdpt_table[pdptei].rw = 1;
    pdpt_table[pdptei].isuser = 1;
    pdpt_table[pdptei].is_krnl_pg = 1;


    pde *pdei_table = (pde*) get_paddr(&pdpt_table[pdptei]);

    set_paddr_pde(&pdei_table[pdei], (unsigned long) phy);

    pdei_table[pdei].present = 1;
    pdei_table[pdei].rw = 1;
    pdei_table[pdei].ps = 1;
    pdei_table[pdei].attr_tab_or_rsvd = 0;
    pdei_table[pdei].isuser = 1;
    pdei_table[pdei].is_krnl_pg = 1;

    //mark phys mem usage
    phys_mem_map[(unsigned long)phy / 2097152 / 8] |= (1 << (((unsigned long)phy/2097152)%8));


}

void page_alloc_tab(pml4e *tab, void *phy, void *vir){
    unsigned long pml4i = (unsigned long)vir >> 39 & 0x1ff;
    unsigned long pdptei = (unsigned long)vir >> 30 & 0x1ff;
    unsigned long pdei = (unsigned long)vir >> 21 & 0x1ff;



    if(tab[pml4i].present == 0){
        set_paddr(&tab[pml4i], (unsigned long)k_pageobj_alloc(&page_heap, 4096));
    }
    tab[pml4i].present = 1;
    tab[pml4i].rw = 1;
    tab[pml4i].isuser = 1;

    pdpte *pdpt_table = (pdpte*) get_paddr(&tab[pml4i]);

    if(get_paddr(&pdpt_table[pdptei]) == 0 || pdpt_table[pdptei].present == 0){
        set_paddr(&pdpt_table[pdptei],(unsigned long) page_lookup_paddr(k_pageobj_alloc(&page_heap, 4096)));
    }
    pdpt_table[pdptei].present = 1;
    pdpt_table[pdptei].rw = 1;
    pdpt_table[pdptei].isuser = 1;
    pdpt_table[pdptei].is_krnl_pg = 0;


    pde *pdei_table = (pde*) get_paddr(&pdpt_table[pdptei]);

    set_paddr_pde(&pdei_table[pdei], (unsigned long) phy);

    pdei_table[pdei].present = 1;
    pdei_table[pdei].rw = 1;
    pdei_table[pdei].ps = 1;
    pdei_table[pdei].attr_tab_or_rsvd = 0;
    pdei_table[pdei].isuser = 1;
    pdei_table[pdei].is_krnl_pg = 0;

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
    pml4_table[pml4i].write_through = 0;
    pml4_table[pml4i].rsvd = 0;
    pml4_table[pml4i].rsvd4 = 0;


    pdpte *pdpt_table = (pdpte*) get_paddr(&pml4_table[pml4i]);

    if(pdpt_table[pdptei].present == 0){
        set_paddr(&pdpt_table[pdptei],(unsigned long) k_pageobj_alloc(&page_heap, 4096));
    }
    pdpt_table[pdptei].present = 1;
    pdpt_table[pdptei].rw = 1;
    pdpt_table[pdptei].write_through = 0;
    pdpt_table[pdptei].rsvd4 = 0;


    pde *pdei_table = (pde*) get_paddr(&pdpt_table[pdptei]);

    set_paddr(&pdei_table[pdei], (unsigned long) phy);

    pdei_table[pdei].present = 1;
    pdei_table[pdei].rw = 1;
    pdei_table[pdei].ps = 1;
    pdei_table[pdei].rsvd8 = 0;
    pdei_table[pdei].rsvd4 = 0;
    pdei_table[pdei].attr_tab_or_rsvd = 1;
    pdei_table[pdei].nocache = 1;

    if(vir == 0){
        dbgconout("ZERO ENTRY ADDR: ");
        dbgnumout_hex((unsigned long)&pdei_table[pdei]);
    }
    //mark phys mem usage
    phys_mem_map[((unsigned long)phy / 2097152 / 8)] |= (1 << ((unsigned long)phy/2097152)%8);

}
void page_alloc_mmio(void *phy, void *vir){
    unsigned long pml4i = (unsigned long)vir >> 39 & 0x1ff;
    unsigned long pdptei = (unsigned long)vir >> 30 & 0x1ff;
    unsigned long pdei = (unsigned long)vir >> 21 & 0x1ff;



    if(pml4_table[pml4i].present == 0){
        set_paddr(&pml4_table[pml4i], (unsigned long)k_pageobj_alloc(&page_heap, 4096));
    }
    pml4_table[pml4i].present = 1;
    pml4_table[pml4i].rw = 1;
    pml4_table[pml4i].rsvd = 0;
    pml4_table[pml4i].rsvd4 = 0;


    pdpte *pdpt_table = (pdpte*) get_paddr(&pml4_table[pml4i]);

    if(pdpt_table[pdptei].present == 0){
        set_paddr(&pdpt_table[pdptei],(unsigned long) k_pageobj_alloc(&page_heap, 4096));
    }
    pdpt_table[pdptei].present = 1;
    pdpt_table[pdptei].rw = 1;
    pdpt_table[pdptei].rsvd4 = 0;


    pde *pdei_table = (pde*) get_paddr(&pdpt_table[pdptei]);

    set_paddr(&pdei_table[pdei], (unsigned long) phy);

    pdei_table[pdei].present = 1;
    pdei_table[pdei].rw = 1;
    pdei_table[pdei].ps = 1;
    pdei_table[pdei].rsvd8 = 0;
    pdei_table[pdei].rsvd4 = 0;
    pdei_table[pdei].attr_tab_or_rsvd = 0;
    pdei_table[pdei].nocache = 1;

    if(vir == 0){
        dbgconout("ZERO ENTRY ADDR: ");
        dbgnumout_hex((unsigned long)&pdei_table[pdei]);
    }
    //mark phys mem usage
    phys_mem_map[((unsigned long)phy / 2097152 / 8)] |= (1 << ((unsigned long)phy/2097152)%8);

}



static inline unsigned char page_physmemmap_is_used(unsigned long paddr){
    return phys_mem_map[paddr/2097152/8] & (1<<((paddr/2097152)%8));

}

pml4e *page_get_curr_tab();
asm("page_get_curr_tab:;\
        movq %cr3, %rax;\
        ret;");

void page_unmap_vaddr(void *vaddr){
	
	unsigned long vir = (unsigned long) vaddr;

    unsigned long pml4i = (unsigned long)vir >> 39 & 0x1ff;
    unsigned long pdptei = (unsigned long)vir >> 30 & 0x1ff;

    pdpte *pdpt_table = (pdpte*) get_paddr(&pml4_table[pml4i]);

    if(pdpt_table == 0){
        dbgconout("pdpt_table is 0");
        return;


    }
    
    pdpt_table[pdptei].present = 0;


	
}

unsigned long page_virt_find_addr(unsigned long pgs){

        pml4e *tab = page_get_curr_tab();

        for(unsigned long addr = 0; addr < 0xFFFFFFFFFFFF; addr += 2097152){

            int mem_not_found = 0;

            unsigned long targ2;

            for(unsigned long addr2 = 0; addr2 < pgs*2097152; addr2 += 2097152){

                targ2 = addr + addr2;

                unsigned long pml4i = targ2 >> 39 & 0x1ff;
                unsigned long pdptei = targ2 >> 30 & 0x1ff;
                unsigned long pdei = targ2 >> 21 & 0x1ff;

                if(tab[pml4i].present == 0){
                    if(pgs < 512*512){
                        return addr;
                    }
                    else{
                    //  addr += 549755813888 - addr% (549755813888);
                        addr2 += 549755813888 - addr2 % (549755813888) - 2097152;
                        continue;
                    }
                }

                pdpte *pdpt_table = (pdpte*) get_paddr(&tab[pml4i]);

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


unsigned long page_virt_find_addr_user(pml4e *tab, unsigned long pgs){

       

        for(unsigned long addr = 0; addr < 0xFFFFFFFFFFFF; addr += 2097152){

            int mem_not_found = 0;

            unsigned long targ2;

            for(unsigned long addr2 = 0; addr2 < pgs*2097152; addr2 += 2097152){

                targ2 = addr + addr2;

                unsigned long pml4i = targ2 >> 39 & 0x1ff;
                unsigned long pdptei = targ2 >> 30 & 0x1ff;
                unsigned long pdei = targ2 >> 21 & 0x1ff;

                if(tab[pml4i].present == 0){
                    if(pgs < 512*512){
                        return addr;
                    }
                    else{
                    //  addr += 549755813888 - addr% (549755813888);
                        addr2 += 549755813888 - addr2 % (549755813888) - 2097152;
                        continue;
                    }
                }

                pdpte *pdpt_table = (pdpte*) get_paddr(&tab[pml4i]);

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

void *page_map_paddr(unsigned long paddr,unsigned long pgs){
    unsigned long off = paddr & 0x1fffff;
    paddr &= ~((unsigned long)0x1FFFFF);
    unsigned long vaddr = page_virt_find_addr(pgs);
    for(unsigned long i=0;i<pgs*2097152; i += 2097152){

        page_alloc((void*)(paddr + i),(void*) (vaddr + i));
    }
    return (void*)(vaddr + off);
}

void *page_map_paddr_mmio(unsigned long paddr,unsigned long pgs){
    unsigned long off = paddr & 0x1fffff;
    paddr &= ~((unsigned long)0x1FFFFF);
    unsigned long vaddr = page_virt_find_addr(pgs);
    for(unsigned long i=0;i<pgs*2097152; i += 2097152){
        page_alloc_mmio((void*)(paddr + i),(void*) (vaddr + i));
    }
    return (void*)(vaddr + off);
}

void page_switch_tab(pml4e *tab);
asm(".globl page_switch_tab;\
	page_switch_tab:\
        movq %rdi, %cr3;\
        ret;");

void *page_find_and_alloc_user(pml4e *tab, unsigned long vaddr, unsigned long pgs){
        //page_switch_tab(tab);
        unsigned long addr = vaddr;

    for(unsigned long i=0;i < 16777216*8 - pgs; ++i){

        if(page_physmemmap_is_used(i * 2097152)){

            continue;
        }
        
        if((page_lookup_paddr_tab(tab, (void*)vaddr) > (void*)&_krnl_end) && page_lookup_pdei(tab, (void*)vaddr)->present){
			return (void*)vaddr; // we mapped it already 
		}

		
        for(unsigned long pi = 0; pi < pgs; ++pi){
            unsigned long off = i + pi;

            while(page_physmemmap_is_used(off*2097152)){
                ++i;
                off = i + pi;
            }


            page_alloc_tab(tab, (void*)(pi * 2097152 + i*2097152),(void*) (addr + pi*2097152));
        }

		page_switch_tab(tab);
        mem_set((void*)addr, 0, pgs * 2097152);

		page_switch_krnl_tab();

        return (void*)addr;
    }
	//page_switch_krnl_tab();

    return (void*)0xDEAD;

}

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

        unsigned long pa =(unsigned long) get_paddr(&pdei_table[pdei]);


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
