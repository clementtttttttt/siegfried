union pml4e;
#include "diskman.h"
#include "syscall.h"
#pragma once

typedef struct tss_t{

    unsigned int rsvd;
    unsigned long rsp_0;
    unsigned long rsp_1;
    unsigned long rsp_2;

    unsigned long rsvd1;

    unsigned long ist_1;
    unsigned long ist_2;
    unsigned long ist_3;
    unsigned long ist_4;
    unsigned long ist_5;
    unsigned long ist_6;
    unsigned long ist_7;
    unsigned long rsvd2;
    unsigned short rsvd3;
    unsigned short iopb;

}__attribute__((packed)) tss_t;



typedef struct task_sframe{

	unsigned long fix_align;
    unsigned long ds;
    unsigned long rax;
    unsigned long rbp;
    unsigned long rbx;
    unsigned long r15;
    unsigned long r14;
    unsigned long r13;
    unsigned long r12;
    unsigned long r11;
    unsigned long r10;
    unsigned long r9;
    unsigned long r8;
    unsigned long rcx;
    unsigned long rdx;
    unsigned long rsi;
    unsigned long rdi;

    unsigned long rip;
    unsigned long cs;
    unsigned long flags;
    unsigned long rsp;
    unsigned long ss;

}__attribute__((packed)) task_int_sframe;

typedef struct task_trap_sframe{

	unsigned long fix_align; //makes the struct aligned to 16 bytes very importante
    unsigned long ds;
    unsigned long rax;
    unsigned long rbp;
    unsigned long rbx;
    unsigned long r15;
    unsigned long r14;
    unsigned long r13;
    unsigned long r12;
    unsigned long r11;
    unsigned long r10;
    unsigned long r9;
    unsigned long r8;
    unsigned long rcx;
    unsigned long rdx;
    unsigned long rsi;
    unsigned long rdi;

    unsigned long errcode;

    unsigned long rip;
    unsigned long cs;
    unsigned long flags;
    unsigned long rsp;
    unsigned long ss;

}__attribute__((packed)) task_trap_sframe;

typedef struct krnl_state{

    unsigned long r15;
    unsigned long r14;
    unsigned long r13;
    unsigned long r12;
    unsigned long r11;
    unsigned long rbp;
    unsigned long rbx;
    unsigned long rip;



}__attribute__((packed)) krnl_state;

enum task_states{
	T_NULL, T_RUNNING, T_DEAD
};

struct task_page_ent{
	unsigned short pages;
	void *addr;
};
#undef errno

void task_msgqueue_push(syscall_msg_t *in);
syscall_msg_t * task_msgqueue_pop(void);
syscall_msg_t * task_nmsg(pid_t dest, pid_t src, syscall_msg_type_t t,size_t dat_sz);



typedef struct task{
    volatile krnl_state *volatile krnl_state; //stack + STACK_SZ - sizeof(krnl_state).
    void* krnl_stack_base;
    void *user_stack_base;
    task_int_sframe *tf;
    char *name;
    unsigned int magic;
    union pml4e *page_tab;
    struct task *next;
    pid_t tid;
    unsigned long errno;
    unsigned long state;
    char ** env;
    char ** argv;
    pid_t creator;
   
	siegfried_dir *cwd;
    ino_t dm_inode;
    
    
    long task_page_ents;
    struct task_page_ent *task_page_ptr;
    
    
    syscall_msg_t *msg_queue[16];
    unsigned char msg_queue_head;
    unsigned char msg_queue_tail;
    
    
    
}task;

extern volatile  task *volatile curr_task;
void task_dump_sframe(task_int_sframe *in);

void task_scheduler();
void task_yield();
void tasks_setup();
void task_save_and_change_krnl_state(volatile krnl_state *volatile*old_ptr_to_stack_addr,volatile  krnl_state *volatile new_ptr_to_stack_addr);

void task_exit(syscall_child_died_type_t);
void task_enter_krnl();
void task_exit_krnl();
task *task_start_func(void *func);
task* task_find_by_tid(pid_t in);

void task_dump_sframe(task_int_sframe *in);
