#define ASM_FILE        1
#include "multiboot2.h"

.section .multiboot

.align 8

header:

.int MULTIBOOT2_HEADER_MAGIC

.int MULTIBOOT_ARCHITECTURE_I386

.int header_end - header

.int -(MULTIBOOT2_HEADER_MAGIC + MULTIBOOT_ARCHITECTURE_I386 + (header_end - header)) 


info_tag_start:

.word MULTIBOOT_HEADER_TAG_INFORMATION_REQUEST

.word 0

.int info_tag_end - info_tag_start

.int MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME

.int MULTIBOOT_TAG_TYPE_CMDLINE

.int MULTIBOOT_TAG_TYPE_BOOTDEV

//.int MULTIBOOT_TAG_TYPE_EFI64

.int MULTIBOOT_TAG_TYPE_ACPI_NEW

.int MULTIBOOT_TAG_TYPE_FRAMEBUFFER

.int MULTIBOOT_TAG_TYPE_ELF_SECTIONS

info_tag_end:

.align 8

fb_tag_start:

.word MULTIBOOT_HEADER_TAG_FRAMEBUFFER

.word 0

.int fb_tag_end - fb_tag_start

#resolution goes here
.int 1280

.int 720

.int 32

.align 8

fb_tag_end:


term_tag_start:

.word MULTIBOOT_HEADER_TAG_END

.word 0

.int 8

term_tag_end:


header_end:
