#include "apic.h"
#include "io.h"

void apic_setup(){

    io_outb(0xa1, 0xff);
    io_outb(0x21, 0xff);

}
