union pml4e;

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

typedef struct task{
    krnl_state *krnl_state; //stack + STACK_SZ - sizeof(krnl_state).
    void* krnl_stack_base;
    task_int_sframe *tf;
    char *name;
    union pml4e *page_tab;
    struct task *next;
    unsigned long tid;
    unsigned long errno;
    unsigned long state;
}task;

extern task *curr_task;


void task_scheduler();
void task_yield();
void tasks_setup();
void task_save_and_change_krnl_state(krnl_state **old_ptr_to_stack_addr, krnl_state *new_ptr_to_stack_addr);

void task_exit(unsigned long);
void task_enter_krnl();
void task_exit_krnl();
task *task_start_func(void *func);
void init_loader_end();
__attribute__((noinline, section(".init_loader"))) void init_loader();
