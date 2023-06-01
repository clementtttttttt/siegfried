#include "apic.h"
#include "io.h"
#include "acpiman.h"
#include "draw.h"
#include "page.h"

acpi_madt *madt;

unsigned int* ioapic_addr;

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




                ioapic_addr = page_map_paddr((unsigned long)ce, 1);
                draw_string("IOAPIC VADDR ");
                draw_hex((unsigned long)ioapic_addr);
                break;

        }

        it = (apic_madt_ent_head *)(((unsigned long) it) + it->sz);
    }


}
