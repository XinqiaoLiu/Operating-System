#define ASM 1
#include "x86_desc.h"

.global kb_irq_handler
.global rtc_irq_handler
kb_irq_handler:
	cli
	
	pushl %eax
	pushl %ebx
	pushl %ecx
	pushl %edx
	pushl %esp
	pushl %ebp
	pushl %esi
	pushl %edi
	pushfl
	
	call kb_handler
	
	popfl
	popl %edi
	popl %esi
	popl %ebp
	popl %esp
	popl %edx
	popl %ecx
	popl %ebx
	popl %eax
	
	sti
	iret
	
rtc_irq_handler:	
    cli

	pushl %eax
	pushl %ebx
	pushl %ecx
	pushl %edx
	pushl %esp
	pushl %ebp
	pushl %esi
	pushl %edi
	pushfl
	
	#call set_reg_C
	call rtc_handler
	
	popfl
	popl %edi
	popl %esi
	popl %ebp
	popl %esp
	popl %edx
	popl %ecx
	popl %ebx
	popl %eax
	
	sti
	iret

