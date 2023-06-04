#include "apic.h"
#include "obj_heap.h"
#include "pci.h"
#include "draw.h"
#include "page.h"
#include "nvme.h"
#include "rtc.h"
#include "pageobj_heap.h"
#include "klib.h"
#include "diskman.h"

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

nvme_disk *nvme_new_disk(nvme_disk ** in){

    if(*in == 0){
        *in = k_obj_calloc(sizeof(nvme_disk), 1);
    }
    else{

        nvme_disk *i = *in;

        while((i) -> next){
            i = i -> next;
        }
        (i) -> next = k_obj_calloc(sizeof(nvme_disk), 1);

        return i->next;
    }
    return *in;

}

void nvme_send_admin_cmd(nvme_ctrl *c, nvme_sub_queue_ent *e){
    mem_cpy((void*)(c->asq_vaddr + c->a_tail_i), e, sizeof(nvme_sub_queue_ent));

    unsigned int old_atail_i = c->a_tail_i;
    c->a_tail_i = (c->a_tail_i + 1) & 0x3f; //lesser than 64 only

    c->bar->sub_queue_tail_doorbell = c->a_tail_i;

    while(c->acq_vaddr[old_atail_i].cint3_raw == 0){

    }


    c->acq_vaddr[old_atail_i].cint3_raw = 0; //overwrite to 0

}

unsigned short io_cmdid_c;

//sector is 512
void nvme_send_io_cmd(nvme_disk *in, unsigned long off_sects, unsigned long opcode, unsigned long num_sects, void *buf){

        draw_string("O=");
        draw_hex(off_sects);
        draw_string("N=");
        draw_hex(num_sects);

    nvme_sub_queue_ent cmd = {0};

    mem_set(&cmd, 0, sizeof(nvme_sub_queue_ent));;

    cmd.cint0.cid = ++io_cmdid_c;
    cmd.cint0.opcode = opcode;
    cmd.nsid = in->id;
    cmd.prp1 = (unsigned long)buf;

    if(num_sects >= (4096 / 512)){
        cmd.prp2 = (unsigned long)buf + 2048;
    }

    cmd.cint10 = off_sects & 0xffffffff;
    cmd.cint11 = off_sects >> 32;

    cmd.cint12 = (num_sects & 0xffffffff) - 1;


    mem_cpy((void*)(in->ctrl->isq_vaddr + in->ctrl->io_tail_i), &cmd, sizeof(nvme_sub_queue_ent));

    unsigned short old_iotail_i = in->ctrl->io_tail_i;

    in->ctrl->io_tail_i = (in->ctrl->io_tail_i + 1) & 0x3f;
    in->ctrl->bar->io_sub_queue_tail_doorbell = in->ctrl->io_tail_i;

    draw_string("NVME WAITING\n");
    while(in->ctrl->icq_vaddr[old_iotail_i].cint3_raw == 0){

    }

    in->ctrl->icq_vaddr[old_iotail_i].cint3_raw = 0; //overwrite ent;


}

nvme_disk *nvme_find_disk_from_inode(unsigned long inode){

    nvme_ctrl *ctrl = nvme_ctrl_list;

    nvme_disk *ret = 0;

    while(ctrl){

        nvme_disk *disk = ctrl->disks;

        while(disk){

            if(disk->inode == inode){
                ret = disk;
                goto breakout;
            }
            disk=disk->next;
        }

        ctrl=ctrl->next;
    }
    breakout:

    return ret;
}

DISKMAN_WRITE_FUNC(nvme_write_disk){

    nvme_disk *disk = nvme_find_disk_from_inode(id);

    nvme_send_io_cmd(disk, off_sects, /*opcode*/1, num_sects, buf);

    //supposed to return write sects, not fucntional for now
    return num_sects;
}

