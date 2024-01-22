#include "pci.h"
#include "debug.h"
#include "io.h"
#include "idt.h"
#include "obj_heap.h"
#include "draw.h"
#include "acpiman.h"
#include "page.h"
pci_dev_ent *pci_root = 0;

pci_bridge_ent * pci_bridge_root = 0;

pci_dev_ent *pci_get_next_dev_ent(pci_dev_ent *in){
    if(in == 0){
        in = pci_root;
    }
    else{
        in = in->next;
    }

    return in;

}

pci_dev_ent* pci_new_dev_ent(){
    if(pci_root == 0){
        pci_root = k_obj_alloc(sizeof(pci_dev_ent));
	pci_root->next = 0;
        return pci_root;

    }
    else{

        pci_dev_ent *i;

        //skip to last ent in linked list.
        for(i = pci_root; i->next != 0; i = i->next);

        i->next = k_obj_alloc(sizeof(pci_dev_ent));
	i->next->next = 0;
        return i->next;
    }
}

pci_bridge_ent* pci_find_bridge_from_ent(unsigned char bus){
		pci_bridge_ent * root = pci_bridge_root;
		
		while(root){
			if(root->ecam_addr == 0){
					return NULL;
			}
			if(bus >= root->start_bus  && bus < root->end_bus ){
				//found it
				return root;
			}
			
			root = root->next;
		}
		return NULL;
}

void pci_write_conw(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset, unsigned short in) {
  /*  unsigned int address;
    unsigned int lbus  = (unsigned int)bus;
    unsigned int lslot = (unsigned int)slot;
    unsigned int lfunc = (unsigned int)func;
    //unsigned short tmp = 0;

    // Create configuration address as per Figure 1
    address = (unsigned int)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xFC) | ((unsigned int)0x80000000));

    unsigned int old = pci_read_coni(bus, slot, func, offset & 0xfc) & (0xffff >> ((2-(offset & 2)) * 8));

    old |= in << ((offset&2)*8);
    // Write out the address
    io_outl(0xCF8, address);
    // Read in the data
    // (offset & 2) * 8) = 0 will choose the first word of the 32-bit register
    io_outl(0xcfc, old);
    *
    */ 
    
    
    pci_bridge_ent *e = pci_find_bridge_from_ent(bus);

    
    volatile unsigned short* addr = ((volatile unsigned short*) ((((unsigned long) offset) + ((unsigned long)e->ecam_addr)  +  (((bus - e->start_bus)) << 20)) | (slot << 15) | (func << 12)));
    *addr = in;
 
}

unsigned short pci_read_conw(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset) {
    /*
     * 
     * unsigned int address;
    unsigned int lbus  = (unsigned int)bus;
    unsigned int lslot = (unsigned int)slot;
    unsigned int lfunc = (unsigned int)func;
    unsigned short tmp = 0;

    // Create configuration address as per Figure 1
    address = (unsigned int)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xFC) | ((unsigned int)0x80000000));

    // Write out the address
    io_outl(0xCF8, address);
    // Read in the data
    // (offset & 2) * 8) = 0 will choose the first word of the 32-bit register
    tmp = (unsigned short)((io_inl(0xCFC) >> ((offset & 2) * 8)) & 0xFFFF);
    return tmp;
    *
    *
    * 
    */
    
    pci_bridge_ent *e = pci_find_bridge_from_ent(bus);

    
    volatile unsigned short* addr = (volatile unsigned short*) ((unsigned long) offset + (unsigned long)e->ecam_addr  +  ((((bus - e->start_bus)) << 20) | (slot << 15) | (func << 12)));
 
	return *addr;
}



unsigned int pci_read_coni(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset) {
  /*  unsigned int address;
    unsigned int lbus  = (unsigned int)bus;
    unsigned int lslot = (unsigned int)slot;
    unsigned int lfunc = (unsigned int)func;

    // Create configuration address as per Figure 1
    address = (unsigned int)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xFC) | ((unsigned int)0x80000000));

    // Write out the address
    io_outl(0xCF8, address);
    // Read in the data
    // (offset & 2) * 8) = 0 will choose the first word of the 32-bit register
    return io_inl(0xcfc);*/
        pci_bridge_ent *e = pci_find_bridge_from_ent(bus);

    
    volatile unsigned int* addr = (volatile unsigned int*) (((unsigned long) offset + (unsigned long)e->ecam_addr  +  (((bus - e->start_bus)) << 20)) | (slot << 15) | (func << 12));
 
	return *addr;
}


