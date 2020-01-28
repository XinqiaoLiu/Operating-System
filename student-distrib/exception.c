#include "exception.h"
#include "lib.h"
//These are exceptions defined by intel
//The exceptions are in the idt and when trigerred will give a "blue screen" using the infinit while loop
void division_by_zero(){
	printf("EXCEPTION : DIVISION BY ZERO!");
	while(1);
}

void single_step(){
	printf("EXCEPTION: SINGLE STEP!");
	while(1);
}

void non_maskable(){
	printf("EXCEPTION: NON MASKABLE(NMI)!");
	while(1);
}

void breakpoint(){
	printf("EXCEPTION: BREAKPOINT!");
	while(1);
}

void overflow_trap(){
	printf("EXCEPTION: OVERFLOW!");
	while(1);
}

void bound_range_exceeded(){
	printf("EXCEPTION: BOUNDS RANGE EXCEEDED!");
	while(1);
}

void invalid_opcode(){
	printf("EXCEPTION: INVALID OPCODE!");
	while(1);
}

void coprocessor_not_available(){
	printf("EXCEPTION: COPROCESSOR NOT AVAILABLE!");
	while(1);
}

void double_fault_exception(){
	printf("EXCEPTION: DOUBLE FAULT!");
	while(1);
}

void coprocessor_segment_overrun(){
	printf("EXCEPTION: COPROCESSOR SEGMENT OVERRUN!");
	while(1);
}

void invalid_task_state_segment(){
	printf("EXCEPTION: INVALID TASK STATE SEGMENT!");
	while(1);
}

void segment_not_present(){
	printf("EXCEPTION: SEGMENT NOT PRESENT!");
	while(1);
}

void stack_exception(){
	printf("EXCEPTION: STACK FAULT!");
	while(1);
}

void general_protection_exception(){
	//cli();
	printf("EXCEPTION: GENERAL PROTECTION FAULT!");
	//while(1);
}

//will give the address of page fault 
void page_fault(){
	printf("EXCEPTION: PAGE FAULT!");
	int fault_addr;
	asm volatile(
	"movl %%cr2,%%eax;"
	"movl %%eax,%0;"
	:"=g"(fault_addr)
	:
	:"eax"
		);
	printf("%x",fault_addr);
	while(1);
}

void reserved(){
	printf("EXCEPTION: RESERVED!");
	while(1);
}

void math_fault(){
	printf("EXCEPTION: MATH FAULT!");
	while(1);
}

void alignment_check(){
	printf("EXCEPTION: ALIGNMENT CHECK!");
	while(1);
}

void machine_check(){
	printf("EXCEPTION: MACHINE CHECK!");
	while(1);
}

void floating_point(){
	printf("EXCEPTION: SIMD FLOATING POINT EXCEPTION!");
	while(1);
}

void virtualization(){
	printf("EXCEPTION: VIRTUALIZATION EXCEPTION!");
	while(1);
}

void control_protection(){
	printf("EXCEPTION: CONTROL PROTECTION EXCEPTION!");
	while(1);
}

