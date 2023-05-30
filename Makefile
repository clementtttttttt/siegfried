CFLAGS=-std=gnu99 -ffreestanding -O2 -Wall -Wextra -m64  -mno-red-zone -mno-sse -mno-sse2  -fno-builtin -nostdlib -static -Werror -Wno-unused-parameter
ASFLAGS=$(CFLAGS)

LDFLAGS=-z max-page-size=0x1000 -mno-red-zone -static

CC=clang --target=x86_64-pc-none-elf -march=x86-64
#CC=x86_64-elf-gcc

SOURCES=$(wildcard *.c)
SOURCES_S=$(wildcard *.S)

OBJECTS=$(patsubst %.c, %.o, $(SOURCES)) 
OBJECTS_S=$(patsubst %.S, %.o, $(SOURCES_S))

all: sfkrnl.elf

$(OBJECTS): %.o : %.c
$(OBJECTS_S): %.o : %.S

sfkrnl.elf: $(OBJECTS) $(OBJECTS_S) linker.ld
	$(CC) -T linker.ld -o sfkrnl.elf -ffreestanding -O2 -nostdlib $(OBJECTS) $(OBJECTS_S) -lgcc  -Wl,-Map=output.map $(LDFLAGS)

iso: sfkrnl.elf
	cp sfkrnl.elf isodir/boot/
	grub-mkrescue isodir -o sf.iso 
test: iso
	qemu-system-x86_64 -cdrom sf.iso -d cpu_reset,int -drive file=test.img,if=none,id=nvm -device nvme,serial=deadbeef,drive=nvm -m 6G  -cpu host -enable-kvm

install: sfkrnl.elf
	cp sfkrnl.elf /boot
