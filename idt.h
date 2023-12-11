
typedef union idt_type_attr{
    struct{
        unsigned char type : 4;
        unsigned char zero : 1;
        unsigned char dpl : 2;
        unsigned char present : 1;
    };

    unsigned char raw;
}idt_type_attr;

typedef struct idt_desc{
    unsigned short offset_l16;
    unsigned short code_seg;
    unsigned char ist;
    idt_type_attr type_attr;
    unsigned short offset_h16;
    unsigned int offset_hh32;
    unsigned int zero;
}idt_desc;


typedef struct idt_tab_desc{

    unsigned short sz;
    idt_desc *addr;

}__attribute__((packed))idt_tab_desc ;

void idt_setup();
void idt_set_irq_ent(unsigned long no, void* addr);
void idt_flush();
void idt_print_stacktrace(unsigned long *stack);
