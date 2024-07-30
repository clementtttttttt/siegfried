CFLAGS=-fno-omit-frame-pointer -Wno-address-of-packed-member  -std=gnu99 -ffreestanding -Og -Wall -Wextra -g -mno-red-zone -fno-builtin -fno-builtin-memcpy -fno-builtin-memset -nostdlib -static -Werror -Wno-unused-parameter -fno-stack-protector -march=k8 -mtune=k8 
ASFLAGS=$(CFLAGS)

LDFLAGS=-z max-page-size=0x1000 -mno-red-zone -static

CC=clang --target=x86_64-pc-none-elf -march=x86-64
#CC=x86_64-pc-none-elf-gcc

SOURCES=$(wildcard *.c)
HEADERS = $(wildcard *.h)

SOURCES_S=$(wildcard *.S)

OBJECTS=$(patsubst %.c, %.o, $(SOURCES))
OBJECTS_S=$(patsubst %.S, %.o, $(SOURCES_S))

OBJECTS2=$(addprefix obj/,$(OBJECTS))
OBJECTS2_S=$(addprefix obj/, $(OBJECTS_S))

all: sfkrnl.elf Makefile

sfkrnl.elf: $(OBJECTS2) $(OBJECTS2_S) linker.ld
	@$(CC) -T linker.ld -o sfkrnl.elf -ffreestanding  -Og -nostdlib $(OBJECTS2) $(OBJECTS2_S)   -Wl,-Map=output.map $(LDFLAGS)
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
test: sf.iso
	qemu-system-x86_64 -S -enable-kvm -s -cdrom sf.iso -machine q35  -m 4096 -d guest_errors,cpu_reset -drive file=test.img,if=none,id=nvm -device nvme,serial=deadbeef,drive=nvm -bios /usr/share/edk2-ovmf/OVMF_CODE.fd    -cpu Skylake-Client -monitor stdio 

clean:
	rm obj -rf -
	rm sfkrnl.elf -rf - 

install: sfkrnl.elf
	cp sfkrnl.elf /boot


dump: sfkrnl.elf 
	objdump -S -D -g -l sfkrnl.elf > dump
