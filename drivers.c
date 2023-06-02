#include "drivers.h"
#include "nvme.h"
#include "diskman.h"

void drivers_setup(){

    nvme_setup();
    diskman_setup();

}
