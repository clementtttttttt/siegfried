#include "pci.h"
#include "debug.h"
#include "io.h"

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

    // Write out the address
    io_outl(0xCF8, address);
    // Read in the data
    // (offset & 2) * 8) = 0 will choose the first word of the 32-bit register
    io_outl(0xcfc, in);

  //  (unsigned short)((io_inl(0xCFC) >> ((offset & 2) * 8)) & 0xFFFF);
  //  return tmp;
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

unsigned char pci_get_sub(unsigned char bus, unsigned char dev, unsigned char func){
    return pci_read_conw(bus, dev, func, 0x10) & 0xff;
}

unsigned char pci_get_sec(unsigned char bus, unsigned char dev, unsigned char func){
    return pci_read_conw(bus, dev, func, 0x18) >> 8;
}

unsigned char pci_get_base(unsigned char bus, unsigned char dev, unsigned char func){

    return pci_read_conw(bus, dev, func, 0x10) >> 8;
}

unsigned char pci_get_type(unsigned char bus, unsigned char dev, unsigned char func){

    return pci_read_conw(bus, dev, func, 0xe) & 0xff;
}

unsigned short pci_get_vendor(unsigned char bus, unsigned char slot, unsigned char func) {
    unsigned short vendor, device;
    /* Try and read the first configuration register. Since there are no
     * vendors that == 0xFFFF, it must be a non-existent device. */
    if ((vendor = pci_read_conw(bus, slot, func, 0)) != 0xFFFF) {
       device = pci_read_conw(bus, slot, func, 2);

       vendor = vendor + device - device;

       return vendor;

    } return (vendor);
}

 void pci_enum_func(unsigned char bus, unsigned char device, unsigned char function) {
     unsigned char baseClass;
     unsigned char subClass;
     unsigned char secondaryBus;

     baseClass = pci_get_base(bus, device, function);
     subClass = pci_get_sub(bus, device, function);

        dbgnumout_hex(pci_get_vendor(bus, device, function));

     if ((baseClass == 0x6) && (subClass == 0x4)) {
         secondaryBus = pci_get_sec(bus, device, function);
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
        for (int function = 0; function < 8; function++) {
             if (pci_get_vendor(0, 0, function) != 0xFFFF) break;
             bus = function;
             pci_enum_bus(bus);
         }
    }

}
