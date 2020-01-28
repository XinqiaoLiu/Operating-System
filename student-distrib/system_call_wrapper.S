#define ASM 1
#include "x86_desc.h"

.global system_call_wrapper

sys_call_table:
		.long 0x0 
		.long halt
		.long execute
		.long read
		.long write
		.long open
		.long close
		.long getargs
		.long vidmap
		.long set_handler
		.long sigreturn
system_call_wrapper:
	
	
	#store regs
	pushl %esp
	pushl %ebp
	pushl %esi
	pushl %edi
	pushfl
	
	#store eax
	pushl %eax
	
	#push args
	pushl %edx
	pushl %ecx
	pushl %ebx
	
	#check system call number
	cmpl $10, %eax
	ja badarg
	cmpl $1, %eax
	jb badarg

nobadsys:	
	call *sys_call_table(,%eax,4)
	
	movl %eax,12(%esp)
restore_all:
	#pop args
	popl %ebx
	popl %ecx
	popl %edx
	
	#pop ret val:eax
	popl %eax
	
	#pop args
	popfl
	popl %edi
	popl %esi
	popl %ebp
	popl %esp
	
	
	iret


badarg:	
	movl $-1, 12(%esp)
	jmp restore_all	
	
	

