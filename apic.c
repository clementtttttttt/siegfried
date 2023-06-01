#include "apic.h"
#include "io.h"
#include "acpiman.h"
#include "draw.h"
#include "page.h"

acpi_madt *madt;

volatile unsigned int* ioapic_addr;

void apic_write_reg(unsigned char idx, unsigned int val){
    ioapic_addr[0] = idx;
    ioapic_addr[0x5] = val;
}

unsigned int apic_read_reg(unsigned char idx){
    ioapic_addr[0] = idx;
    return ioapic_addr[4];
}

void apic_get_redir_ent(unsigned int irq, apic_redir_ent *ent){
    ent->lower_raw = apic_read_reg(0x10 + irq*2);
    ent->upper_raw = apic_read_reg(0x10 + irq*2 + 1);
}

void apic_setup(){

    //disable old pic
    io_outb(0xa1, 0xff);
    io_outb(0x21, 0xff);

    madt = (acpi_madt *)acpiman_get_tab("APIC");

    draw_string("APIC ADDR ");
    draw_hex((unsigned long)madt->apic_addr);

    apic_madt_ent_head *it = &madt->ents_start;
    while((((unsigned long)it) - ((unsigned long)madt)) < madt->header.sz){
        draw_string("FOUND APIC ENT TYPE ");
        draw_hex(it->type);

        apic_ioapic_ent *ce = (apic_ioapic_ent*) it;

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
                    apic_get_redir_ent(0, &ent);
                    draw_hex(apic_read_reg(1));
                    draw_hex((unsigned long)ent.raw);

                }

                break;

        }

        it = (apic_madt_ent_head *)(((unsigned long) it) + it->sz);
    }


}
