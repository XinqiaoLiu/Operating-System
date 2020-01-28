#include "system_call.h"
#include "types.h"
#include "file_system.h"
#include "rtc.h"


int process_status[process_number] = {0}; 
int current_process_idx = 0;
int total_process = 0; 
/*
 * operation_table_setup
 * DESCRIPTION: setup of operation table
 * INPUTS: None
 * OUTPUTS: None
 * RETURN VALUE: None
 */
void operation_table_setup(){
	// stdin
	stdin_table.open = &terminal_open;
	stdin_table.close = &terminal_close;
	stdin_table.read = &terminal_read;
	stdin_table.write = &invalid_write;
	// stdout
	stdout_table.open = &terminal_open;
	stdout_table.close = &terminal_close;
	stdout_table.read = &invalid_read;
	stdout_table.write = &terminal_write;
	// rtc
	rtc_table.open = &rtc_open;
	rtc_table.read = &rtc_read;
	rtc_table.write = &rtc_write;
	rtc_table.close = &rtc_close;
	// directory
	dir_table.open = &open_directory;
	dir_table.read = &read_directory;
	dir_table.write = &write_directory;
	dir_table.close = &close_directory;
	// file
	file_table.open = &file_open;
	file_table.read = &file_read;
	file_table.write = &file_write;
	file_table.close = &file_close;

}


/* halt
 * DESCRIPTION: halt the executable file
 * INPUTS: uint8_t status
 * OUTPUTS: None
 * RETURN VALUE: 0 when success
 */
int32_t halt(uint8_t status){
	int i;
	pcb_t * pcb_ptr = (pcb_t*)(user_program_base_addr - (current_process_idx+1)*pcb_inc);
	process_status[current_process_idx] = 0;
	current_process_idx = pcb_ptr->parent_id;
	total_process--;
	//TO DO: turn off the flags, call close
	for(i=fd_user_start; i<=fd_max; i++){
		pcb_ptr->file_descriptor_table[i].operation_table_ptr=NULL;
		pcb_ptr->file_descriptor_table[i].inode=-1;
		pcb_ptr->file_descriptor_table[i].file_position=0;
		pcb_ptr->file_descriptor_table[i].flags=0;
	}
	// map the memory to the parent program
	if(process_status[0]==0){
		execute((const uint8_t*)"shell");
		total_process = 1;
	}
	
	//TO DO: disable, enable, flush tlb, change paging asm
	
	
	
	map_new_program(user_virtual,user_program_base_addr + current_process_idx*four_mb);
	
	
	asm volatile(
	"movl %%cr3, %%eax;"
	"movl %%eax, %%cr3; "
	:
	:
	:"eax"
	);
	
	tss.esp0 = user_program_base_addr - (pcb_ptr->parent_id+1)*pcb_inc - cntx_swch_offset;
	tss.ss0 = KERNEL_DS;
	//update esp ebp to parent
	asm volatile(
		"movl %0, %%esp\n"
		"movl %1, %%ebp\n"
		"movl %2, %%eax;"
		:
		:"r"(pcb_ptr->parent_esp), "r"(pcb_ptr->parent_ebp),"r"((uint32_t)status)
		:"%eax"
	);
	
	asm volatile(
		"leave;"
		"ret;"
	);
	return 0;
}

/* execute
 * DESCRIPTION: execute the file
 * INPUTS: const uint8_t* command
 * OUTPUTS: None
 * RETURN VALUE: 0 when success/ -1 when fail
 */
