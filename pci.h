#ifndef PCI_H
#define PCI_H


typedef struct pci_dev_ent{

    struct pci_dev_ent *next;

    unsigned char bus,dev,func;

    unsigned char cl, subcl;

    unsigned short vendor, devid;



} pci_dev_ent;

typedef struct pci_bridge_ent{
	struct pci_bridge_ent* next;
	unsigned char start_bus;
	unsigned char end_bus;
	volatile void* ecam_addr;
	
} pci_bridge_ent;


void pci_enum();
void pci_enum_bus(unsigned char bus);
pci_dev_ent *pci_get_next_dev_ent(pci_dev_ent *in);
unsigned short pci_read_conw(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset);
void pci_write_conw(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset, unsigned short in);
unsigned int pci_read_coni(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset) ;
void pci_write_coni(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset, unsigned int in) ;
unsigned long pci_read_bar(unsigned char bus, unsigned char dev, unsigned char func, unsigned char off);
#endif
