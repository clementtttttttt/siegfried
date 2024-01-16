#include "drivers.h"
#include "nvme.h"
#include "diskman.h"
#include "kb.h"

void drivers_setup(){

    nvme_setup();
    diskman_setup();
	    kb_setup();

	
}