int32_t execute(const uint8_t* command){
	
	int i;
	int j;
	int len;
	int arg_idx;
	int process_num;
	uint8_t filename_buf[FILENAME_LEN	] = {'\0'};
	uint8_t command_buf[KB_BUFFER_LEN] = {'\0'};
	uint8_t file_info_buf[magic_number_len];
	uint32_t entry;
	
	for(i = 0; i < FILENAME_LEN	;i++){
		if(command[i] == ' ')
			break;
		filename_buf[i] = command[i];
		if(command[i] == '\0')
			break;
	}
	arg_idx = i+1;
	j = i + 1;
	//TO DO:
	//parse the args
	for(arg_idx = i+1;arg_idx<KB_BUFFER_LEN;arg_idx++){
		if(command[arg_idx]==' ' || command[arg_idx] == '\0'){
			len = arg_idx-j;
			break;
		}
		command_buf[arg_idx-j] = command[arg_idx];
	}
	
	//Execuatable check
	dentry_t dentry;
	//check filename validity
	if(read_dentry_by_name(filename_buf,&dentry)!=0){
		printf("execute, cannot find filename. ");
		return -1;
	}
	//check file type
	if(dentry.file_type!=REGULAR_FILE){
		printf("execute, flie type not executable");
		return -1;
	}
	
	//check file ELF first four magic number
	//memory address of the entry point from where the process starts executing
	read_data(dentry.inode_num,0,file_info_buf,magic_number_len);
	if(file_info_buf[0]!=DEL || file_info_buf[1]!=E || file_info_buf[two]!=L || file_info_buf[three]!= F){
		printf("execute, ELF error");
		return -1;
	}
	//copy the entry point
	read_data(dentry.inode_num,entry_point_start_dix,(uint8_t*)&entry,entry_point_len);
	//check if process array is full
	if(total_process == process_number-1){
		printf("limit process number reached");
	}
	
	
	
	//find the process number
	for(i = 0; i<process_number; i++){
		if(process_status[i]==0){
			process_status[i] = 1;
			process_num = i;
			
			total_process++;
			break;
		}
	}
	
	
	
	//paging
	map_new_program(user_virtual,user_program_base_addr + process_num*four_mb);
	
	
	
	//TO DO: flush TLB??
	asm volatile(
	"movl %%cr3, %%eax; "
	"movl %%eax, %%cr3; "
	:
	:
	:"eax"
	);
	
	//load program 
	read_data(dentry.inode_num,0,(uint8_t*)user_virtual,four_mb);
	
	//create pcb
	pcb_t * pcb_ptr = (pcb_t*)(user_program_base_addr - (process_num+1)*pcb_inc);
	pcb_ptr->process_id = process_num;
	pcb_ptr->parent_id = current_process_idx;
	
	//update cuurent process idx
	current_process_idx = process_num;
	
	//copy the command args into pcb
	for(i=0; i<KB_BUFFER_LEN; i++){
		if(command_buf[i] == '\0' || command_buf[i] == ' '){
			break;
		}
		pcb_ptr->command_arg[i] = command_buf[i];
	}
	//update command info
	pcb_ptr->command_len = len;
	if(len==0){
		pcb_ptr->command_flag = 0;
	}
	else{
		pcb_ptr->command_flag = 1;
	}
	
	
	
	//set file descriptor for new process
	//TO DO: initialize file_descriptor : is that needed?
	file_descriptor_init(pcb_ptr->file_descriptor_table);
	
	//initialize stdin
	(pcb_ptr->file_descriptor_table[0]).flags = 1;
	(pcb_ptr->file_descriptor_table[0]).operation_table_ptr = &stdin_table;
	
	//initialize stdout
	(pcb_ptr->file_descriptor_table[1]).flags = 1;
	(pcb_ptr->file_descriptor_table[1]).operation_table_ptr = &stdout_table;
	
	//context switch
	tss.esp0 = user_program_base_addr - (process_num+1)*pcb_inc - cntx_swch_offset;
	tss.ss0 = KERNEL_DS;
	
	
	//set up parent process info
	asm volatile(
	"movl %%ebp, %%eax;"
	: "=a" (pcb_ptr->parent_ebp)
	);
	asm volatile(
	"movl %%esp, %%eax;"
	: "=a" (pcb_ptr->parent_esp)
	);
	
	asm volatile(
	
	"movl $0x2b, %%eax;"  //USER_DS
	"movw %%ax,%%es;"
	"movw %%ax,%%ds;"
	"movw %%ax,%%fs;"
	"pushl $0x2b;"         //USER_DS
	"pushl $0x83ffffc;"    //set ESP
	"pushfl;"				//EFLAGS
	"popl %%ebx;"
	"orl $0x200, %%ebx;"
	"pushl %%ebx;"
	"pushl $0x23;"			//USER_CS
	"pushl %0;"
	"iret;"
	:
	:"r"(entry)
	:"%eax","%ebx"
	);
	
	return 0;
}

