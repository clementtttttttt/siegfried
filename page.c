#include "page.h"
#include "pageobj_heap.h"
#include "debug.h"
#include "draw.h"
#include "klib.h"
#include "idt.h"
//page allocator 3000

pml4e pml4_table[512] __attribute__ ((aligned (4096)));
void* krnl_tab_addr;
void page_switch_krnl_tab(){
	asm("movq %0, %%cr3"::"r"(krnl_tab_addr));
}

static inline void set_paddr(void* in, unsigned long inaddr){
    pml4e *t = in;

    inaddr >>= 12;

    t -> paddr = inaddr & 0xFFFFFFF;
    //FIXME: detect bits and avoid writing on reserved shit

    t->paddrU = 0;
    t -> paddrU = inaddr >> 28 & 0xff;
    
	t->noexec = 0;
	t->rsvd4 = 0;
	t->rsvd = 0;

    return;
}

static inline void* get_paddr(void* in){
    pml4e *t = in;

    return (void*)(((unsigned long)(t -> paddr) | (t -> paddrU << 28)) << 12);
}

static inline void set_paddr_pde(void* in, unsigned long inaddr){
    pde *t = in;
    inaddr >>= 21;

    t -> paddr = inaddr & 0x7FFFF;

    //FIXME: detect bits and avoid writing on reserved shit

    t->paddrU = 0;
    t -> paddrU = (inaddr >> 19) & 0xff;
    
	t->noexec = 0;
	t->rsvd4 = 0;
	t->rsvd8 = 0;
	t->pkey = 0;


    return;
}



unsigned char phys_mem_map[16777216];
    extern KHEAPSS page_heap;

