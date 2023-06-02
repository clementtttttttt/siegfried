#include "pci.h"

typedef struct nvme_cap_reg{

    unsigned long max_queue_ents : 16;

    unsigned long requires_cont : 1;
    unsigned long arb_t_supported: 2;
    unsigned long rsvd : 5;

    unsigned long timeout : 8;
    unsigned long db_stride : 4;
    unsigned long can_sub_reset : 1;
    unsigned long cmdset_t_supported : 8;
    unsigned long rsvd3 : 3;
    unsigned long min_page_sz : 4;
    unsigned long max_page_sz : 4;
    unsigned long rsvd8 : 8;


}__attribute__((packed))nvme_cap_reg;

typedef struct cint0_t{
        unsigned char opcode;

        unsigned char fuse_op : 2;
        unsigned char rsvd : 5;
        unsigned char use_sgl : 1;

        unsigned short cid;
}cint0_t;

typedef struct nvme_sub_queue_ent{

    cint0_t cint0;

    unsigned int nsid;

    unsigned long rsvd_l;

    unsigned long metadata_addr;
    unsigned long prp1;
    unsigned long prp2;

    unsigned int cint10, cint11, cint12, cint13, cint14, cint15;


}__attribute__((packed)) nvme_sub_queue_ent;

typedef struct nvme_cmpl_queue_ent{
    unsigned int cint0;
    unsigned int rsvd;

    union{
        struct{
            unsigned short sub_queue_idx;
            unsigned short sub_queue_id;
        };
        unsigned int cint2_raw;
    };

    union{
        struct{
            unsigned short cmd_id;
            unsigned short is_new : 1;
            unsigned short stat : 15;
        };
        unsigned int cint3_raw;
    };
}__attribute__((packed)) nvme_cmpl_queue_ent;

typedef struct nvme_bar0{

    nvme_cap_reg cap;
    unsigned int ver;
    unsigned int int_disable;
    unsigned int int_enable;
    unsigned int ctrl_conf;

    unsigned int rsvd;

    unsigned int ctrl_stat;
    unsigned int nvm_reset;
    unsigned int queue_att;
    nvme_sub_queue_ent * sub_queue_addr; //ZERO OUT BIT 0, queue is hard coded to 64
    nvme_cmpl_queue_ent *cmpl_queue_addr ; //ZERO OUT BIT 0

    char we_dont_give_a_shit [0xFC8];

    unsigned int sub_queue_tail_doorbell;


}__attribute__((packed)) nvme_bar0;

typedef struct nvme_ctrl_info{

    unsigned short vendor_id;
    unsigned short sub_vendor_id;

    char serial[20];
    char model[40];


}__attribute__((packed)) nvme_ctrl_info;

typedef struct nvme_ctrl{
    struct nvme_ctrl *next;
    volatile nvme_bar0 *bar;
    pci_dev_ent *pci_dev;

    volatile nvme_sub_queue_ent *asq_vaddr;
    volatile nvme_cmpl_queue_ent *acq_vaddr;
    volatile nvme_sub_queue_ent *isq_vaddr;
    volatile nvme_cmpl_queue_ent *icq_vaddr;
    unsigned char a_tail_i, io_tail_i;

    nvme_ctrl_info *ctrl_info; //pointer to  4096b data struct

}nvme_ctrl;



void nvme_setup();
void draw_scroll_text_buf();
