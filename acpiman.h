#include "apic.h"

typedef struct acpi_rsdp_desc_leg{
    char magic[8];
    unsigned char chksum;
    char vendor_str[6];
    unsigned char rev;
    unsigned int rsdt_addr;
}__attribute__((packed)) acpi_rsdp_desc_leg ;

typedef struct acpi_rsdp_desc{
    acpi_rsdp_desc_leg first;
    unsigned int sz;
    unsigned long xsdt_addr;
    unsigned char chksum_ext;
    unsigned char rsvd[3];

}__attribute__((packed)) acpi_rsdp_desc ;

typedef struct acpi_sdt_header{
    char magic[4];
    unsigned int sz;
    unsigned char rev;
    unsigned char chksum;
    char vendor_str[6];
    char vendor_tab_str[8];
    unsigned int vendor_rev;
    unsigned int author_id;
    unsigned int author_rev;
}__attribute__((packed))acpi_sdt_header;

typedef struct acpi_xsdt{
    char magic[4];
    unsigned int sz;
    unsigned char rev;
    unsigned char chksum;
    char vendor_str[6];
    char vendor_tab_str[8];
    unsigned int vendor_rev;
    unsigned int author_id;
    unsigned int author_rev;

    acpi_sdt_header *tabs_ptr[0];

}__attribute__((packed))acpi_xsdt;

typedef struct acpi_madt{
    acpi_sdt_header header;

    unsigned int apic_addr;
    unsigned int flags;

    apic_madt_ent_head ents_start;

}acpi_madt;

typedef struct acpi_fadt{
    acpi_sdt_header header;

    unsigned int firmtrl;
    unsigned int dsdt;

    unsigned char rsvd;

    unsigned char power_prof;

    unsigned short sci_int;
    unsigned int smi_cmd;
    unsigned char acpi_enable;

    unsigned char acpi_disable;

    unsigned  char s4bios_req;
    unsigned char pstat_ctlr;

    unsigned int pm1a_evblk;
    unsigned int pm1b_evblk;
    unsigned int pm1a_ctrlblk;
    unsigned int pm1b_ctrlblk;
    unsigned int pm2_ctrlblk;
    unsigned int pm_timerblk;
    unsigned int gpe0_blk;
    unsigned int gpe1_blk;


}acpi_fadt;

void acpiman_setup(void*);
acpi_sdt_header *acpiman_get_tab(char* magic);