void page_clone_page_tab(pml4e *dest, pml4e *src){

	pml4e *old_phy = page_get_curr_tab();
	page_switch_krnl_tab();
    //each pml4i ent represents 512gib
    for(unsigned long pml4i = 0; pml4i < 512; ++pml4i){
        dest[pml4i].raw = src[pml4i].raw;

        if(dest[pml4i].present){

            pdpte *pdpt_table_dest = (k_pageobj_alloc(&page_heap, 4096));
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
    page_switch_tab(old_phy);

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
    k_pageobj_free(&page_heap, tab);
	
}

void page_clone_krnl_tab(pml4e *dest){
    page_clone_page_tab(dest, pml4_table);
}


void page_init_map(){
//    phys_mem_map = k_pageobj_alloc(&page_heap, 16777216);
	krnl_tab_addr = &pml4_table;
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

	pml4e *old_phy = page_get_curr_tab();
	page_switch_krnl_tab();

    pdpte *pdpt_table = (pdpte*) get_paddr(&tab[pml4i]);

    if(pdpt_table == 0){
        dbgconout("pdpt_table is 0");
        while(1){}
        return (void*)0;


    }

    pde *pdei_table = (pde*) get_paddr(&pdpt_table[pdptei]);

    if(pdei_table == 0){
            dbgconout("pdpt_table is 0");
        while(1){}

        return (void*)0;
    }

	void *ret=  get_paddr(&pdei_table[pdei]) + off;
	page_switch_tab(old_phy);
	return ret;
}

void page_dump_pde(pde* in){
	draw_string("==BEGIN PAGE TAB ENT DUMP==\n");
	if(in->noexec) draw_string("XD ON\n");
	


	draw_string("PKEY=");
	draw_hex(in->pkey);
	draw_string("IS_KRNL_PG=");
	draw_hex(in->is_krnl_pg);
	draw_string("AVL6=");
	draw_hex(in->avl6);
	draw_string("RSVD4=");
	draw_hex(in->rsvd4);
	draw_string("VADDRU=");
	draw_hex(in->paddrU);
	draw_string("VADDR=");
	draw_hex(in->paddr);
	draw_string("RSVD8=");
	draw_hex(in->rsvd8);
		draw_string("attr_tab_or_rsvd=");
	draw_hex(in->attr_tab_or_rsvd);
		draw_string("CUSTOM3=");
	draw_hex(in->custom3);
	
	draw_string("BITS: ");
	if(in->glob) draw_string("GLOB ");
	if(in->dirty) draw_string("DIRTY ");
	if(in->isaccessed) draw_string("ACCESSED ");
	if(in->nocache) draw_string("NOCACHE ");
	if(in->write_through) draw_string("WRITETHRU ");
	if(in->isuser) draw_string("USER ");
	if(in->rw) draw_string("RW ");
	if(in->present) draw_string("PRSNT ");

	
	
	
	draw_string("\n==END PAGE TAB ENT DUMP==");
}


pde* page_lookup_pdei_tab(pml4e *tab, void *in){
	
    unsigned long vir = (unsigned long) in;

    unsigned long pml4i = (unsigned long)vir >> 39 & 0x1ff;
    unsigned long pdptei = (unsigned long)vir >> 30 & 0x1ff;
    unsigned long pdei = (unsigned long)vir >> 21 & 0x1ff;

    pdpte *pdpt_table = (pdpte*) get_paddr(&tab[pml4i]);

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
    extern int _krnl_end;
    if( vir < ((unsigned long)&_krnl_end+0x400000)){
			return in; //anything below krnl_end and heap is id mapped
	}
	
		pml4e *old_phy = page_get_curr_tab();
	page_switch_krnl_tab();

    unsigned long off = vir & 0x1fffff;

    unsigned long pml4i = (unsigned long)vir >> 39 & 0x1ff;
    unsigned long pdptei = (unsigned long)vir >> 30 & 0x1ff;
    unsigned long pdei = (unsigned long)vir >> 21 & 0x1ff;

    pdpte *pdpt_table = (pdpte*) get_paddr(&pml4_table[pml4i]);

    if(pdpt_table == 0){
        dbgconout("pdpt_table is 0");
		while(1){}
        return (void*)vir;


    }

    pde *pdei_table = (pde*) get_paddr(&pdpt_table[pdptei]);

    if(pdei_table == 0){
            dbgconout("pdpt_table is 0");

        return (void*)vir;
    }


    void *ret= get_paddr(&pdei_table[pdei]) + off;

	page_switch_tab(old_phy);
	return ret;
}

void page_alloc(void *phy, void *vir){
    unsigned long pml4i = (unsigned long)vir >> 39 & 0x1ff;
    unsigned long pdptei = (unsigned long)vir >> 30 & 0x1ff;
    unsigned long pdei = (unsigned long)vir >> 21 & 0x1ff;


    if(pml4_table[pml4i].present == 0){
		unsigned long paddr = (unsigned long)(k_pageobj_alloc(&page_heap, 4096));
        set_paddr(&pml4_table[pml4i], paddr);
    }
    pml4_table[pml4i].present = 1;
    pml4_table[pml4i].rw = 1;
    pml4_table[pml4i].isuser = 1;
    pml4_table[pml4i].is_krnl_pg = 1;



    pdpte *pdpt_table = (pdpte*) get_paddr(&pml4_table[pml4i]);

    if(get_paddr(&pdpt_table[pdptei]) == 0 || pdpt_table[pdptei].present == 0){
		unsigned long paddr = (unsigned long)(k_pageobj_alloc(&page_heap, 4096));
        set_paddr(&pdpt_table[pdptei],paddr);
            pdpt_table[pdptei].isuser = 0;

    }
    pdpt_table[pdptei].present = 1;
    pdpt_table[pdptei].rw = 1;
    pdpt_table[pdptei].is_krnl_pg = 1;


    pde *pdei_table = (pde*) get_paddr(&pdpt_table[pdptei]);

    set_paddr_pde(&pdei_table[pdei], (unsigned long) phy);

    pdei_table[pdei].present = 1;
    pdei_table[pdei].rw = 1;
    pdei_table[pdei].ps = 1;
    pdei_table[pdei].attr_tab_or_rsvd = 0;
    pdei_table[pdei].isuser = 0;
    pdei_table[pdei].is_krnl_pg = 1;

    //mark phys mem usage
    phys_mem_map[(unsigned long)phy / 2097152 / 8] |= (1 << (((unsigned long)phy/2097152)%8));


}

void page_alloc_tab(pml4e *tab, void *phy, void *vir){
    unsigned long pml4i = (unsigned long)vir >> 39 & 0x1ff;
    unsigned long pdptei = (unsigned long)vir >> 30 & 0x1ff;
    unsigned long pdei = (unsigned long)vir >> 21 & 0x1ff;



    if(tab[pml4i].present == 0){
				unsigned long paddr = (unsigned long)(k_pageobj_alloc(&page_heap, 4096));

        set_paddr(&tab[pml4i], paddr);
    }
    tab[pml4i].present = 1;
    tab[pml4i].rw = 1;
    tab[pml4i].isuser = 1;
    tab[pml4i].noexec = 0;

    pdpte *pdpt_table = (pdpte*) get_paddr(&tab[pml4i]);

    if(get_paddr(&pdpt_table[pdptei]) == 0 || pdpt_table[pdptei].present == 0){
				unsigned long paddr = (unsigned long)(k_pageobj_alloc(&page_heap, 4096));

        set_paddr(&pdpt_table[pdptei],paddr);
    }
    pdpt_table[pdptei].present = 1;
    pdpt_table[pdptei].rw = 1;
    pdpt_table[pdptei].isuser = 1;
    pdpt_table[pdptei].is_krnl_pg = 0;
    pdpt_table[pdptei].noexec = 0;


    pde *pdei_table = (pde*) get_paddr(&pdpt_table[pdptei]);

    set_paddr_pde(&pdei_table[pdei], (unsigned long) phy);

    pdei_table[pdei].present = 1;
    pdei_table[pdei].rw = 1;
    pdei_table[pdei].ps = 1;
    pdei_table[pdei].attr_tab_or_rsvd = 0;
    pdei_table[pdei].isuser = 1;
    pdei_table[pdei].is_krnl_pg = 0;
    pdei_table[pdei].noexec = 0;

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
        
        
 pml4e *page_get_krnl_tab(){
		return krnl_tab_addr;
}

void page_unmap_vaddr(void *vaddr){
	
        unsigned long vir =(unsigned long) vaddr ;
        unsigned long pml4i = (unsigned long)vir >> 39 & 0x1ff;
        unsigned long pdptei = (unsigned long)vir >> 30 & 0x1ff;
        unsigned long pdei = (unsigned long)vir >> 21 & 0x1ff;

        pde *pdei_table = (pde*) get_paddr(&(((pdpte*)get_paddr(&pml4_table[pml4i]))[pdptei]));

        

        //FIXME: remove 48bit hard limit.
       



        pdei_table[pdei].present = 0;

	
}

unsigned long page_virt_find_addr(unsigned long pgs){

        pml4e *tab = page_get_curr_tab();
        
        page_switch_krnl_tab();

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
						page_switch_tab(tab);
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
						page_switch_tab(tab);

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
				page_switch_tab(tab);
                return addr;
            }

        }
        page_switch_tab(tab);
        return 0xDEAD;
}


unsigned long page_virt_find_addr_user(pml4e *tab, unsigned long pgs){
		pml4e *tee = page_get_curr_tab();
		
		page_switch_krnl_tab();
       

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
				page_switch_tab(tee);
				

                return addr;
            }

        }
        
        page_switch_tab(tee);
        return 0xDEAD;
}


