#include "pci.h"
#include "debug.h"
#include "io.h"
#include "obj_heap.h"
#include "draw.h"

pci_dev_ent *pci_root = 0;

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

void pci_write_conw(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset, unsigned short in) {
    unsigned int address;
    unsigned int lbus  = (unsigned int)bus;
    unsigned int lslot = (unsigned int)slot;
    unsigned int lfunc = (unsigned int)func;
    //unsigned short tmp = 0;
//TODO: implement pci_write_conw
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

}

unsigned short pci_read_conw(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset) {
    unsigned int address;
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
}

unsigned int pci_read_coni(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset) {
    unsigned int address;
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
    return io_inl(0xcfc);
}

void pci_write_coni(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset, unsigned long in) {
    unsigned int address;
    unsigned int lbus  = (unsigned int)bus;
    unsigned int lslot = (unsigned int)slot;
    unsigned int lfunc = (unsigned int)func;
    //unsigned short tmp = 0;
//TODO: implement pci_write_conw
    // Create configuration address as per Figure 1
    address = (unsigned int)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xFC) | ((unsigned int)0x80000000));

    // Write out the address
    io_outl(0xCF8, address);
    // Read in the data
    // (offset & 2) * 8) = 0 will choose the first word of the 32-bit register
    io_outl(0xcfc, in);

  //  (unsigned short)((io_inl(0xCFC) >> ((offset & 2) * 8)) & 0xFFFF);
  //  return tmp;
}

unsigned char pci_get_sub(unsigned char bus, unsigned char dev, unsigned char func){
    return pci_read_conw(bus, dev, func, 0xa) & 0xff;
}

unsigned char pci_get_sec(unsigned char bus, unsigned char dev, unsigned char func){
    return pci_read_conw(bus, dev, func, 0x18) >> 8;
}

unsigned char pci_get_cl(unsigned char bus, unsigned char dev, unsigned char func){

    return pci_read_conw(bus, dev, func, 0xa) >> 8;
}

unsigned char pci_get_type(unsigned char bus, unsigned char dev, unsigned char func){

    return pci_read_conw(bus, dev, func, 0xe) & 0xff;
}

unsigned short pci_get_vendor(unsigned char bus, unsigned char dev, unsigned char func) {
    /* Try and read the first configuration register. Since there are no
     * vendors that == 0xFFFF, it must be a non-existent device. */

    return pci_read_conw(bus, dev, func, 0);
}

 void pci_enum_func(unsigned char bus, unsigned char dev, unsigned char func) {
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


     if ((base == 0x6) && (sub == 0x4)) {
         secondaryBus = pci_get_sec(bus, dev, func);
         pci_enum_bus(secondaryBus);
     }
 }

void pci_enum_dev(unsigned char bus, unsigned char device) {
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


void pci_enum_bus(unsigned char bus) {
     unsigned char device;

     for (device = 0; device < 32; device++) {
         pci_enum_dev(bus, device);
     }
}



void pci_enum(){

    unsigned char bus;
    

    unsigned char type = pci_get_type(0,0,0);
    

    if((type&0x80) == 0){
        pci_enum_bus(0);
    }
    else{
    	draw_string("PCI: found a multi bus system");
        for (int function = 0; function < 8; function++) {
             if (pci_get_vendor(0, 0, function) == 0xFFFF) continue;
             bus = function;
	     draw_string("PCI: scanning bus ");
	     draw_hex(bus);
             pci_enum_bus(bus);
         }
    }

}
