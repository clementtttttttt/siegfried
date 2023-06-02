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




}nvme_cap_reg;

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
    unsigned long sub_queue_addr;
    unsigned long cmpl_queue_addr;

}__attribute__((packed)) nvme_bar0;


void nvme_setup();
void draw_scroll_text_buf();
