OPT_FLAGS=-O0

CFLAGS= -Werror -Wno-array-bounds  -Wno-address-of-packed-member  -std=gnu99 -ffreestanding $(OPT_FLAGS) -Wall -Wextra -g -mno-red-zone  -nostdlib -static  -Wno-unused-parameter -march=k8 -mtune=k8 
ASFLAGS=$(CFLAGS)

LDFLAGS=-z max-page-size=0x1000 -mno-red-zone -static $(OPT_FLAGS)
#CC=clang --target=x86_64-pc-none-elf -march=x86-64
CC=x86_64-pc-none-elf-gcc

SOURCES=$(wildcard *.c)
HEADERS = $(wildcard *.h)

SOURCES_S=$(wildcard *.S)

OBJECTS=$(patsubst %.c, %.o, $(SOURCES))
OBJECTS_S=$(patsubst %.S, %.o, $(SOURCES_S))

OBJECTS2=$(addprefix obj/,$(OBJECTS))
OBJECTS2_S=$(addprefix obj/, $(OBJECTS_S))

all: posh/posh sf.iso Makefile

posh/posh: posh/main.c posh/comp.sh
	cd posh && ./comp.sh
	

sfkrnl.elf: $(OBJECTS2) $(OBJECTS2_S) linker.ld
	@$(CC)  -T linker.ld -o sfkrnl.elf -ffreestanding  -nostdlib $(OBJECTS2) $(OBJECTS2_S)   -Wl,-Map=output.map $(LDFLAGS)
	@echo CCLD\($(CC)\) $@

obj/%.o : %.c | obj
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo CC\($(CC)\) $@

obj/%.o : %.S | obj
	@$(CC) $(ASFLAGS) -c $< -o $@
	@echo CCAS\($(CC)\) $@

obj :
	mkdir obj

sf.iso: sfkrnl.elf
	cp sfkrnl.elf isodir/boot/
	grub-mkrescue isodir -o sf.iso 
test: all
	qemu-system-x86_64 -enable-kvm -D log -S -s -cdrom sf.iso -machine q35  -m 4096 -d int,cpu_reset -drive file=/dev/nvme0n1,if=none,id=nvm -snapshot -device nvme,serial=deadbeef,drive=nvm -bios /usr/share/edk2-ovmf/OVMF_CODE.csm.fd    -cpu kvm64 -monitor stdio -boot splash-time=0

clean:
	rm obj -rf -
	rm sfkrnl.elf -rf - 

install: sfkrnl.elf
	cp sfkrnl.elf /boot


dump: sfkrnl.elf 
	objdump -S -D -g -l sfkrnl.elf > dump
