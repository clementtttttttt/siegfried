#include "multiboot2.h"
#include "debug.h"
#include "page.h"
#include "pageobj_heap.h"
#include "obj_heap.h"
#include "klib.h"
#include "pci.h"
#include "idt.h"
#include "draw.h"
#include "drivers.h"
#include "io.h"
#include "timer.h"
#include "apic.h"
#include "acpiman.h"

unsigned int* m_info;

extern int _krnl_end;

void krnl_main(unsigned int bootmagic, unsigned int* m_info_old){

    if(bootmagic == MULTIBOOT2_BOOTLOADER_MAGIC){
        dbgconout("BOOTLOADER MAGIC PASS\r\n");
    }
    else
    {
        dbgconout("BOOTLOADER MAGIC FAIL\r\n");
        return;
    }

    idt_setup();

    k_pageobj_heap_setup();
    page_init_map();
    for(unsigned long i=0;i<(((unsigned long)&_krnl_end)/0x200000 + 1);++i){
        page_alloc((void*) (0x200000 * i), (void*) ( 0x200000 * i));
    }
    page_flush();

    unsigned int sz = m_info_old[0];
    m_info = k_obj_alloc(sz);
    mem_cpy(m_info, m_info_old, sz);


    unsigned long long fbaddr, fbw, fbh, fbb, fbp;

        struct multiboot_tag_new_acpi *acpitag;


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
                fbp = fbtag->common.framebuffer_pitch;

                draw_setup(fbaddr, fbw, fbh, fbb, fbp);
                page_flush();


            break;


            case MULTIBOOT_TAG_TYPE_ACPI_NEW:
                acpitag = (struct multiboot_tag_new_acpi *)tag_ptr;
            break;
        }
        tag_ptr = (struct multiboot_tag*) ((unsigned long long)tag_ptr +  ((tag_ptr->size + 7) & ~7));

    }
    acpiman_setup(&acpitag->rsdp);


    apic_setup();
    timer_setup();



    pci_enum();

    drivers_setup();

    draw_string("REACHED END OF KRNL MAIN\r\n");



 //   char count []= " ";
    while(1){

        //draw_string("THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG. \n the quick brown fox jumps over the lazy dog. \n");
    }


}