void pci_write_bar(unsigned short bus,unsigned char dev,unsigned char func,unsigned short off,unsigned long in)

{

  unsigned long bar;
  
  bar = pci_read_coni(bus, dev,func, off);
  if ((bar & 1) == 0) {
   		//mem
		unsigned char type = (bar >> 1) & 0b11;
		if(type){//64bit
				pci_write_coni(bus, dev,func, off+4, in >> 32);//write in 4 bytes
		}
		pci_write_coni(bus, dev,func, off, in);
  }
  else {
	  
	  pci_write_coni(bus, dev, func, off, in);

  }
  return;
}


void pci_write_coni(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset, unsigned int in) {
 /*   unsigned int address;
    unsigned int lbus  = (unsigned int)bus;
    unsigned int lslot = (unsigned int)slot;
    unsigned int lfunc = (unsigned int)func;
    //unsigned short tmp = 0;

    // Create configuration address as per Figure 1
    address = (unsigned int)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xFC) | ((unsigned int)0x80000000));

    // Write out the address
    io_outl(0xCF8, address);
    // Read in the data
    // (offset & 2) * 8) = 0 will choose the first word of the 32-bit register
    io_outl(0xcfc, in);

  //  (unsigned short)((io_inl(0xCFC) >> ((offset & 2) * 8)) & 0xFFFF);
  //  return tmp;*/
      pci_bridge_ent *e = pci_find_bridge_from_ent(bus);

    
    volatile unsigned int* addr = (volatile unsigned int*) ((unsigned long) offset + (unsigned long)e->ecam_addr  + ( (((bus - e->start_bus)) << 20) | (slot << 15) | (func << 12)));
    *addr = in;
}

unsigned long pci_read_bar(unsigned char bus, unsigned char dev, unsigned char func, unsigned short off){
	unsigned long bar= pci_read_coni(bus, dev, func, off);
	if(bar & 1){
			//io
			bar &= 0xfffffffc;
	}
	else{
		//mem
		unsigned char type = (bar >> 1) & 0b11;
		if(type){//64bit
				bar |= ((unsigned long)pci_read_coni(bus,dev,func,off+4)) << 32; //read in 4 bytes
		}
		bar &= 0xfffffffffffffff0; //get rid of stuff we dont need
		
	}
	return bar;
};

unsigned long pci_read_bar_size(unsigned char bus, unsigned char dev, unsigned char func, unsigned short off){
	unsigned short cmd = pci_read_conw(bus,dev, func, 4); //cmd
	pci_write_conw(bus, dev, func, 4, cmd & ~(0b11)); //memory and io decode disable
	
	unsigned long old_bar = pci_read_bar(bus, dev, func, off);
	
	pci_write_bar(bus, dev, func, off, 0xffffffffffffffff);
	
	unsigned long sz = pci_read_bar(bus, dev, func, off);
	sz = ~sz + 1;
	sz &= 0xffffffff; //FIXME: doesnt work properly when 64 bit
	
	pci_write_bar(bus, dev, func, off, old_bar);//fix bar
	pci_write_conw(bus, dev, func, 4, cmd); //fix cmd
	
	
	return sz;
}

inline unsigned char pci_get_sub(unsigned char bus, unsigned char dev, unsigned char func){
    return pci_read_conw(bus, dev, func, 0xa) & 0xff;
}

inline unsigned char pci_get_sec(unsigned char bus, unsigned char dev, unsigned char func){
    return pci_read_conw(bus, dev, func, 0x18) >> 8;
}

inline unsigned char pci_get_cl(unsigned char bus, unsigned char dev, unsigned char func){

    return pci_read_conw(bus, dev, func, 0xa) >> 8;
}

inline unsigned char pci_get_type(unsigned char bus, unsigned char dev, unsigned char func){

    return pci_read_conw(bus, dev, func, 0xe) & 0xff;
}

unsigned short pci_get_vendor(unsigned char bus, unsigned char dev, unsigned char func) {
    /* Try and read the first configuration register. Since there are no
     * vendors that == 0xFFFF, it must be a non-existent device. */

    return pci_read_conw(bus, dev, func, 0);
}

