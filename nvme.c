#include "apic.h"
#include "obj_heap.h"
#include "pci.h"
#include "draw.h"
#include "page.h"
#include "nvme.h"
#include "rtc.h"
#include "pageobj_heap.h"
#include "klib.h"

extern KHEAPSS page_heap;


nvme_ctrl *nvme_ctrl_list;

nvme_ctrl *nvme_new_ctrl(){

    nvme_ctrl *ret;

    if(nvme_ctrl_list == 0){
        ret = nvme_ctrl_list = k_obj_calloc(sizeof(nvme_ctrl),1);
    }
    else{
        nvme_ctrl *i = nvme_ctrl_list;
        while(i->next){
            i = i->next;
        }
        ret = i -> next = k_obj_calloc(sizeof(nvme_ctrl),1);

    }
    return ret;
}

void nvme_send_admin_cmd(nvme_ctrl *c, nvme_sub_queue_ent *e){
    mem_cpy((void*)(c->asq_vaddr + c->a_tail_i), e, sizeof(nvme_sub_queue_ent));

    unsigned int old_atail_i = c->a_tail_i;
    c->a_tail_i = (c->a_tail_i + 1) & 0x3f; //lesser than 64 only

    c->bar->sub_queue_tail_doorbell = c->a_tail_i;

        draw_hex((unsigned long)(&c->bar->sub_queue_tail_doorbell) - (unsigned long)(c->bar));

    while(c->acq_vaddr[old_atail_i].cint3_raw == 0){

    }
        draw_string("CINT3_RAW=");
        draw_hex(c->acq_vaddr[old_atail_i].cint3_raw);

    c->acq_vaddr[old_atail_i].cint3_raw = 0; //overwrite to 0



}

void nvme_setup_pci_dev(pci_dev_ent *in){


        //enable pci bus mastering
    unsigned int pcicmdreg = pci_read_coni(in->bus, in->dev, in->func, 0x4); //get status + cmd
    pcicmdreg |= 1<<2;
    pci_write_coni(in->bus, in->dev, in->func, 0x4, pcicmdreg);

    unsigned long bar0_p = pci_read_coni(in->bus, in->dev, in->func, 0x10) | (((unsigned long)pci_read_coni(in->bus, in->dev, in->func, 0x14)) << 32); //assume 64bit bar, nvme is modern

    bar0_p &= 0xFFFFFFFFFFFFFFF0;

    volatile nvme_bar0 *bar0 = page_map_paddr_mmio(bar0_p, 1);


    nvme_ctrl *curr = nvme_new_ctrl();

    curr -> bar = bar0;
    curr -> pci_dev = in;
    curr -> acq_vaddr = (nvme_cmpl_queue_ent *)k_pageobj_alloc(&page_heap, 4096);
    curr -> asq_vaddr = (nvme_sub_queue_ent*)k_pageobj_alloc(&page_heap, 4096);

    unsigned int bar0_anded = (bar0->ctrl_conf & 0xfffffffe); //nvme disable
    bar0->ctrl_conf = bar0_anded;

    bar0->queue_att = 0x003f003f; //64 ents for both queues
    bar0->sub_queue_addr = (nvme_sub_queue_ent *)page_lookup_paddr((unsigned long)curr->asq_vaddr); //pointer to an array of sz 64
    bar0->cmpl_queue_addr = (nvme_cmpl_queue_ent*) page_lookup_paddr((unsigned long)curr->acq_vaddr);

    bar0->int_disable = 0xffffffff;
    bar0->ctrl_conf = 0x460001;
    draw_hex((unsigned long)bar0->sub_queue_addr);

    //wiat for csts.rdy
    while(!(bar0->ctrl_stat & 1)){
        if(bar0->ctrl_stat & 0b10){
            draw_string("ERROR: NVME CSTS.CFS SET\n");
            return;
        }
    }

    draw_string("NVME CTRL REENABLED\n");

    //io cmpl queue create;

    nvme_sub_queue_ent cmd = {0};
    cmd.cint0.opcode = 5;

    unsigned long old;

    cmd.prp1 = page_lookup_paddr(old = (unsigned long)k_pageobj_alloc(&page_heap, 4096));

    draw_hex(old);
    draw_hex(cmd.prp1);


    cmd.cint11 = 0x00000001; //pc enabled
    cmd.cint10 = 0x003f0001; //queue id 1, 64 ents//
    cmd.nsid = 0;
    nvme_send_admin_cmd(curr, &cmd);



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
