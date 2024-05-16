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
    mem_set((void*)&c->acq_vaddr[c->a_tail_i], 0, sizeof(nvme_cmpl_queue_ent));


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

    nvme_sub_queue_ent cmd;
    
    mem_set(&cmd, 0, sizeof(nvme_sub_queue_ent));;

    cmd.cint0.cid = ++io_cmdid_c;
    cmd.cint0.opcode = opcode;
    cmd.nsid = in->id;
    cmd.prp1 = (unsigned long)buf;

    void* prp2_vm = k_pageobj_alloc(&page_heap, 4096);

    void* prp2 = page_lookup_paddr(prp2_vm);

    if(num_sects >= (4096 / 512)){
        cmd.prp2 = (unsigned long)prp2;
    }
    else cmd.prp2 = 0;

    cmd.cint10 = off_sects & 0xffffffff;
    cmd.cint11 = off_sects >> 32;

    cmd.cint12 = (num_sects & 0xffffffff) - 1;


    mem_cpy((void*)(in->ctrl->isq_vaddr + in->ctrl->io_tail_i), &cmd, sizeof(nvme_sub_queue_ent));
    unsigned short old_iotail_i = in->ctrl->io_tail_i;

    if((++in->ctrl->io_tail_i) == 0x40){

        in->ctrl->io_tail_i = 0;
    	in->ctrl->phase = !in->ctrl->phase;
    }

    in->ctrl->bar->io_sub_queue_tail_doorbell = in->ctrl->io_tail_i;
    //mmio_pokel(&in->ctrl->bar->io_sub_queue_tail_doorbell, in->ctrl->io_tail_i);
	
    while(in->ctrl->icq_vaddr[old_iotail_i].phase == in->ctrl->phase){
	
    }

     in->ctrl->bar->io_cmpl_queue_tail_doorbell = old_iotail_i;

    in->ctrl->icq_vaddr[old_iotail_i].cmd_id = 0; //overwrite ent;
    in->ctrl->icq_vaddr[old_iotail_i].stat = 0; //overwrite ent;

  //  in->ctrl->icq_vaddr[old_iotail_i].cint3_raw = in->ctrl->phase;

    //FIXME!!!!!: proper handling of ios with more than 4 sects or smth

    if(num_sects >= (4096 / 512)){
       // mem_cpy((void*)((unsigned long)buf), buf_io, 4096 );
        //mem_cpy((void*)((unsigned long)buf + 4096), prp2_vm, num_sects * 512 - 4096);
    }else{
//           mem_cpy((void*)((unsigned long)buf), read_buf, num_sects*512 );

    }

    
    k_pageobj_free(&page_heap, prp2_vm);

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

    nvme_send_io_cmd(disk, off_bytes, /*opcode*/1, num_bytes/512, buf);

    //supposed to return write sects, not fucntional for now
    return num_bytes;
}

DISKMAN_READ_FUNC(nvme_read_disk){
    nvme_disk *disk = nvme_find_disk_from_inode(id);
	

    if(disk == 0) return 0;
   
    
    unsigned long buf_sz = num_bytes  + (off_bytes%512?512:0);
    
    buf_sz += 512- (buf_sz %512); //round up

 	void *rdbuf = k_pageobj_alloc(&page_heap,buf_sz);

    nvme_send_io_cmd(disk, off_bytes/512, /*opcode*/2, buf_sz / 512, page_lookup_paddr_tab(page_get_krnl_tab(), rdbuf));

    mem_cpy(buf,(void*) ((unsigned long)rdbuf+off_bytes%512), num_bytes);
	
    k_pageobj_free(&page_heap,rdbuf);

    //supposed to return read sects, not fucntional for now
    return num_bytes;
}



