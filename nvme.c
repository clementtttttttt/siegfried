#include "pci.h"
#include "draw.h"
#include "page.h"
#include "nvme.h"
#include "rtc.h"

void nvme_setup_pci_dev(pci_dev_ent *in){

    unsigned long bar0_p = pci_read_coni(in->bus, in->dev, in->func, 0x10) | (((unsigned long)pci_read_coni(in->bus, in->dev, in->func, 0x14)) << 32); //assume 64bit bar, nvme is modern

    bar0_p &= 0xFFFFFFFFFFFFFFF0;

    volatile nvme_bar0 *bar0 = page_map_paddr_mmio(bar0_p, 1);

    draw_string("NVME BAR=");
    draw_hex((unsigned long)bar0_p);
    draw_string("NVME VER=");
    draw_hex(bar0->ver);

    //enable pci bus mastering
    unsigned short cmd = pci_read_conw(in->bus, in->dev, in->func, 0x4); //get status + cmd
    cmd &= 1<<2;
    pci_write_conw(in->bus, in->dev, in->func, 0x4, cmd);

    //get msi cap
    unsigned char cap_off = pci_read_conw(in->bus, in->dev, in->func, 0x34) & 0xfc;

    unsigned char cap_id = (pci_read_conw(in->bus, in->dev, in->func, cap_off));

    while(cap_id != 0xc5){
        draw_string("==START CAP_OFF PRNT==\n");
        draw_hex(cap_off);
        draw_hex(cap_id);
        draw_string("==END CAP_ID PRNT==\n");

        cap_off = (pci_read_conw(in->bus, in->dev, in->func, cap_off) >> 8);
        cap_id = (pci_read_conw(in->bus, in->dev, in->func, cap_off));

    }

    draw_string("NVME MSI=");
    draw_hex(pci_read_coni(in->bus, in->dev,in->func, cap_off));







}

void nvme_setup(){

    pci_dev_ent* it = 0;

    while((it = pci_get_next_dev_ent(it)) != 0){
        if(it->cl == 1 && it->subcl == 8){
            nvme_setup_pci_dev(it);
        }
    }
}
