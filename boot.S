.section .bss
.align 16
stack_bottom:
.skip 65536
stack_top:
.skip 1024

.section .text
.global _start
.type _start, @function

tss: 
	.skip 0x64

gdtr:
	.word gdt_end - gdt_start - 1
	.quad gdt_start


gdt_start:

	.null_sd: .quad 0

	.krnlT_sd: 
		.word 0xffff
		.word 0
		.byte 0
		.byte 0x9a
		.byte 0xAF
		.byte 0

	.krnlD_sd: 
		.word 0xffff
		.word 0
		.byte 0
		.byte 0x92
		.byte 0xcf
		.byte 0
	
	.userT_sd:
		.word 0xffff
		.word 0
		.word 0
		.byte 0xfa
		.byte 0xAF
		.byte 0

	.userD_sd:
		.word 0xffff
		.word 0
		.word 0
		.byte 0xf2
		.byte 0xcf
		.byte 0

	.tss_sd:
		.word 0x64
		.tss_addr_w16: .word 0 //$tss & 0xffff
		.tss_addr_b24: .byte 0 //($tss >> 16) & 0xff
		.byte 0x89
		.byte 0x00
		.tss_addr_b32: .byte 0 //($tss >> 24) & 0xff
		.tss_addr_i64: .int 0 //($tss >> 32) & 0xffff
	

gdt_end:

eax_keep: .quad 0
ebx_keep: .quad 0

.align 0x1000

dummy_page:
	.skip 0x4000

_start:

	.code32
	//fix stack
	movl $stack_top, %esp

	//save eax&ebx
	movl %eax, eax_keep
	movl %ebx, ebx_keep


	//write msr to enable lm bit
    movl $0xC0000080, %ecx
    rdmsr
    orl $0b100000000 , %eax
    wrmsr


	//fix pages

	//pml4
	movl $dummy_page + 0x1000, %eax
	orl $0b11, %eax
	movl %eax, dummy_page

	//pdpt
	movl $dummy_page + 0x2000, %eax
	orl $0b00000011, %eax
	movl %eax, dummy_page+0x1000

	//pd

	movl $0, %ecx
	movl $dummy_page + 0x2000, %edi
	.L1:
		movl %ecx, %eax
		orl $0b10000011, %eax
		movl %eax, (%edi)

		addl $1 << 21, %ecx
		addl $8, %edi

		cmpl $0x40 << 21, %ecx
		jnz .L1

	movl %cr4, %eax
	orl $0b100000, %eax                // Set the PAE and PGE bit.
    movl %eax, %cr4

    movl $dummy_page, %edx                    // Point CR3 at the PML4.
    movl %edx, %cr3

    //enable paging
	movl %cr0, %ebx
    orl $0x80000001, %ebx
    movl %ebx, %cr0

	//fill out tss seg
	movl $tss, %edx
	
	movw %dx, .tss_addr_w16
	shr $16, %edx
	
	movb %dl, .tss_addr_b24
	shr $8, %edx
	
	movb %dl, .tss_addr_b32
	shr $8, %edx
	
	movw %dx, .tss_addr_i64

	//load own gdt segs
	lgdt gdtr

	movw $0x10, %dx

	movw %dx, %ds
	movw %dx, %es
	movw %dx, %fs
	movw %dx, %gs
	movw %dx, %ss

	pushl $0x10
	pushl $0x8
	pushl $1f

	iretl
	1:

	.code64

	//enable sse for fast code

	movq %cr0, %rax
	btrq $2, %rax
	movq %rax, %cr0

	movq %cr4, %rax
	btsq $9, %rax
	btsq $10, %rax
	movq %rax,%cr4

	//enter c code

	movq eax_keep, %rdi

	movq ebx_keep, %rsi



	call krnl_main


	jmp .


.size _start, . - _start
