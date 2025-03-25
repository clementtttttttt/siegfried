#include "drivers.h"
#include "nvme.h"
#include "diskman.h"
#include "kb.h"
#include "devfs.h"
void drivers_setup(){

   
   devfs_setup();
   nvme_setup();
    diskman_setup();
	    kb_setup();

	
}
