#include "multiboot2.h"
#include "debug.h"
#include "page.h"
#include "pageobj_heap.h"
#include "obj_heap.h"
#include "klib.h"
#include "pci.h"
#include "idt.h"

unsigned long long fbaddr, fbw, fbh, fbb;

unsigned int* m_info;

unsigned long addr[50000];

void krnl_main(unsigned int bootmagic, unsigned int* m_info_old){

    if(bootmagic == MULTIBOOT2_BOOTLOADER_MAGIC){
        dbgconout("BOOTLOADER MAGIC PASS\r\n");
    }
    else
    {
        dbgconout("BOOTLOADER MAGIC FAIL\r\n");
        return;
    }

    k_pageobj_heap_setup();

    unsigned int sz = m_info_old[0];
    m_info = k_obj_alloc(sz);
    mem_cpy(m_info, m_info_old, sz);

    idt_setup();

    struct multiboot_tag *tag_ptr = (struct multiboot_tag *)&(m_info[2]);
    while(((unsigned long long) tag_ptr - (unsigned long long) m_info) < sz){
     //   dbgnumout(tag_ptr->type);
        struct multiboot_tag_framebuffer *fbtag = (struct multiboot_tag_framebuffer *) tag_ptr;
        switch(tag_ptr->type){
            case MULTIBOOT_TAG_TYPE_CMDLINE:

            break;

            case MULTIBOOT_TAG_TYPE_FRAMEBUFFER:


                fbaddr = fbtag->common.framebuffer_addr;
                fbw = fbtag->common.framebuffer_width;
                fbh = fbtag->common.framebuffer_height;
                fbb = fbtag->common.framebuffer_bpp;




            break;
        }
        tag_ptr = (struct multiboot_tag*) ((unsigned long long)tag_ptr +  ((tag_ptr->size + 7) & ~7));

    }


    page_init_map();


    for(unsigned long i=0;i<0x40;++i){
        page_alloc((void*) (0x200000 * i), (void*) ( 0x200000 * i));
    }


    for(unsigned long i=0;i<0x10;++i){
        page_alloc_dev((void*) (fbaddr + 0x200000 * i), (void*) 0xFE000000000 + 0x200000 * i);
    }
    fbaddr = 0xfe000000000;


    page_flush();

    pci_enum();

    dbgconout("REACHED END OF KRNL MAIN\r\n");

        unsigned int count = 0xff;




    while(1){


        extern unsigned char phys_mem_map[];
        for(unsigned long i = 0; i < fbw*fbh; ++i){
            if(phys_mem_map[i/8] & (1 << ((i)%8))){
                ((unsigned int*)fbaddr)[i] = 0x11111111 * (count/12);
            }
        }
        count++;

    }


}
