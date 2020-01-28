//xliu152
#include "x86_desc.h"
#include "idt.h"
#include "exception.h"
#include "kb.h"
#include "irq.h"
#include "rtc.h"  
#include "system_call_wrapper.h"

#define SYSTEM_CALL 		0x80
#define SYSTEM_CALL_DPL 	3 
#define HAEDWARE_INT_DPL	0
#define	EXCEPTION_DPL		0

#define DIVISION 			0x00
#define SINGLE 				0x01
#define NMI 				0x02
#define BP 					0x03
#define OVERFLOW 			0x04
#define BOUNDS 				0x05
#define OPCODE 				0x06
#define NA_COP 				0x07
#define DOUBLE_FAULT 		0x08
#define SEG_OVR_COP 		0x09
#define INVLD_TSS 			0x0A
#define SEG_NO_PRESENT 		0x0B
#define STACK_FAULT 		0x0C
#define GP 					0x0D
#define PAGE_FAULT 			0x0E
#define RESERVED			0x0F
#define MATH				0x10
#define ALIGN				0x11
#define MACHINE				0x12
#define FLOAT				0x13
#define VIRT				0x14
#define CTL_PROT			0x15
#define RTC_IDX				40
#define EXCP_LEN            32
#define KB_INT_INDEX		33



//initialize the interrupt descriptor table
//input  : none
//output : none
//first 32 entries are intel defined exceptions 
//the following wntries untill 256 are intterupts and system calls to be done

void idt_init(){
	int index;
	//intel defined exceptions
	for(index = 0; index < EXCP_LEN; index++){
		idt[index].seg_selector = KERNEL_CS;
		idt[index].reserved0 = 0;
		idt[index].reserved1 = 1;
		idt[index].reserved2 = 1;
		idt[index].reserved3 = 1;
		idt[index].reserved4 = 0;
		idt[index].size = 1;
		idt[index].dpl = 0;
		idt[index].present = 1;
	}

	for(index = EXCP_LEN; index < NUM_VEC;index++){ 
		idt[index].seg_selector = KERNEL_CS;
		idt[index].reserved0 = 0;
		idt[index].reserved1 = 1;
		idt[index].reserved2 = 1;
		idt[index].reserved3 = 0;
		idt[index].reserved4 = 0;
		idt[index].size = 1;
		idt[index].dpl = 0;
		idt[index].present = 1;		
		
		if( index == SYSTEM_CALL){
			idt[index].dpl = SYSTEM_CALL_DPL;
			idt[index].reserved3 = 1;
		}		
	
	}


	SET_IDT_ENTRY(idt[DIVISION], division_by_zero);
	SET_IDT_ENTRY(idt[SINGLE], single_step);
	SET_IDT_ENTRY(idt[NMI], non_maskable);
	SET_IDT_ENTRY(idt[BP], breakpoint);
	SET_IDT_ENTRY(idt[OVERFLOW], overflow_trap);
	SET_IDT_ENTRY(idt[BOUNDS], bound_range_exceeded);
	SET_IDT_ENTRY(idt[OPCODE], invalid_opcode);
	SET_IDT_ENTRY(idt[NA_COP], coprocessor_not_available);
	SET_IDT_ENTRY(idt[DOUBLE_FAULT], double_fault_exception);
	SET_IDT_ENTRY(idt[SEG_OVR_COP], coprocessor_segment_overrun);
	SET_IDT_ENTRY(idt[INVLD_TSS], invalid_task_state_segment);
	SET_IDT_ENTRY(idt[SEG_NO_PRESENT], segment_not_present);
	SET_IDT_ENTRY(idt[STACK_FAULT], stack_exception);
	SET_IDT_ENTRY(idt[GP], general_protection_exception);
	SET_IDT_ENTRY(idt[PAGE_FAULT], page_fault);
	SET_IDT_ENTRY(idt[RESERVED], reserved);
	SET_IDT_ENTRY(idt[MATH], math_fault);
	SET_IDT_ENTRY(idt[ALIGN], alignment_check);
	SET_IDT_ENTRY(idt[MACHINE], machine_check);
	SET_IDT_ENTRY(idt[FLOAT], floating_point);
	SET_IDT_ENTRY(idt[VIRT], virtualization);
	SET_IDT_ENTRY(idt[CTL_PROT], control_protection);
	
	//keyboard int entry
	SET_IDT_ENTRY(idt[KB_INT_INDEX],kb_irq_handler);
	SET_IDT_ENTRY(idt[RTC_IDX],rtc_irq_handler);
	//system call
	SET_IDT_ENTRY(idt[SYSTEM_CALL], system_call_wrapper);
}



