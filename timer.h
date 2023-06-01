void timer_setup();

void timer_wait();

typedef union lvt_tmr_reg{
    struct{
        unsigned int int_base : 8;
        unsigned int rsvd4 : 4;
        unsigned int int_pending : 1;
        unsigned int rsvd3_sec : 3;
        unsigned int is_disabled : 1;
        unsigned int mode : 3;
        unsigned int rsvd15 : 12;
    }__attribute__((packed));
    unsigned int raw;
}__attribute__((packed))lvt_tmr_reg;
