#include "tasks.h"
#include "draw.h"
#include "obj_heap.h"
#include "klib.h"
#include "page.h"
#include "pageobj_heap.h"
#include "syscall.h"
#include <stdatomic.h>

unsigned char tasking_enabled = 0;

task *tasks;

task *curr_task;

extern tss_t tss;
extern KHEAPSS page_heap;
#define TASK_STACK_SZ 8192

atomic_int sched_lock;

void task_set_tss(unsigned long in){
        tss.rsp_0 = in;
}

void task_load_task_regs_and_spawn();
asm("task_load_task_regs_and_spawn:;\
        movq $2f, rip_save ;\
        jmp idt_load_regs\n\
        2: ;\
        orq $0x200, 16(%rsp);\
                                ;\
        iretq");


void task_pre_init(){

}

void task_use_krnl_tab(){
        //asm("movq $pml4_table,%%rax; movq %%rax, %%cr3":::"rax");
}

void task_exit(){
        if(curr_task->tid == 1){
                draw_string("INIT DIED\n");
                while(1);
        }
}

unsigned long tid_counter = 0;

task *task_new(){
    char *stack; //get rid of pointer arithmatic bullshit

    task *curr;

    if(tasks == 0){
        curr = tasks = k_obj_alloc(sizeof(task));
        curr->next = 0;
    }
    else{
        curr = tasks;
        while(curr->next) curr = curr->next;

        curr->next = k_obj_alloc(sizeof(task));
        curr=curr->next;
        curr->next = 0;

    }

    //make init task. tasking code inspired by xv6

    stack = curr->krnl_stack_base = k_obj_alloc(TASK_STACK_SZ);
    stack += (TASK_STACK_SZ - sizeof(task_int_sframe));
    curr->tf = (task_int_sframe*) stack;

    stack -= 8;
    *((void(**)())stack) = task_load_task_regs_and_spawn;

    stack -= sizeof(krnl_state);
    curr->krnl_state = (krnl_state*) stack;
    mem_set(curr->krnl_state, 0 , sizeof(krnl_state));

    curr->krnl_state->rip = (unsigned long) task_pre_init;

    curr -> tid = ++tid_counter;

    stack -= 8;

    *((void(**)())stack) = task_exit;


    return curr;
}

void init_loader(){


        while(1){
              // syscall0(0);
               syscall0(3);
        }

}

void second(){
        while(1){
              // syscall0(0);
                        syscall0(5);
             //   syscall0(1);

        }
}

void task_start_func(void *func){
        task *new = task_new();

        new -> tf -> cs = 0x18 | 3;
        new -> tf -> ds = new -> tf -> ss = 0x20 | 3;

        new -> tf -> flags = 0x200;
        new -> tf -> rip = (unsigned long) func;

        new -> tf -> rsp = (unsigned long) k_obj_alloc(TASK_STACK_SZ);
}


void tasks_setup(){

    asm("cli");

    mem_set(&tss, 0, sizeof(tss));

    tss.rsp_0 = (unsigned long)k_obj_alloc(16384);
    tss.iopb = 0xffff;
    tasking_enabled = 1;
    asm volatile("movw $0x28, %%ax; ltrw %%ax":::"ax");

    task_start_func(init_loader);
    task_start_func(second);

    curr_task = tasks;

}




void task_save_and_change_krnl_state(krnl_state **old_ptr_to_stack_addr, krnl_state *new_ptr_to_stack_addr);


asm(".globl task_save_and_change_krnl_state;\
    task_save_and_change_krnl_state:;\
        cli;\
        pushq %rbx;\
        pushq %rbp;\
        pushq %r11;\
        pushq %r12;\
        pushq %r13;\
        pushq %r14;\
        pushq %r15;\
                ;\
        movq %rsp, (%rdi);\
        movq %rsi, %rsp;\
;\
        popq %r15;\
        popq %r14;\
        popq %r13;\
        popq %r12;\
        popq %r11;\
        popq %rbp;\
        popq %rbx;\
        movl $0, sched_lock;\
        xchg %bx,%bx;\
        retq;\
        ");

krnl_state *scheduler_state;

volatile int task_in_krnl = 0;

void task_scheduler(){

        atomic_store(&sched_lock, 1);

        while(1){

                if(curr_task == 0) continue; //continue while curr task is 0


                if(curr_task->next == 0) {
                        curr_task = tasks;
                }
                else {
                        curr_task = curr_task->next;
                }


                task_set_tss((unsigned long)curr_task->krnl_stack_base + TASK_STACK_SZ);

                task_save_and_change_krnl_state(&scheduler_state, curr_task->krnl_state);
        }

}

void task_yield(){

        if(atomic_load(&sched_lock)){
                return;
        }
        if(curr_task == 0 ){
                return;
        }
        if(scheduler_state == 0){
                return;
        }

        atomic_store(&sched_lock, 1);

        task_use_krnl_tab();
        task_save_and_change_krnl_state(&curr_task->krnl_state, scheduler_state);


}

void task_enter_krnl(){
        task_in_krnl = 1;
}
void task_exit_krnl(){
        task_in_krnl = 0;
}
