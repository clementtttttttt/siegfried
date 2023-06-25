CFLAGS=-std=gnu99 -ffreestanding -O2 -Wall -Wextra -m64  -mno-red-zone  -fno-builtin -nostdlib -static -Werror -Wno-unused-parameter
ASFLAGS=$(CFLAGS)

LDFLAGS=-z max-page-size=0x1000 -mno-red-zone -static

CC=clang --target=x86_64-pc-none-elf -march=x86-64
#CC=x86_64-elf-gcc

SOURCES=$(wildcard *.c)
HEADERS = $(wildcard *.h)

SOURCES_S=$(wildcard *.S)

OBJECTS=$(patsubst %.c, %.o, $(SOURCES))
OBJECTS_S=$(patsubst %.S, %.o, $(SOURCES_S))

OBJECTS2=$(addprefix obj/,$(OBJECTS))
OBJECTS2_S=$(addprefix obj/, $(OBJECTS_S))

all: sfkrnl.elf

sfkrnl.elf: $(OBJECTS2) $(OBJECTS2_S) linker.ld
	@echo $(OBJECTS2_S)
	$(CC) -T linker.ld -o sfkrnl.elf -ffreestanding -O2 -nostdlib $(OBJECTS2) $(OBJECTS2_S) -lgcc  -Wl,-Map=output.map $(LDFLAGS)

obj/%.o : %.c | obj
	$(CC) $(CFLAGS) -c $< -o $@

obj/%.o : %.S | obj
	$(CC) $(ASFLAGS) -c $< -o $@

obj :
	mkdir obj

sf.iso: sfkrnl.elf
	cp sfkrnl.elf isodir/boot/
	grub-mkrescue isodir -o sf.iso 
test: sf.iso
	qemu-system-x86_64 -cdrom sf.iso -d cpu_reset,int -drive file=test.img,if=none,id=nvm -device nvme,serial=deadbeef,drive=nvm -m 6G  -L /usr/share/ovmf/x64 -bios OVMF.fd -smp 3  -cpu host,+x2apic  -enable-kvm

clean:
	rm obj -rf -
	rm sfkrnl.elf -rf - 

install: sfkrnl.elf
	cp sfkrnl.elf /boot



