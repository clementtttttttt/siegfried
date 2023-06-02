#include "apic.h"
#include "obj_heap.h"
#include "pci.h"
#include "draw.h"
#include "page.h"
#include "nvme.h"
#include "rtc.h"
#include "pageobj_heap.h"

extern KHEAPSS page_heap;


nvme_ctrl *nvme_ctrl_list;

nvme_ctrl *nvme_new_ctrl(){

    nvme_ctrl *ret;

    if(nvme_ctrl_list == 0){
        ret = nvme_ctrl_list = k_obj_alloc(sizeof(nvme_ctrl));
    }
    else{
        nvme_ctrl *i = nvme_ctrl_list;
        while(i->next){
            i = i->next;
        }
        ret = i -> next = k_obj_alloc(sizeof(nvme_ctrl));

    }
    return ret;
}

void nvme_setup_pci_dev(pci_dev_ent *in){


        //enable pci bus mastering
    unsigned int cmd = pci_read_coni(in->bus, in->dev, in->func, 0x4); //get status + cmd
    cmd |= 1<<2;
    pci_write_coni(in->bus, in->dev, in->func, 0x4, cmd);

    unsigned long bar0_p = pci_read_coni(in->bus, in->dev, in->func, 0x10) | (((unsigned long)pci_read_coni(in->bus, in->dev, in->func, 0x14)) << 32); //assume 64bit bar, nvme is modern

    bar0_p &= 0xFFFFFFFFFFFFFFF0;

    volatile nvme_bar0 *bar0 = page_map_paddr_mmio(bar0_p, 1);


    nvme_ctrl *curr = nvme_new_ctrl();

    curr -> bar = bar0;
    curr -> pci_dev = in;
    curr -> acq_vaddr = (unsigned long)k_pageobj_alloc(&page_heap, 4096);
    curr -> asq_vaddr = (unsigned long)k_pageobj_alloc(&page_heap, 4096);

    unsigned int bar0_anded = (bar0->ctrl_conf & 0xfffffffe); //nvme disable
    bar0->ctrl_conf = bar0_anded;

    bar0->queue_att = 0x003f003f; //64 ents for both queues
    bar0->sub_queue_addr = page_lookup_paddr(curr->asq_vaddr);
    bar0->cmpl_queue_addr = page_lookup_paddr(curr->acq_vaddr);

    bar0->int_disable = 0xffffffff;
    bar0->ctrl_conf = 0x460001;


    //wiat for csts.rdy
    while(!(bar0->ctrl_stat & 1)){
        if(bar0->ctrl_stat & 0b10){
            draw_string("ERROR: NVME CSTS.CFS SET\n");
            return;
        }
    }

    draw_string("NVME CTRL REENABLED\n");


    //get msi cap
    unsigned char cap_off = pci_read_conw(in->bus, in->dev, in->func, 0x34) & 0xfc;

    unsigned char cap_id = (pci_read_conw(in->bus, in->dev, in->func, cap_off));

    while(cap_id != 0x11 && cap_id != 0){
        draw_string("==START CAP_OFF PRNT==\n");
        draw_hex(cap_off);
        draw_hex(cap_id);
        draw_string("==END CAP_ID PRNT==\n");

        cap_off = (pci_read_conw(in->bus, in->dev, in->func, cap_off) >> 8);
        cap_id = (pci_read_conw(in->bus, in->dev, in->func, cap_off));

    }
        draw_string("==START CAP_OFF PRNT==\n");
        draw_hex(cap_off);
        draw_hex(cap_id);
        draw_string("==END CAP_ID PRNT==\n");
    if(cap_id == 0){
        draw_string("NVME MSI CAP NOT FOUND!\n");
        return;
    }

    draw_string("NVME MSIX OFF0x4=");
    draw_hex(pci_read_coni(in->bus, in->dev,in->func, cap_off + 0x4));







}

void nvme_setup(){

    pci_dev_ent* it = 0;

    while((it = pci_get_next_dev_ent(it)) != 0){
        if(it->cl == 1 && it->subcl == 8){
            nvme_setup_pci_dev(it);
        }
    }
}
