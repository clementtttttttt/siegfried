#include "apic.h"
#include "io.h"
#include "acpiman.h"
#include "draw.h"
#include "page.h"
#include "obj_heap.h"

acpi_madt *madt;

volatile unsigned int* ioapic_addr;
volatile unsigned int* apic_addr;

unsigned long cpu0_apic_id;

void apic_write_reg(unsigned char idx, unsigned int val){
    ioapic_addr[0] = idx;
    ioapic_addr[0x4] = val;
}

unsigned int apic_read_reg(unsigned char idx){
    ioapic_addr[0] = idx;
    return ioapic_addr[4];
}

void apic_get_redir_ent(unsigned int irq, apic_redir_ent *ent){
    ent->lower_raw = apic_read_reg(0x10 + irq*2);
    ent->upper_raw = apic_read_reg(0x10 + irq*2 + 1);
}

void apic_write_redir_ent(unsigned int irq, apic_redir_ent *ent){
    apic_write_reg(0x10 + irq*2, ent->lower_raw);
    apic_write_reg(0x10 + irq*2 + 1, ent->upper_raw);
}

apic_cpu_tab *cpus_tab;

void apic_setup(){

    //remap old pic irqs
    io_outb(0x20, 0x11);
    io_wait();
    io_outb(0xa0, 0x11);
    io_wait();
    io_outb(0x21, 0x20);
    io_wait();
    io_outb(0xa1, 0x28);
    io_wait();
    io_outb(0x21, 4);
    io_wait();
    io_outb(0xa1, 2);
    io_wait();

    io_outb(0x21, 0x1);
    io_wait();
    io_outb(0xa1, 0x1);
    io_wait();

    //disable old pic
    io_outb(0xa1, 0xff);
    io_outb(0x21, 0xff);


    //parse madt and stuff
    madt = (acpi_madt *)acpiman_get_tab("APIC");

    draw_string("APIC ADDR ");
    draw_hex((unsigned long)madt->apic_addr);
    apic_addr = page_map_paddr(madt->apic_addr, 1);


    apic_madt_ent_head *it = &madt->ents_start;
    unsigned long cpu_count = 0;;
    while((((unsigned long)it) - ((unsigned long)madt)) < madt->header.sz){
        draw_string("FOUND APIC ENT TYPE ");
        draw_hex(it->type);

        apic_ioapic_ent *ce = (apic_ioapic_ent*) it;
        apic_override_ent *ove = (apic_override_ent*) it;
        apic_cpu_ent *cpe = (apic_cpu_ent*) it;

        switch(it->type){
            case 1: //IO APIC ENT TYPE.

                if(ce->int_base == 0){
                    //only deal with irqs

                    ioapic_addr = page_map_paddr((unsigned long)ce->apic_addr, 1);
                    draw_string("IRQ IOAPIC VADDR ");
                    draw_hex((unsigned long)ioapic_addr);
                    draw_string("IRQ IOAPIC PADDR ");
                    draw_hex((unsigned long)ce->apic_addr);

                    apic_redir_ent ent;
                    apic_get_redir_ent(0x2, &ent);
                    draw_string("REDIR ENT 0 = ");

                    ent.mask = 0;
                    ent.int_num = 0x20;

                    draw_hex(ent.lower_raw);

                    apic_write_redir_ent(2, &ent);

                    apic_get_redir_ent(0, &ent);

                }

                break;

            case 2://INTERRUPT SOURCE ENT TYPE

                draw_string("IRQ SRC: ");
                draw_hex(ove -> irq_src);
                draw_string ("GSINT: ");
                draw_hex(ove -> int_base);

                break;
            case 0: //CPU ID ENTRY

                ++cpu_count;
                cpus_tab = k_obj_realloc(cpus_tab, sizeof(apic_cpu_tab) * cpu_count);

                cpus_tab[cpu_count-1].apic_id = cpe->apic_id;
                cpus_tab[cpu_count-1].cpu_id = cpe->cpu_id;
                cpus_tab[cpu_count-1].flags = cpe->flags;

                draw_string("CPU_ENT FLAGS: ");
                draw_hex(cpus_tab[cpu_count-1].flags);


        }

        it = (apic_madt_ent_head *)(((unsigned long) it) + it->sz);
    }


    apic_addr[0x80/4] = 0;

    //enable receiving irqs on APIC not IOAPIC
    apic_addr[0xf0/4] |= 0x100;
    apic_addr[0xf0/4] |= 0xff;

    draw_string("APIC SIVR=");
    draw_hex(apic_addr[0xf0/4]);



}
