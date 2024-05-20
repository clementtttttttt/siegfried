#include "tasks.h"
#include "draw.h"
#include "obj_heap.h"
#include "klib.h"
#include "page.h"
#include "pageobj_heap.h"
#include "syscall.h"
#include <stdatomic.h>
#include "runner.h"
#include "idt.h"

unsigned char tasking_enabled = 0;

task *tasks=0;

task *curr_task=0;

extern tss_t tss;
extern KHEAPSS page_heap;
#define TASK_STACK_SZ 1048576 //account for red zone

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
        retq;\
        ");

void task_switch_tab(pml4e *tab);
asm("task_switch_tab:\
        movq %rdi, %cr3;\
        ret;");


void task_pre_init(){

}



unsigned long tid_counter = 0;

task *task_new(){
    char *stack; //get rid of pointer arithmatic bullshit

    task *curr;

    if(tasks == 0){
        curr = tasks = k_obj_alloc(sizeof(task));
        if(curr == 0) {
				idt_print_stacktrace(__builtin_frame_address(0));
		}
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
    stack += (TASK_STACK_SZ - sizeof(task_int_sframe)-512);
    
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


task *task_start_func(void *func){ //note: requires func mapped to user space
        task *new = task_new();

        new -> tf -> cs = 0x18 | 3;
        new -> tf -> ds = new -> tf -> ss = 0x20 | 3;

        new -> tf -> flags = 0x200;
        new -> tf -> rip = (unsigned long) func;

        new->page_tab = k_pageobj_alloc(&page_heap, 4096 );
        page_clone_krnl_tab(new->page_tab);
		
		new->user_stack_base = page_find_and_alloc_user(new -> page_tab, page_virt_find_addr_user(new->page_tab, 1),1);
        new -> tf -> rsp = ((unsigned long) new->user_stack_base)+ TASK_STACK_SZ-512; //


        return new;
        //new -> page_tab
}

extern pml4e pml4_table[];

extern unsigned long krnl_init_inode;


void tasks_setup(){

    asm("cli");
    mem_set(&tss, 0, sizeof(tss));
    curr_task = 0;
    tss.rsp_0 = (unsigned long)k_obj_alloc(16384);
    tss.iopb = 0xffff;
    tasking_enabled = 1; 
    asm volatile("movw $0x28, %%ax; ltrw %%ax":::"ax");

    
    if(!runner_spawn_task(krnl_init_inode, "/sbin/sfinit",0,0,0)){ //tid 0 = fail
	if(!runner_spawn_task(krnl_init_inode, "/sfinit",0,0,0)){ //try other paths

	}
    }

	

   	
    curr_task = tasks;


}



krnl_state *scheduler_state;

volatile int task_in_krnl = 0;

void task_scheduler(){
	
		sched_lock = 1;
		
        while(1){
	                page_switch_krnl_tab();

                if(curr_task == 0 && tasks == 0) continue; //continue while curr task is 0

				
                if(curr_task->next == 0) {
                        curr_task = tasks;
                }
                else {
                        curr_task = curr_task->next;
                }
			//	draw_hex((unsigned long)curr_task);
 
				if(curr_task->state == T_DEAD){
					if(curr_task == tasks){
						if(curr_task == 0){
								draw_string("WE RAN OUT OF TASKS!");
								draw_hex((unsigned long)curr_task);
								draw_hex((unsigned long)tasks);

								while(1){}
						}
						
						if(curr_task->next != 0){
							tasks = curr_task->next;
						}
						
	
					}
					else{
						task *iter= tasks;

						while(iter->next != curr_task)iter=iter->next;
					
						iter->next = curr_task->next;
					}
					k_obj_free(curr_task->krnl_stack_base);
					page_free_found_user(curr_task->page_tab, (unsigned long)curr_task->user_stack_base, 1);
					page_free_tab(curr_task->page_tab);
			

					task *nxt = curr_task->next;
					if(curr_task != tasks){
						k_obj_free(curr_task);
					}
					if(nxt == 0) curr_task = tasks;
					continue;
			
				}


                task_set_tss((unsigned long)curr_task->krnl_stack_base + TASK_STACK_SZ-512-sizeof(task_int_sframe));

		                page_switch_tab(curr_task->page_tab);

                task_save_and_change_krnl_state(&scheduler_state, curr_task->krnl_state);
                
        }

}

void task_dump_sframe(task_int_sframe *in){
	unsigned long *in_long = (unsigned long*)in; 
	char *reg_name[] = {
		"DS",
		"RAX",
		"RBP",
		"RBX",
		"R15",
		"R14",
		"R13",
		"R12",
		"R11",
		"R10",
		"R9",
		"R8",
		"RCX",
		"RDX", 
		"RSI",
		"RDI",
		
		
		
		
	};
	for(int i=0;i<16;++i){
		draw_string(reg_name[i]);
		draw_string(":");
		draw_hex(in_long[i]);
	}

}

void task_exit(unsigned long code){


	
	page_switch_krnl_tab();

		
	curr_task->state = T_DEAD;


	asm("sti");
	while(1){}
}

void task_yield(){


        if(sched_lock){
                return;
        }
        if(curr_task == 0 ){
                return;
        }
        if(scheduler_state == 0){
                return;
        }

        atomic_store(&sched_lock, 1);
		
		
        task_save_and_change_krnl_state(&curr_task->krnl_state, scheduler_state);
		
	
}

void task_enter_krnl(){
        task_in_krnl = 1;
}
void task_exit_krnl(){
        task_in_krnl = 0;
}
