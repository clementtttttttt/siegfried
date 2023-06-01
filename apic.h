#ifndef APIC_H
#define APIC_H

typedef struct apic_madt_ent_head{

    unsigned char type;
    unsigned char sz;

}__attribute__((packed))apic_madt_ent_head;

typedef struct apic_cpu_ent{
    apic_madt_ent_head head;
    unsigned char cpu_id;
    unsigned char apic_id;
    unsigned int flags;
}__attribute__((packed))apic_cpu_ent;

typedef struct apic_ioapic_ent{
    apic_madt_ent_head head;
    unsigned char apic_id;
    unsigned char rsvd;
    unsigned int apic_addr;
    unsigned int int_base;
}__attribute__((packed))apic_ioapic_ent;

typedef struct apic_override_ent{
    apic_madt_ent_head head;
    unsigned char bus_src;
    unsigned char irq_src;
    unsigned int int_base;
    unsigned short flags;
}__attribute__((packed)) apic_override_ent;

typedef struct apic_nmi_src_ent{
    apic_madt_ent_head head;

    unsigned char nmi_src;
    unsigned char rsvd;
    unsigned short flags;
    unsigned int int_base;
}__attribute__((packed)) apic_nmi_src_ent;

typedef struct apic_loc_nmi_ent{
    apic_madt_ent_head head;

    unsigned char acpi_cpu_id;
    unsigned short flags;
    unsigned char lint;
}__attribute__((packed))apic_loc_nmi_ent;

typedef struct apic_loc_override_ent{
    apic_madt_ent_head head;

    unsigned short rsvd;
    unsigned long apic_addr;
}__attribute__((packed))apic_loc_override_ent;

typedef struct apic_loc_x2apic_ent{
    apic_madt_ent_head head;

    unsigned short rsvd;
    unsigned int cpu_id;
    unsigned int flags;
    unsigned int apic_id;
}__attribute__((packed))apic_loc_x2apic_ent;

typedef union apic_redir_ent{

    struct {
    unsigned long int_num : 8;
    unsigned long deliv_mode : 3;
    unsigned long dest_mode : 1;
    unsigned long deliv_status : 1;
    unsigned long polarity : 1;
    unsigned long remote_irr : 1;
    unsigned long trigger_mode : 1;
    unsigned long mask : 1;

    unsigned long rsvd : 39;

    unsigned long dest : 8;
    };

    struct{
        unsigned int lower_raw;
        unsigned int upper_raw;
    };

    unsigned long raw;

}__attribute__((packed))apic_redir_ent;

typedef struct apic_cpu_tab{
    unsigned char cpu_id;
    unsigned char apic_id;
    unsigned int flags;
}apic_cpu_tab;

void apic_setup();

#endif
