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

static task *tasks=0;

volatile task *volatile curr_task=0;

extern tss_t tss;
extern KHEAPSS page_heap;
#define TASK_STACK_SZ 1048576 //account for red zone

char scheduler_started = 0;


int get_tasks_list_len(){
		task *task_it = tasks;
		int i=0;
		do{
			++i;
			task_it = task_it->next;
		}
		while(task_it != tasks);
		return i;
			
}

inline static void task_set_tss(unsigned long in){
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
        curr->next = tasks;
    }
    else{
        curr = tasks;
        while(curr->next != tasks) curr = curr->next;

        curr->next = k_obj_alloc(sizeof(task));
        curr=curr->next;
        curr->next = tasks;

    }

	curr->magic =  0x1badb001;

    //make init task. tasking code inspired by xv6

    stack = curr->krnl_stack_base = k_obj_alloc(TASK_STACK_SZ);
    stack += (TASK_STACK_SZ - sizeof(task_int_sframe)-512);
    
    curr->tf = (task_int_sframe*) stack;

    stack -= 8;
    *((void(**)())stack) = task_load_task_regs_and_spawn;

    stack -= sizeof(krnl_state);
    curr->krnl_state = (krnl_state*) stack;
    //not volatile yet
    mem_set((void*)curr->krnl_state, 0 , sizeof(krnl_state));

    curr->krnl_state->rip = (unsigned long) task_pre_init;

    curr -> tid = ++tid_counter;
    
    curr-> state = T_RUNNING;

	curr -> msg_queue_tail = 0;
	curr->msg_queue_head = 0;
	mem_set(curr->msg_queue, 0 ,sizeof(curr->msg_queue));

    stack -= 8;

	curr->cwd = 0;

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
		
		        if(new->page_tab == 0){
			while(1){}
		}
		
		new->user_stack_base = page_find_and_alloc_user(new -> page_tab, page_virt_find_addr_user(new->page_tab, 1),1);
        new -> tf -> rsp = ((unsigned long) new->user_stack_base)+ TASK_STACK_SZ-512; //


        return new;
        //new -> page_tab
}

extern pml4e pml4_table[];

extern unsigned long krnl_init_inode;

task* task_find_by_tid(pid_t in){
	task *it = tasks;
	
	do{
			if(it->tid == in){
				return it;
			}
			it = it->next;
			
	}	while(it->next != tasks);
	return NULL;
}

void tasks_setup(){

    asm("cli");
    mem_set(&tss, 0, sizeof(tss));
    curr_task = 0;
    tss.iopb = 0xffff;
    tasking_enabled = 1; 
    asm volatile("movw $0x28, %%ax; ltrw %%ax":::"ax");

    
    if(runner_spawn_task(krnl_init_inode, "/sbin/sfinit",0,0,0)<0){ //tid <0 = fail
	if(runner_spawn_task(krnl_init_inode, "/sfinit",0,0,0)<0){ //try other paths

	}
    }

	

   	
    curr_task = tasks;


}



static volatile krnl_state *volatile scheduler_state;

volatile int task_in_krnl = 0;

void task_cleanup_zombie(){

					
					if((unsigned long)curr_task->krnl_stack_base == 0xDEADBEEF){
							draw_string("attempting to clean up a cleaned up task?\n");
							draw_hex(curr_task==tasks);
							while(1){}
					}
					k_obj_free(curr_task->krnl_stack_base);
					curr_task->krnl_stack_base =(void*) 0xDEADBEEF;
					
					k_obj_free(curr_task->env);
					
			
					
					page_free_found_user(curr_task->page_tab, (unsigned long)curr_task->user_stack_base, 1);
					
					for(int i=0; i<curr_task->task_page_ents;++i){
						//page_free_found_user(curr_task->page_tab,(unsigned long)curr_task->task_page_ptr[i].addr, curr_task->task_page_ptr[i].pages);
						page_unmark_phys_mem_map((unsigned long)curr_task->task_page_ptr[i].addr, curr_task->task_page_ptr[i].pages);
					}
					k_obj_free(curr_task->task_page_ptr);
					
					
					if(curr_task->page_tab == page_get_krnl_tab()){
						while(1){}
					}
					page_free_tab(curr_task->page_tab);
					
					task *next = curr_task->next;
					task *prev = tasks;
					
					while(prev->next != curr_task){
						
						prev = prev->next;
					}
					prev -> next = next;
					
					if(curr_task == tasks){

						
						task *iter= tasks;
						while(iter->next != tasks) iter = iter->next;

						if(iter == tasks){
								draw_string("WE RAN OUT OF TASKS!");
								draw_hex((unsigned long)curr_task);
								draw_hex((unsigned long)tasks);

								while(1){}
						}
						
						iter->next = tasks->next;
						tasks = tasks->next;
						k_obj_free((void*)curr_task);
						curr_task = tasks;
					}
					else{
						k_obj_free((void*)curr_task);
						
					}	

}

extern void apic_ack_int();

void task_msgqueue_push(syscall_msg_t *in){
		task *i = task_find_by_tid(in->dest);
		if(i == 0){
			return; //TODO: good error handling
		}
		i->msg_queue[i->msg_queue_head++] = in;
		i->msg_queue_head &= 0b1111; //wrap around at 16
		while(i->msg_queue_tail == i->msg_queue_head){
			task_yield();
		}
}
syscall_msg_t * task_msgqueue_pop(void){
		curr_task->msg_queue_tail &= 0b1111;
		while(curr_task->msg_queue_tail == curr_task->msg_queue_head){
			task_yield();
		}

		return curr_task->msg_queue[(curr_task->msg_queue_tail++)];
}
void task_scheduler(){
		scheduler_started = 1;
        while(1){

                if(curr_task == 0 && tasks == 0) continue; //continue while curr task is 0


				                curr_task = curr_task->next;

				
				
					if(curr_task->state == T_DEAD){
								task_cleanup_zombie();
								curr_task = tasks;
							
					}else{

                
                task_set_tss((unsigned long)curr_task->krnl_stack_base + TASK_STACK_SZ-512-sizeof(task_int_sframe));
				page_switch_tab(curr_task->page_tab);
                task_save_and_change_krnl_state(&scheduler_state, curr_task->krnl_state);
                
				page_switch_krnl_tab();
				}
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

syscall_msg_t * task_nmsg(pid_t dest, pid_t src, syscall_msg_type_t t,size_t dat_sz){
	syscall_msg_t *ret = k_obj_alloc(sizeof(syscall_msg_t) + dat_sz);
	ret->dest = dest;
	ret->src = src;
	mem_set(ret->spec_dat, 0,dat_sz);
	return ret;
}

void task_exit(syscall_child_died_type_t code){


	
	page_switch_krnl_tab();
	syscall_msg_t *exit_msg = task_nmsg(curr_task->creator, curr_task->tid, MSG_CHILD_DIED, sizeof(syscall_child_died_type_t));
	*((syscall_child_died_type_t*)&exit_msg->spec_dat) = code;
	task_msgqueue_push(exit_msg);
		
	curr_task->state = T_DEAD; //notify scheduler that body of task needs to be clean up 

	while(1){
		task_yield(); //yield loop just incase the scheduler didnt free the task
	}
}

void task_yield(){


        if(curr_task == 0 ){
                return;
        }
        if(scheduler_state == 0){
                return;
        }
        if(!scheduler_started){
			return ;
		} 
        

		
        task_save_and_change_krnl_state(&curr_task->krnl_state, scheduler_state);
		
	
}

void task_enter_krnl(){
        task_in_krnl = 1;
}
void task_exit_krnl(){
        task_in_krnl = 0;
}
