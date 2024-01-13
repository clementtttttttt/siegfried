#include "diskman.h"
#include "draw.h"
#include "klib.h"
#include "obj_heap.h"

char buf[512];
gpt_partlist_ent *gparts = 0;

gpt_partlist_ent *diskman_gpt_new_ent(){

    gpt_partlist_ent *ret;

    if(gparts == 0){
        ret = gparts = k_obj_alloc(sizeof(gpt_partlist_ent));
    }
    else{
        ret = gparts;
        while(ret->next) ret=ret->next;

        ret->next = k_obj_alloc(sizeof(gpt_partlist_ent));

        ret = ret->next;
    }
	
	ret->next = 0;

    return ret;
}

gpt_partlist_ent *gpt_find_ent(unsigned long inode){


    gpt_partlist_ent *e = gparts;

    while(e){

        if(e -> inode == inode){
            return e;
        }

        e = e ->next;
    }

    return (void*)0;

}

unsigned long diskman_gpt_read(unsigned long inode, unsigned long off, unsigned long num, void *buf){

    gpt_partlist_ent *e = gpt_find_ent(inode);

    e->disk->read_func(e->disk->inode, e->lba_start*512 + off, num, buf);


    return off;
}

unsigned long diskman_gpt_write(unsigned long inode, unsigned long off, unsigned long num, void *buf){

    gpt_partlist_ent *e = gpt_find_ent(inode);

    e->disk->write_func(e->disk->inode, e->lba_start*512 + off, num, buf);


    return off;
}

void diskman_gpt_enum(diskman_ent *in){
	
    gpt_header *head = k_obj_alloc(sizeof(gpt_header));

    mem_set(head, 0, sizeof(gpt_header));

    in->read_func(in->inode, 512, sizeof(gpt_header), head );
asm("sti");

    draw_string("FOUND GPT TAB: UUID=");
    draw_string_w_sz(head->guid, 16);
    draw_string("\n");

    in->uuid = k_obj_alloc(17);
    mem_cpy(in->uuid, head->guid, 16);
    in->uuid_len = 16;

    draw_string("GPT PARTENT LBA=");
    draw_hex(head->lba_part_ent);
    draw_string("GPT NUMPART=");
    draw_hex(head->num_parts);

    draw_string("PARTENT SZ=");
    draw_hex(head->parts_ent_sz);

	    char *esect = k_obj_alloc(head->parts_ent_sz);


    for(unsigned long i=0; i < head->num_parts; ++i){

      //z  //draw_hex(head->lba_part_ent*512+((i*head->parts_ent_sz)));
        in->read_func(in->inode, head->lba_part_ent*512 + (i * head->parts_ent_sz), head->parts_ent_sz, esect);


		        
 
        gpt_partent *ent = (gpt_partent*)&esect[0];

        __int128 cmp_z = 0;
        if(!mem_cmp(ent->type_guid, (char*)&cmp_z, 16)){
            continue;
        }

        diskman_ent *dev = diskman_new_ent();

        dev->uuid = k_obj_alloc(16);
        mem_cpy(dev->uuid, &ent->part_guid, 16);

        dev->uuid_len = 16;
        dev->ispart = 1;

        dev->read_func  = (void*) diskman_gpt_read;
        dev->write_func = (void*) diskman_gpt_write;

		    
        gpt_partlist_ent *e = diskman_gpt_new_ent();


        e->inode = dev->inode;
        e->lba_start = ent->lba_start;
        e->lba_end = ent->lba_end;
        e->attr = ent->attr;
        e->disk = in;


		    
        draw_string("\nFOUND PART: PART_UUID=");
        draw_string_w_sz((char*)&ent->part_guid, 16);
        draw_string("\n");
        draw_string("PARTNAME=");
        draw_string_w_sz(ent->partname, 72);
        draw_string("\n");
        draw_string("PART INODE=");
        draw_hex(e->inode);
        draw_string("PART OFF=");
        draw_hex(e->lba_start);



    }

    k_obj_free(head);
    k_obj_free(esect);
    asm("cli");
}