void pci_enum_func(unsigned short bus, unsigned char dev, unsigned char func) {
    unsigned char base;
    unsigned char sub;
    unsigned char secondaryBus;

    base = pci_get_cl(bus, dev, func);

    sub = pci_get_sub(bus, dev, func);


    unsigned short devid, vendor;

    devid = pci_read_conw(bus, dev, func, 2);
    vendor = pci_get_vendor(bus, dev, func);

    pci_dev_ent* e = pci_new_dev_ent();


    e -> bus = bus;
    e -> dev = dev;
    e -> func = func;

    e -> cl = base;
    e -> subcl = sub;

    e -> vendor = vendor;
    e -> devid = devid;
	
	//VMDEEE
    if(bus == 0xe1){
	draw_string("adding nvme\n");
	
    }

    if(vendor == 0x8086 && (devid == 0x467f || devid == 0x201d)){
		unsigned char bits = pci_read_conw(bus, dev, func, 0x44) >> 8;
		bits &= 0b11;
		
		unsigned char vmd_bus;
		if(bits == 0) vmd_bus = 0;
		if(bits == 0x1) vmd_bus = 128;
		if(bits == 0b10) vmd_bus = 224;
		
		pci_bridge_ent* biter = pci_bridge_root;
		while(biter->ecam_addr){
		
			biter = biter->next;
		}
		
		
		biter->ecam_addr = (volatile void*)pci_read_bar(bus, dev, func, 0x10);
		biter->ecam_addr = (volatile void*) page_map_paddr_mmio((unsigned long)biter->ecam_addr, pci_read_bar_size(bus, dev, func, 0x10) / 0x200000);
		//map it
		
		//biter->ecam_addr &= 0xfffffffffffffff0;
		
		//if(biter->ec
		
		biter->start_bus = vmd_bus;
		biter->end_bus = vmd_bus + (unsigned long)pci_read_bar_size(bus, dev, func, 0x10) / (1 << 20);
		
		
		
			pci_enum_bus(vmd_bus);
		
		biter->next = k_obj_alloc(sizeof(pci_bridge_ent*));
		biter->next->ecam_addr = 0;
		
		
	}

    if ((base == 0x6) && (sub == 0x4)) {
         secondaryBus = pci_get_sec(bus, dev, func);
         pci_enum_bus(secondaryBus);
     }
 }

void pci_enum_dev(unsigned short bus, unsigned char device) {
     unsigned char function = 0;

     unsigned short vendor = pci_get_vendor(bus, device, function);
     if (vendor == 0xFFFF) return; // Device doesn't exist
     pci_enum_func(bus, device, function);
     unsigned char headerType = pci_get_type(bus, device, function);
     if( (headerType & 0x80) != 0) {
         // It's a multi-function device, so check remaining functions
         for (function = 1; function < 8; function++) {
             if (pci_get_vendor(bus, device, function) != 0xFFFF) {
                 pci_enum_func(bus, device, function);
             }
         }
     }
}


void pci_enum_bus(unsigned short bus) {
     unsigned char device;

     for (device = 0; device < 32; device++) {
         pci_enum_dev(bus, device);
     }
}



void pci_enum(){

	acpi_mcfg *mcfg_tab =  (acpi_mcfg*)acpiman_get_tab("MCFG");
	int bridges_num;
	
	draw_string_w_sz((char*)mcfg_tab, mcfg_tab->header.sz);
	draw_string("MCFG TAB ADDR: ");
	draw_hex((unsigned long) mcfg_tab);

	draw_string("MCFG BRIDGE NUMS:");
	
	
	draw_hex((bridges_num = (mcfg_tab->header.sz - 44 )/ 16));
	
	draw_string("MCFG SIZE: ");
	draw_hex(mcfg_tab->header.sz);
	
	pci_bridge_root = k_obj_alloc(sizeof(pci_bridge_ent));
	
	pci_bridge_ent *it = pci_bridge_root;
	
	for(int i=0; i < bridges_num; ++i){
		draw_string("BRIDGE ");
		draw_hex(i);
		
		draw_string("MCFG ECAM ADDR: ");
		draw_hex((unsigned long)mcfg_tab->bridges[i].ecam_addr);
			
		draw_string("MCFG ENDBUS: ");
		draw_hex((unsigned long) mcfg_tab->bridges[i].end_bus);
		
		it->ecam_addr = page_map_paddr_mmio((unsigned long) mcfg_tab->bridges[i].ecam_addr, ((mcfg_tab->bridges[i].end_bus - mcfg_tab->bridges[i].start_bus) << 20) / 2097152);
		it->start_bus = mcfg_tab->bridges[i].start_bus;
		it->end_bus = mcfg_tab->bridges[i].end_bus;
		

		it->next = k_obj_alloc(sizeof(pci_bridge_ent));
		it = it -> next;
		it->ecam_addr = 0;
		
		
	}
    //unsigned char bus;
    

  //  unsigned char type = pci_get_type(0,0,0);
    

        pci_enum_bus(0);


}
