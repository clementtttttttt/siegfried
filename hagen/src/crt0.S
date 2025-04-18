.section .text

.global _start
_start:
	# Set up end of the stack frame linked list.
	movq $0, %rbp
	pushq %rbp # rip=0
	pushq %rbp # rbp=0
	movq %rsp, %rbp

	# We need those in a moment when we call main.
	pushq %rsi
	pushq %rdi

	# Prepare signals, memory allocation, stdio and such.
#	call initialize_standard_library

	# Run the global constructors.
	call _init

	# Restore argc and argv.
	popq %rdi
	popq %rsi

	# Run main
	call main

	# Terminate the process with the exit code.
	movq %rax, %rsi
	movq $0, %rdi
	int $0xf0
.size _start, . - _start
