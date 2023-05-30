typedef struct pci_dev_ent{

    struct pci_dev_ent *next;

    unsigned char bus,dev,func;

    unsigned char cl, subcl;

    unsigned short vendor;



} pci_dev_ent;


void pci_enum();
void pci_enum_bus(unsigned char bus);
