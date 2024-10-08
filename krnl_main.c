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
#include "rtc.h"
#include "diskman.h"
#include "kb.h"
#include "tasks.h"
#include "syscall.h"
#include "elf.h"

unsigned int* m_info;

extern int _krnl_end;

krnl_state *old_krnl_state, *new_krnl_state;

char krnl_cmdline[4096];
unsigned long krnl_init_inode;
extern int _krnl_start;

				void dbgconchar(char);

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
	page_alloc(0,0);

    for(unsigned long i=((unsigned long)&_krnl_start / 0x200000); i<(((unsigned long)&_krnl_end)/0x200000 + 2);++i){
        page_alloc((void*) (0x200000 * i), (void*) ( 0x200000 * i));
    }
    extern int _krnl_end;
	


    //page_map_paddr(0, (((unsigned long)&_krnl_end+0x5000))/0x200000+(0x1000000/0x200000));
    //while(1){}
    page_flush(); 

    //while(1){}

	    unsigned int sz = m_info_old[0];
    
	char m_info_mem[sz];
	
	m_info = (unsigned int*) m_info_mem;
    mem_cpy(m_info, m_info_old, sz);
    
    unsigned long long fbaddr, fbw, fbh, fbb, fbp;

        struct multiboot_tag_new_acpi *acpitag = 0;

        krnl_init_inode = 5;


    struct multiboot_tag *tag_ptr = (struct multiboot_tag *)&(m_info[2]);
    while(((unsigned long long) tag_ptr - (unsigned long long) m_info) < sz){
     //   dbgnumout(tag_ptr->type);
        struct multiboot_tag_framebuffer *fbtag = (struct multiboot_tag_framebuffer *) tag_ptr;

        switch(tag_ptr->type){
            case MULTIBOOT_TAG_TYPE_CMDLINE:{
                struct multiboot_tag_string *cmdline = (struct multiboot_tag_string*) tag_ptr;

                unsigned long cmdline_strlen = cmdline->size - 4 - 4;

                if(cmdline_strlen > 4096){
                    cmdline_strlen = 4096;
                }

                mem_set(krnl_cmdline, 0, 4096);

                mem_cpy(krnl_cmdline, cmdline->string, cmdline_strlen);


            }
            break;

            case MULTIBOOT_TAG_TYPE_ELF_SECTIONS:{

                struct multiboot_tag_elf_sections *elfptr = (void* ) tag_ptr;
				dbgconout("ELF SYMBOLS:\r\n");
				
				char *str_tab = (char*)((elf_section_header*)elfptr->sections)[elfptr->shndx].sh_addr;

				for(unsigned int i=0;i<elfptr->num;++i){
						elf_section_header * it = &((elf_section_header*)elfptr->sections)[i];
						dbgconout(str_tab + it->sh_name);
						dbgconout(":");
						dbgnumout_hex((unsigned long)it->sh_addr);
				}
				
				

            }
            break;

			case MULTIBOOT_TAG_TYPE_MMAP:{
				struct multiboot_tag_mmap *mmap_tag = (void*) tag_ptr;
				if(mmap_tag->entry_version == 1){}
				multiboot_memory_map_t * ents = mmap_tag->entries;
				
				while((unsigned long)ents < ((unsigned long)mmap_tag->size + (unsigned long)mmap_tag)){
					
					if(ents->type == MULTIBOOT_MEMORY_AVAILABLE){
							page_mark_available_mem_range(ents->addr, ents->len);
					}
					
					ents = (multiboot_memory_map_t*)((unsigned long)ents + mmap_tag->entry_size);
				}

					
			}
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


    if(acpitag != 0){
        acpiman_setup(&acpitag->rsdp);
	}

	
    apic_setup();
    

    syscall_setup();


    rtc_setup();
    timer_setup();
    pci_enum();

drivers_setup();


    draw_string("CMDLINE=\"");
    draw_string(krnl_cmdline);
    draw_string("\"\n");

    str_tok_result res;
    str_tok(krnl_cmdline, ' ', &res);

    do{

        if(!mem_cmp(&krnl_cmdline[res.off], "root=", str_len("root="))){
            unsigned long i=0;
            for(i=0; krnl_cmdline[res.off+i] != '=' && krnl_cmdline[res.off+i] != 0; ++i);
            ++i;

            unsigned long l = 0;

            for(l = 0; krnl_cmdline[res.off+i+l] != 0 && krnl_cmdline[res.off+i+l] != ' '; ++l);

            unsigned long in = atoi_w_sz(&krnl_cmdline[res.off + i],l);

            krnl_init_inode = in;

        }

        str_tok(krnl_cmdline, ' ', &res);

    }
    while(res.sz != 0);

    //load tasks and init loader	
    tasks_setup();

    asm("cli");

    draw_string("STARTING SCHEDULER\r\n");

    //kernel enters scheduler at this point.

    task_scheduler();

    while(1){
 //       syscall0(0);

    }


}