DISKMAN_READ_FUNC(nvme_read_disk){

    nvme_disk *disk = nvme_find_disk_from_inode(id);
    draw_string("BUF ADDR=");
    draw_hex((unsigned long)buf);
    nvme_send_io_cmd(disk, off_sects, /*opcode*/2, num_sects, buf);


    //supposed to return read sects, not fucntional for now
    return num_sects;
}

void nvme_setup_pci_dev(pci_dev_ent *in){


        //enable pci bus mastering
    unsigned int pcicmdreg = pci_read_coni(in->bus, in->dev, in->func, 0x4); //get status + cmd
    pcicmdreg |= 1<<2;
    pci_write_coni(in->bus, in->dev, in->func, 0x4, pcicmdreg);

    unsigned long bar0_p = pci_read_coni(in->bus, in->dev, in->func, 0x10) | (((unsigned long)pci_read_coni(in->bus, in->dev, in->func, 0x14)) << 32); //assume 64bit bar, nvme is modern

    bar0_p &= 0xFFFFFFFFFFFFFFF0;

    volatile nvme_bar0 *bar0 = page_map_paddr(bar0_p, 1);


    nvme_ctrl *curr = nvme_new_ctrl();

    curr -> bar = bar0;
    curr -> pci_dev = in;
    curr -> acq_vaddr = (nvme_cmpl_queue_ent *)k_pageobj_alloc(&page_heap, 4096);
    curr -> asq_vaddr = (nvme_sub_queue_ent*)k_pageobj_alloc(&page_heap, 4096);
//    curr -> acq_vaddr = (nvme_cmpl_queue_ent *) page_find_and_alloc(1);
 //   curr -> asq_vaddr = (nvme_sub_queue_ent *) ((unsigned long)curr->acq_vaddr + 0x100000);


    unsigned int bar0_anded = (bar0->ctrl_conf & 0xfffffffe); //nvme disable
    bar0->ctrl_conf = bar0_anded;

    bar0->queue_att = 0x003f003f; //64 ents for both queues
    bar0->sub_queue_addr = (nvme_sub_queue_ent *)page_lookup_paddr((void*)curr->asq_vaddr); //pointer to an array of sz 64
    bar0->cmpl_queue_addr = (nvme_cmpl_queue_ent*) page_lookup_paddr((void*)curr->acq_vaddr);

    bar0->int_disable = 0xffffffff;
    bar0->ctrl_conf = 0x460001;

    //FIXME: removing this line causes the driver to hang in real hw. i dont focking know why.
    draw_hex((unsigned long)bar0->sub_queue_addr);
 


    //wiat for csts.rdy
    while(!(bar0->ctrl_stat & 1)){
        if(bar0->ctrl_stat & 0b10){
            draw_string("ERROR: NVME CSTS.CFS SET\n");
            return;
        }
    }
    draw_hex((unsigned long)pci_read_conw(in->bus,in->dev,in->func,0x3c));

    draw_string("NVME CTRL REENABLED\n");


    //io cmpl queue create;
    nvme_sub_queue_ent cmd = {0};
    cmd.cint0.opcode = 5;
    cmd.cint0.cid = 1;

    cmd.prp1 = (unsigned long) page_lookup_paddr((void*)(curr->icq_vaddr = k_pageobj_alloc(&page_heap, 4096)));
    cmd.cint11 = 0x00000001; //pc enabled
    cmd.cint10 = 0x003f0001; //queue id 1, 64 ents//
    cmd.nsid = 0;
    nvme_send_admin_cmd(curr, &cmd);

    //io sub queue create
    mem_set(&cmd, 0, sizeof(nvme_sub_queue_ent));;

    cmd.cint0.opcode = 0x1;
    cmd.cint0.cid = 0x1;

    cmd.prp1 = (unsigned long) page_lookup_paddr( (void*)(curr->isq_vaddr = k_pageobj_alloc(&page_heap, 4096)));
    cmd.cint10 = 0x003f0001; //64 ents sz and queue id 1
    cmd.cint11 = 0x00010001; //cmpl queue id 1 continous 1
    cmd.nsid = 0;
    nvme_send_admin_cmd(curr, &cmd);

    //get ctrller info
    mem_set(&cmd, 0, sizeof(nvme_sub_queue_ent));;

    cmd.cint0.opcode = 0x6;
    cmd.cint0.cid = 0x0;

    cmd.prp1 = (unsigned long) page_lookup_paddr((void*) (curr->ctrl_info = k_pageobj_alloc(&page_heap, 4096)));
    cmd.cint10 = 0x00000001; //id ctrl number
    cmd.cint11 = 0; //no cint11
    cmd.nsid = 0;
    nvme_send_admin_cmd(curr, &cmd);

    draw_string("==START NVME CTRL_INFO PRNT==\n");

    draw_string("MODEL=");
    draw_string_w_sz(curr->ctrl_info->model, 40);
    draw_string("\n");

    draw_string("SERIAL=");
    draw_string_w_sz(curr->ctrl_info->serial, 20);
    draw_string("\n");

    draw_string("==END NVME CTRL_INFO PRNT==\n");

    //get active nsids
    mem_set(&cmd, 0, sizeof(nvme_sub_queue_ent));;

    cmd.cint0.opcode = 0x6;
    cmd.cint0.cid = 0x0;

    cmd.prp1 = (unsigned long) page_lookup_paddr((void*) (curr->ns_list = k_pageobj_alloc(&page_heap, 4096)));
    cmd.cint10 = 0x00000002; //ns list number
    cmd.cint11 = 0; //no cint11
    cmd.nsid = 0;
    nvme_send_admin_cmd(curr, &cmd);

    draw_string("==START NAMESPACE IDS PRNT==\n");
    int i=0;
    while(curr->ns_list[i] != 0){
        draw_hex(curr->ns_list[i]);

        nvme_disk *curr_disk = nvme_new_disk(&curr->disks);

        //get active nsids
        mem_set(&cmd, 0, sizeof(nvme_sub_queue_ent));;

        cmd.cint0.opcode = 0x6;
        cmd.cint0.cid = 0x0;

        cmd.prp1 = (unsigned long)page_lookup_paddr((void*) (curr_disk->info = k_pageobj_alloc(&page_heap, 4096)));
        cmd.cint10 = 0x00000000; //disk id number
        cmd.cint11 = 0; //no cint11
        cmd.nsid = curr->ns_list[i]; //disk id specify
        nvme_send_admin_cmd(curr, &cmd);

        curr_disk->ctrl = curr;
        curr_disk->id = curr->ns_list[i];


        //register disk in diskman
        diskman_ent *ent = diskman_new_ent();
        curr_disk -> inode = ent->inode;
        ent->read_func = nvme_read_disk;
        ent->write_func = nvme_write_disk;


        draw_string("DISK SZ_IN_SECT=");
        draw_hex(curr_disk->info->lba_format_sz & 0x7);

        draw_string("DISK LBA_SECT=");
        curr_disk->sector_sz_in_bytes = 1 << (curr_disk->info->lba_format_supports[curr_disk->info->lba_format_sz & 0x7].lba_data_sz);
        draw_hex(curr_disk->sector_sz_in_bytes);

        ++i;
    }
    draw_string("==END NAMESPACE IDS PRNT==\n");


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

    //opcodes: 0x1 = write, 0x2 = read
    draw_string("NVME DISK CONTENT DUMP TEST:\n");

    //opcodes: 0x1 = write, 0x2 = read
    unsigned long *buff = k_pageobj_alloc(&page_heap, 4096);
    nvme_send_io_cmd(curr->disks, 1, 2, 1, buff);

    draw_string_w_sz((char*)buff, 512);
    draw_string("\n");





}

void nvme_setup(){

    pci_dev_ent* it = 0;

    while((it = pci_get_next_dev_ent(it)) != 0){
        if(it->cl == 1 && it->subcl == 8){
            nvme_setup_pci_dev(it);
        }
    }
}
