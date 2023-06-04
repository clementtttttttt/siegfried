#include "diskman.h"
#include "draw.h"
#include "klib.h"

void diskman_gpt_enum(diskman_ent *in){

    draw_string("FOUND GPT PART\n");

    char buf[512];
    mem_set(buf, 0, 512);

    in->read_func(in->inode, 1, 1, buf );

    gpt_header *head = (gpt_header*) buf;

    draw_string(head->magic);
}
