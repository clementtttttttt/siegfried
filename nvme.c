#include "pci.h"
#include "draw.h"
#include "page.h"
#include "nvme.h"

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




}

void nvme_setup(){

    pci_dev_ent* it = 0;

    while((it = pci_get_next_dev_ent(it)) != 0){
        if(it->cl == 1 && it->subcl == 8){
            nvme_setup_pci_dev(it);
        }
    }
}