/* read
 * DESCRIPTION: call read functions according to file type
 * INPUTS: int32_t fd, void* buf, int32_t nbytes
 * OUTPUTS: None
 * RETURN VALUE: 0 when success/ -1 when fail
 */
int32_t read(int32_t fd, void* buf, int32_t nbytes){
		//check fd validity
	//stdout write only????
	if(fd< fd_min || fd > fd_max || buf==NULL || nbytes<0 ){
		//printf("write, bad args\n");
		return -1;
	}
	//find current pcb
	pcb_t * pcb_ptr = (pcb_t*)(user_program_base_addr - (current_process_idx+1)*pcb_inc);
	//check if current file flag is on
	if(pcb_ptr->file_descriptor_table[fd].flags != 1){
		//printf("fd not open\n");
		return -1;
	}
	
	return (*(pcb_ptr->file_descriptor_table[fd].operation_table_ptr->read))(fd,buf,nbytes);
}
	
/* write
 * DESCRIPTION: call write functions according to file type
 * INPUTS: int32_t fd, const void* buf, int32_t nbytes
 * OUTPUTS: None
 * RETURN VALUE: 0 when success/ -1 when fail
 */
int32_t write(int32_t fd, const void* buf,int32_t nbytes){
	//check fd validity
	if(fd< fd_min || fd > fd_max || buf==NULL || nbytes<0 ){
		printf("write, bad args\n");
		return -1;
	}
	//find current pcb
	pcb_t * pcb_ptr = (pcb_t*)(user_program_base_addr - (current_process_idx+1)*pcb_inc);
	//check if current file flag is on
	if(pcb_ptr->file_descriptor_table[fd].flags != 1){
		//printf("fd not open\n");
		return -1;
	}
	
	return (*(pcb_ptr->file_descriptor_table[fd].operation_table_ptr->write))(fd,buf,nbytes);
}

/* open
 * DESCRIPTION: open the file
 * INPUTS: const uint8_t* filename
 * OUTPUTS: None
 * RETURN VALUE: 0 when success/ -1 when fail
 */
int32_t open(const uint8_t* filename){
	int i;
	dentry_t dentry;
	
	//find the file by filename and check the validity
	
	if(read_dentry_by_name(filename,&dentry)==-1){
		return -1;
	}
	
	//find the current process pcb and allocate the file descriptor
	pcb_t* pcb_ptr = (pcb_t*)(user_program_base_addr - (current_process_idx+1)*pcb_inc);
	//find the unused file descriptor 
	for(i = 0; i <= fd_max; i++){
		if((pcb_ptr->file_descriptor_table[i]).flags==0)
			break;
	}
	if(i==fd_max+1){
		printf("cannot open more files, file descriptor table is full\n");
		return -1;
	}
	
	//classify by filetype
	//rtc
	if(dentry.file_type==0){
		//file operation table pointer
		(pcb_ptr->file_descriptor_table[i]).operation_table_ptr = &rtc_table;
		//inode
		(pcb_ptr->file_descriptor_table[i]).inode = 0;
		
		//file position
		//TO DO:
		//DO I REALLY NEED THIS FOR RTC FILE?
		
		
	}
	//directory
	else if(dentry.file_type==1){
		//file operation table pointer
		(pcb_ptr->file_descriptor_table[i]).operation_table_ptr = &dir_table;
		//inode
		(pcb_ptr->file_descriptor_table[i]).inode = 0;
		//file position
		//TO DO:
		//DO I REALLY NEED THIS FOR RTC FILE?
		
		
	}
	//regular file
	else if(dentry.file_type==REGULAR_FILE){
		//file operation table pointer
		(pcb_ptr->file_descriptor_table[i]).operation_table_ptr = &file_table;
		//inode
		(pcb_ptr->file_descriptor_table[i]).inode = dentry.inode_num;
		//file position
		
	}
	else{
		//invalid file type
		printf("open, invalid file type\n");
		return -1;
	}
	
	//initialize the file position to read
	(pcb_ptr->file_descriptor_table[i]).file_position = 0;
	//set the flag to be in use
	(pcb_ptr->file_descriptor_table[i]).flags = 1;
	return i;
}

