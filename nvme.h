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
    nvme_sub_queue_ent * volatile sub_queue_addr; //ZERO OUT BIT 0, queue is hard coded to 64
					 //
    nvme_cmpl_queue_ent * volatile cmpl_queue_addr ; //ZERO OUT BIT 0

    char we_dont_give_a_shit [0xFC8];

    unsigned int sub_queue_tail_doorbell;
    unsigned int comp_queue_tail_doorbell;

    unsigned int io_sub_queue_tail_doorbell;
    unsigned int io_cmpl_queue_tail_doorbell;



}__attribute__((packed)) nvme_bar0;

typedef struct nvme_ctrl_info{

    unsigned short vendor_id;
    unsigned short sub_vendor_id;

    char serial[20];
    char model[40];

    //fixme: complete nvme ctrl info


}__attribute__((packed)) nvme_ctrl_info;

typedef struct lba_format{
    unsigned short meta_sz;
    unsigned short lba_data_sz:8;
    unsigned short rel_perf : 2;
    unsigned short rsvd : 6;
}__attribute__((packed))lba_format;

typedef struct nvme_disk_info{
    unsigned long sz_in_sects;
    unsigned long cap_in_sects;
    unsigned long used_in_sects;
    unsigned char features;
    unsigned char no_of_formats;
    unsigned char lba_format_sz;

    unsigned char meta_caps;
    unsigned char prot_caps;
    unsigned char prot_types;
    unsigned char nmic_caps;
    unsigned char res_caps;

    char rsvd[88];

    unsigned long euid;

    lba_format lba_format_supports[15];

    char rsvd2 [202];


}__attribute__((packed)) nvme_disk_info;

struct nvme_disk;

typedef struct nvme_ctrl{
    struct nvme_ctrl *next;
    volatile nvme_bar0 *bar;
    pci_dev_ent *pci_dev;

    volatile nvme_sub_queue_ent *asq_vaddr;
    volatile nvme_cmpl_queue_ent *acq_vaddr;
    volatile nvme_sub_queue_ent *isq_vaddr;
    volatile nvme_cmpl_queue_ent *icq_vaddr;
    unsigned char a_tail_i, io_tail_i;
    unsigned int *ns_list;

    nvme_ctrl_info *ctrl_info; //pointer to  4096b data struct

    struct nvme_disk *disks;
}nvme_ctrl;

typedef struct nvme_disk{
    struct nvme_disk *next;
    nvme_disk_info *info;
    unsigned int sector_sz_in_bytes;
    nvme_ctrl *ctrl;
    unsigned int id;
    unsigned long inode;

} nvme_disk;



void nvme_setup();
void draw_scroll_text_buf();
