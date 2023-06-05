#include "diskman.h"
#include "draw.h"
#include "klib.h"
#include "obj_heap.h"

char buf[512];

void diskman_gpt_enum(diskman_ent *in){

    mem_set(buf, 0, 512);

    in->read_func(in->inode, 1, 1, buf );

    gpt_header *head = k_obj_alloc(512);

    mem_cpy(head, buf, 512);

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

    for(unsigned long i=0; i < head->num_parts; ++i){
        in->read_func(in->inode, head->lba_part_ent+i, 1, buf);
    }
}