extern int _krnl_end;

void *page_map_paddr(unsigned long paddr,unsigned long pgs){
    unsigned long off = paddr & 0x1fffff;
    paddr &= ~((unsigned long)0x1FFFFF);

    
    unsigned long vaddr = page_virt_find_addr(pgs);
    if(vaddr == 0xdead){
	return (void*)0;
    }
    for(unsigned long i=0;i<pgs*2097152; i += 2097152){

        page_alloc((void*)(paddr + i),(void*) (vaddr + i));
    }
    return (void*)(vaddr + off);
}

void *page_map_paddr_mmio(unsigned long paddr,unsigned long pgs){
    unsigned long off = paddr & 0x1fffff;
    paddr &= ~((unsigned long)0x1FFFFF);
    unsigned long vaddr = page_virt_find_addr(pgs);
    if(vaddr == 0xdead){
		return (void*)0;
    }
    
    for(unsigned long i=0;i<pgs*2097152; i += 2097152){
        page_alloc_mmio((void*)(paddr + i),(void*) (vaddr + i));
    }
    return (void*)(vaddr + off);
}

void page_switch_tab(pml4e *tab){
		page_switch_krnl_tab();
			tab = page_lookup_paddr(tab);
		
		asm volatile("\
        movq %0, %%cr3;\
        "::"r"(tab):);
 }