void nvme_setup_pci_dev(pci_dev_ent *in){


        //enable pci bus mastering
    unsigned int pcicmdreg = pci_read_coni(in->bus, in->dev, in->func, 0x4); //get status + cmd
    pcicmdreg |= 1<<2;
    pci_write_coni(in->bus, in->dev, in->func, 0x4, pcicmdreg);

    unsigned long bar0_p = pci_read_bar(in->bus, in->dev, in->func, 0x10);
    draw_string("IN PTR=");
    draw_hex((unsigned long)in);
    draw_string("NVME BDF=");
    draw_hex(in->bus);
    draw_hex(in->dev);
    draw_hex(in->func);
    draw_string("END NVME BDF\n");

    volatile nvme_bar0 *bar0 = page_map_paddr_mmio(bar0_p, 1);
	
	draw_string("NVME bar sz = ");
	draw_hex(pci_read_bar_size(in->bus, in->dev, in->func, 0x10));

    nvme_ctrl *curr = nvme_new_ctrl();

    curr -> bar = bar0;
    curr -> pci_dev = in;
    curr -> acq_vaddr = (nvme_cmpl_queue_ent *)k_pageobj_alloc(&page_heap, 4096);
    curr -> asq_vaddr = (nvme_sub_queue_ent*)k_pageobj_alloc(&page_heap, 4096);  

	//zero out entries just for good measure
	mem_set((void*)curr->acq_vaddr, 0, 4096);
	mem_set((void*)curr->asq_vaddr, 0, 4096);

    //    curr -> acq_vaddr = (nvme_cmpl_queue_ent *) page_find_and_alloc(1);
 //   curr -> asq_vaddr = (nvme_sub_queue_ent *) ((unsigned long)curr->acq_vaddr + 0x100000);


    bar0->ctrl_conf_raw &= ~NVME_CTRL_ENABLE;

    
    while((bar0->ctrl_stat & 1)){
        if(bar0->ctrl_stat & 0b10){
            draw_string("ERROR: NVME CSTS.CFS SET\n");
            return;
        }
    }
    
    bar0->queue_att = 0x003f003f; //64 ents for both queues
    bar0->sub_queue_addr = (nvme_sub_queue_ent *)page_lookup_paddr((void*)curr->asq_vaddr); //pointer to an array of sz 64
    bar0->cmpl_queue_addr = (nvme_cmpl_queue_ent*) page_lookup_paddr((void*)curr->acq_vaddr);

    bar0->int_disable = 0xffffffff;
    bar0->ctrl_conf_raw = 0x460001;

 
    draw_string("PGSZ=");
    draw_hex(1 << (12+(NVME_CTRL_PGSZ(bar0->ctrl_conf_raw))));
    
    draw_string("VER=");
    draw_hex(bar0->version);
	
    //wiat for csts.rdy
    while(!(bar0->ctrl_stat & 1)){
        if(bar0->ctrl_stat & 0b10){
            draw_string("ERROR: NVME CSTS.CFS SET\n");
            return;
        }
    }
    draw_hex((unsigned long)pci_read_conw(in->bus,in->dev,in->func,0x3c));

    draw_string("NVME CTRL REENABLED\n");

    //delete subm queue
    nvme_sub_queue_ent cmd;
    mem_set(&cmd, 0, sizeof(nvme_sub_queue_ent));
   
    cmd.cint0.opcode = 0;
    cmd.cint0.cid = 1;
    cmd.cint10 = 1;
    nvme_send_admin_cmd(curr, &cmd);

    //delete cmpl queue
    mem_set(&cmd, 0, sizeof(nvme_sub_queue_ent));

    cmd.cint0.opcode = 0x4;
    cmd.cint0.cid = 1;
    cmd.cint10 = 1;
    nvme_send_admin_cmd(curr, &cmd);

    
    //io cmpl queue create
    mem_set(&cmd, 0, sizeof(nvme_sub_queue_ent));

    cmd.cint0.opcode = 5;
    cmd.cint0.cid = 1;

    cmd.prp1 = (unsigned long) page_lookup_paddr((void*)(curr->icq_vaddr = k_pageobj_alloc(&page_heap, 4096)));
    cmd.cint11 = 0x00000001; //pc enabled
    cmd.cint10 = 0x003f0001; //queue id 1, 64 ents//
    cmd.nsid = 0;
    nvme_send_admin_cmd(curr, &cmd);

	draw_string("CREATING IO SUBQ\n");

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
    cmd.cint0.cid = 0x1;

    cmd.prp1 = (unsigned long) page_lookup_paddr((void*) (curr->ctrl_info = k_pageobj_alloc(&page_heap, 4096)));
    cmd.cint10 = 0x00000001; //id ctrl number
    cmd.cint11 = 0; //no cint11
    cmd.cint14 = 0; //no cint14
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
    cmd.cint0.cid = 0x1;

    cmd.prp1 = (unsigned long) page_lookup_paddr((void*) (curr->ns_list = k_pageobj_alloc(&page_heap, 4096)));
    cmd.cint10 = 0x00000002; //ns list number
    cmd.cint11 = 0; //no cint11
    cmd.nsid = 0;
    nvme_send_admin_cmd(curr, &cmd);

    draw_string("==START NAMESPACE IDS PRNT==\n");
    int i=0;
    while(curr->ns_list[i] != 0){
        nvme_disk *curr_disk = nvme_new_disk(&curr->disks);

        //get active nsids
        mem_set(&cmd, 0, sizeof(nvme_sub_queue_ent));;

        cmd.cint0.opcode = 0x6;
        cmd.cint0.cid = 0x1;

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
        ent->uuid = 0;
        ent->uuid_len = 0;

        draw_string("DISK SZ_IN_SECT=");
        draw_hex(curr_disk->info->lba_format_sz & 0x7);

        draw_string("DISK LBA_SECT=");
        curr_disk->sector_sz_in_bytes = 1 << (curr_disk->info->lba_format_supports[curr_disk->info->lba_format_sz & 0x7].lba_data_sz);
        draw_hex(curr_disk->sector_sz_in_bytes);

        draw_string("DISK INODE=");
        draw_hex(curr_disk->inode);

        ++i;
    }
    draw_string("==END NAMESPACE IDS PRNT==\n");


    //get msi cap
    unsigned char cap_off = pci_read_conw(in->bus, in->dev, in->func, 0x34) & 0xfc;

    unsigned char cap_id = (pci_read_conw(in->bus, in->dev, in->func, cap_off));

    while(cap_id != 0x11 && cap_id != 0){
        draw_string("==START CAP_ID PRNT==\n");
        draw_hex(cap_off);
        draw_hex(cap_id);
        draw_string("==END CAP_ID PRNT==\n");

        cap_off = (pci_read_conw(in->bus, in->dev, in->func, cap_off) >> 8);
        cap_id = (pci_read_conw(in->bus, in->dev, in->func, cap_off));

    }
        draw_string("==START CAP_ID PRNT==\n");
        draw_hex(cap_off);
        draw_hex(cap_id);
        draw_string("==END CAP_ID PRNT==\n");
    if(cap_id == 0){
        draw_string("NVME MSI CAP NOT FOUND!\n");
        return;
    }

    
    

	//zero out entries just for good measure
	mem_set((void*)curr->acq_vaddr, 0, 4096);
	mem_set((void*)curr->asq_vaddr, 0, 4096);


}

void nvme_setup(){

    pci_dev_ent* it = 0;

      while((it = pci_get_next_dev_ent(it)) != 0){
  /*      draw_string("==PCI DEVICE DETECTION==");
        draw_string("VENDOR+DEV:");
        draw_hex(it->vendor);
        draw_hex(it->devid);
        draw_string("CLASS+SUBCLASS:");
        draw_hex(it->cl);
        draw_hex(it->subcl);*/
	if(it->cl == 1 && it->subcl == 8){
            nvme_setup_pci_dev(it);
        }
    }
}
