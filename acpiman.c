#include "acpiman.h"
#include "klib.h"
#include "obj_heap.h"
#include "debug.h"
#include "draw.h"
#include "page.h"
#include "io.h"

acpi_rsdp_desc sys_rsdp_desc;

acpi_xsdt *sys_xsdt_addr;

int acpiman_isvalid(void* stuff, unsigned long size){
    unsigned char* bytes = stuff;
    unsigned long sum = 0;
    for(unsigned long i=0;i<size;++i){
        sum += bytes[i];
    }
    return !((unsigned char)sum);
}

void acpi_gas_poke(acpi_gas *addr, unsigned long in){
	draw_hex(addr->address_space);
	draw_hex(addr->access_size);
	draw_hex(addr->addr);
	draw_hex(sizeof(acpi_gas));
	switch((acpi_gas_addrspace_t)addr->address_space){
		case GAS_IO:
		{
			switch((acpi_gas_sz_t)addr->access_size){
				case SZ_CHAR:
					io_outb(addr->addr, in);
				break;
				case SZ_INT:
					io_outl(addr->addr, in);
				break;
				case SZ_NULL:
					io_outb(addr->addr, in);
				break;
				default:
				break;
			}
		}
		break;
		case GAS_MEM:
		{
			void *a = page_map_paddr_mmio(addr->addr, 1);
			switch((acpi_gas_sz_t)addr->access_size){
				case SZ_CHAR:
					*(unsigned char*)a = in;
				break;
				case SZ_SHORT:
					*(unsigned short*)a = in;
				break;
				case SZ_INT:
					*(unsigned int*)a = in;
				break;
				default:
				break;
			}
		}
		default:
		break;
	}
}

acpi_sdt_header *acpiman_get_tab(char* magic){
    unsigned long ptrs = (sys_xsdt_addr->sz - sizeof(acpi_sdt_header))/8;

    for(unsigned long i=0;i<ptrs;++i){
        if(!mem_cmp(sys_xsdt_addr->tabs_ptr[i]->magic, magic,4)){
			if(!acpiman_isvalid(sys_xsdt_addr->tabs_ptr[i], sys_xsdt_addr->tabs_ptr[i]->sz)){
					draw_string("INVALID ACPI TABLE");
					while(1){}
			}
            return sys_xsdt_addr->tabs_ptr[i];
        }
    }
    return (acpi_sdt_header*) 0;
}

static acpi_gas reset_addr;
static unsigned long reset_value;

int acpiman_try_reboot(){
	
	
	

	acpi_gas_poke(& reset_addr, reset_value);
	//reset failed if we reach this point
	while(1){}
	return -1;
}
void acpiman_setup(void* rsdp){
    mem_cpy(&sys_rsdp_desc, rsdp, sizeof(acpi_rsdp_desc));



    if(mem_cmp(sys_rsdp_desc.first.magic, "RSD PTR ", 8)){
        draw_string("BAD RSDP MAGIC: ");
        sys_rsdp_desc.first.magic[7] = 0;
        draw_string(sys_rsdp_desc.first.magic);
        draw_string("\n");

        // break away
        return;

    }



    if(!acpiman_isvalid(&sys_rsdp_desc.first, sizeof(acpi_rsdp_desc_leg))){
        draw_string("DODGY LEGACY RSDP: CHECKSUM NOT 0\n");
        return;
    }

    if(!acpiman_isvalid(&sys_rsdp_desc, sizeof(acpi_rsdp_desc))){
        draw_string("DODGY RSDP: CHECKSUM NOT 0\n");
        return;
    }

    draw_string("RSDP OEM: ");
    draw_string_w_sz(sys_rsdp_desc.first.vendor_str,6);
    draw_string("\n");

    sys_xsdt_addr = page_map_paddr(sys_rsdp_desc.xsdt_addr, 2);

    draw_string("XSDT PADDR: ");
    draw_hex(sys_rsdp_desc.xsdt_addr);
    draw_string("XSDT VADDR: ");
    draw_hex((unsigned long)sys_xsdt_addr);
    draw_string("XSDT SZ: ");
    draw_hex((unsigned long)sys_xsdt_addr->sz);


    //check xsdt
    if(!acpiman_isvalid(sys_xsdt_addr, sys_xsdt_addr->sz)){
        draw_string("DODGY XSDT: CHECKSUM NOT 0");
        return;
    }

    if(mem_cmp(sys_xsdt_addr->magic, "XSDT", 4)){
        draw_string("DODGY XSDT: BAD MAGIC");
        return;
    }

    unsigned long ptrs = (sys_xsdt_addr->sz - sizeof(acpi_sdt_header))/8;


    for(unsigned int i=0;i<ptrs;++i){
        acpi_sdt_header *h = sys_xsdt_addr->tabs_ptr[i];
     //   draw_string("FOUND PTR AT ");
     //   draw_hex((unsigned long)h);

        if((((unsigned long)h) >> 21) != (((unsigned long)sys_rsdp_desc.xsdt_addr) >> 21) ) {
				h = page_map_paddr((unsigned long) h, 1);
		}
		else {
				h = (acpi_sdt_header*)( ((unsigned long)sys_xsdt_addr & (~0x1fffff)) + ((unsigned long)h & 0x1fffff));
		}
        sys_xsdt_addr->tabs_ptr[i] = h;

       // draw_string("MAGIC IS ");
        char magic[5];
        mem_cpy(magic, h->magic, 4);
        //draw_string(magic);
    }

	acpi_fadt *fadt = (acpi_fadt*)acpiman_get_tab("FACP");
	reset_addr = fadt->reset_reg;
	reset_value = fadt->reset_value;


}