void *page_find_and_alloc_user(pml4e *tab, unsigned long vaddr, unsigned long pgs){
	pml4e *old_tab = page_get_curr_tab();
	
	page_switch_krnl_tab();
        unsigned long addr = vaddr;

    for(unsigned long i=0;i < 16777216*8 - pgs; ++i){

        if(page_physmemmap_is_used(i * 2097152)){

            continue;
        }
        
        if((page_lookup_paddr_tab(tab, (void*)vaddr) > (void*)&_krnl_end) && page_lookup_pdei_tab(tab, (void*)vaddr)->present && page_lookup_pdei_tab(tab, (void*)vaddr)->isuser){
			page_switch_tab(old_tab);
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


		volatile unsigned long *test_ptr = (volatile unsigned long*)addr;
		for(unsigned long i=0; i<pgs*2097152/8;++i){
				test_ptr[i] = i+3;


				if(test_ptr[i] != i+3){
						draw_string("MEMORY ERROR IN PAGE_FIND_AND_ALLOC_USER\nVADDR=");
						draw_hex((unsigned long)&(test_ptr)[i]);
						draw_string("PADDR=");
						draw_hex((unsigned long)page_lookup_paddr_tab(tab, (void*)&test_ptr[i]));
						page_switch_krnl_tab();
						idt_print_stacktrace_depth(__builtin_frame_address(0), 3);
						halt_and_catch_fire();
				}
		}
		        mem_set((unsigned long*)test_ptr, 0, pgs * 2097152);

		page_switch_tab(old_tab);
		
        return (void*)addr;
    }
	//page_switch_krnl_tab();
		page_switch_tab(old_tab);

	draw_string("OUT OF PAGES");
	while(1){}
    return (void*)0xDEAD;

}

void *page_find_and_alloc(unsigned long pgs){

    for(unsigned long i=0;i < 16777216*8 - pgs; ++i){

        if(page_physmemmap_is_used(i * 2097152)){

            continue;
        }

		
        unsigned long addr;
			addr = page_virt_find_addr(pgs);

		
        for(unsigned long pi = 0; pi < pgs; ++pi){
            unsigned long off = i + pi;

            while(page_physmemmap_is_used(off*2097152)){
                ++i;
                off = i + pi;
            }

            page_alloc((void*)(pi * 2097152 + i*2097152),(void*) (addr + pi*2097152));
        }
		
		for(unsigned long i=0; i<pgs*2097152/8;++i){
				((unsigned long*)addr)[i] = i;
				if(((unsigned long*)addr)[i] != i){
						draw_string("MEMORY ERROR IN PAGE_FIND_AND_ALLOC, addr=");
						draw_hex((unsigned long)&((unsigned long*)addr)[i]);
						halt_and_catch_fire();
				}
		}
    //    dbgconout("PFAA: RETURN ");
      //  dbgnumout_hex(addr);
		mem_set((void*)addr, 0, 0x200000*pgs);

        return (void*)addr;
    }
    return (void*)0xDEAD;

}

void page_free_found(unsigned long in_vaddr, unsigned long pgs){
			pml4e *old_phy = page_get_curr_tab();
		page_switch_krnl_tab();
        
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
    
    page_switch_tab(old_phy);
    page_flush();
}

void page_free_found_user(pml4e *tab, unsigned long in_vaddr, unsigned long pgs){
   			pml4e *old_phy = page_get_curr_tab();
		page_switch_krnl_tab();
    for(unsigned long i=0;i<pgs;++i){
        unsigned long vir = in_vaddr + i*2097152;
        unsigned long pml4i = (unsigned long)vir >> 39 & 0x1ff;
        unsigned long pdptei = (unsigned long)vir >> 30 & 0x1ff;
        unsigned long pdei = (unsigned long)vir >> 21 & 0x1ff;

        pde *pdei_table = (pde*) get_paddr(&(((pdpte*)get_paddr(&tab[pml4i]))[pdptei]));

        unsigned long pa =(unsigned long) get_paddr(&pdei_table[pdei]);


        //FIXME: remove 48bit hard limit.
        pa &= 0xffffffffffff;


        phys_mem_map[pa/2097152/8] &= ~(1 << ((pa/2097152)%8));


        pdei_table[pdei].present = 0;

    }
        page_switch_tab(old_phy);

    page_flush();
}


void page_flush(){
    asm inline("movq %0, %%rdx \n movq %%rdx, %%cr3"::"r" (&pml4_table):"rdx");

}
