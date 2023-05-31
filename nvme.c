#include "pci.h"
#include "debug.h"

void nvme_setup_pci_dev(pci_dev_ent *in){
    dbgconout("FOUND NVME\r\n");
    dbgnumout_hex(in->bus);
    dbgnumout_hex(in->dev);
    dbgnumout_hex(in->func);

}

void nvme_setup(){

    pci_dev_ent* it = 0;

    while((it = pci_get_next_dev_ent(it)) != 0){
        dbgnumout_hex((unsigned long)it->cl);
        if(it->cl == 1 && it->subcl == 8){
            nvme_setup_pci_dev(it);
        }
    }
}