/* close
 * DESCRIPTION: close the file
 * INPUTS: int32_t fd
 * OUTPUTS: None
 * RETURN VALUE: 0 when success/ -1 when fail
 */
int32_t close(int32_t fd){
	if(fd < fd_user_start || fd > fd_max){
		return -1;
	}
	pcb_t * pcb_ptr = (pcb_t*)(user_program_base_addr - (current_process_idx+1)*pcb_inc);
	
	if(pcb_ptr->file_descriptor_table[fd].flags == 0){
		return -1;
	}
	// close files
	pcb_ptr->file_descriptor_table[fd].flags = 0;
	// return success
	return 0;
}

/* getargs
 * DESCRIPTION: read the program's command line arguments into a user-level buf
 * INPUTS: uint8_t* buf, int32_t nbytes
 * OUTPUTS: None
 * RETURN VALUE: 0 when success/ -1 when fail
 */
int32_t getargs(uint8_t* buf, int32_t nbytes){
	int i;
	pcb_t* pcb_ptr = (pcb_t*)(user_program_base_addr - (current_process_idx+1)*pcb_inc);
	if(pcb_ptr->command_flag==0){
		printf("no command line arguments received\n");
		return -1;
	}
	if(pcb_ptr->command_len > nbytes){
		printf("command length does not fit the buffer\n");
		return -1;
	}
	//copy the command into the user level buffer
	for(i=0; i < nbytes; i++){
		buf[i] = pcb_ptr->command_arg[i];
	}
	return 0;
}

/* vidmap
 * DESCRIPTION:maps to the txt to video memory
 * INPUTS: uint8_t** screen_start
 * OUTPUTS: None
 * RETURN VALUE: 0 when success/ -1 when fail
 */
int32_t vidmap(uint8_t** screen_start){
	if((uint32_t)screen_start<virtual_user_program_start ||(uint32_t)screen_start>virtual_user_program_end)
		return -1;
	map_video_mem();
	*screen_start = (uint8_t*)virtual_user_program_end;
	return virtual_user_program_end;
}
/* set_handler
 * DESCRIPTION: not defined yet
 * INPUTS:  int32_t signum
			void* handler_address
 * OUTPUTS: NONE
 * RETURN VALUE: 0 when success/ -1 when fail
 */
int32_t set_handler(int32_t signum, void* handler_address){
	return -1;
}
/* sigreturn
 * DESCRIPTION: not defined yet
 * INPUTS:  NONE
 * OUTPUTS: NONE
 * RETURN VALUE: 0 when success/ -1 when fail
 */
 int32_t sigreturn(void){
	 return -1;
 }
/* invalid_write
 * DESCRIPTION: print warning for invalid write
 * INPUTS: int32_t fd, const void* buf,int32_t nbytes
 * OUTPUTS: None
 * RETURN VALUE: -1 when fail
 */
int32_t invalid_write(int32_t fd, const void* buf,int32_t nbytes){
	printf("invalid write");
	return -1;
}

/* invalid_write
 * DESCRIPTION: print warning for invalid read
 * INPUTS: int32_t fd, void* buf,int32_t nbytes
 * OUTPUTS: None
 * RETURN VALUE: -1 when fail
 */
int32_t invalid_read(int32_t fd, void* buf,int32_t nbytes){
	printf("invalid read");
	return -1;
}

/* file_descriptor_init
 * DESCRIPTION: initialize file descriptor
 * INPUTS: descriptor_t* descriptor_table
 * OUTPUTS: None
 * RETURN VALUE: None
 */
void file_descriptor_init(descriptor_t* descriptor_table){
	int i;
	for(i=0;i<fd_max;i++){
		descriptor_table[i].inode = 0;
		descriptor_table[i].file_position = 0;
		descriptor_table[i].flags = 0;
	}
}
