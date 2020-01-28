#ifndef SYSTEM_CALL_H
#define SYSTEM_CALL_H

#include "file_system.h"
#include "types.h"
#include "paging.h"
#include "kb.h"
#include "x86_desc.h"

#define process_number	6
#define user_program_base_addr	0x800000
#define pcb_inc			0x2000
#define fd_max			7
#define fd_min			0
#define user_virtual	0x08048000
#define fd_user_start	2
#define four_mb			0x400000
#define virtual_user_program_start	0x08000000
#define virtual_user_program_end	0x08400000
#define entry_point_start_dix	24
#define entry_point_len	4
#define DEL	0x7f
#define E	0x45
#define L	0x4c
#define F	0x46
#define magic_number_len 	4
#define cntx_swch_offset	4
#define two	2
#define three	3
#define max_buffer 128
#define FD_length 	8
int current_process_idx;



typedef struct file_operation_table {
	int (*open)(const uint8_t* filename);
	int (*read)(int32_t fd, void* buf, int32_t nbytes);
	int (*write)(int32_t fd, const void* buf, int32_t nbytes);
	int (*close)(int32_t fd);
} operation_t;

typedef struct file_descriptor {
	operation_t* operation_table_ptr;
	uint32_t inode;
	uint32_t file_position;
	uint32_t flags;
} descriptor_t;

typedef struct process_control_block {
	descriptor_t file_descriptor_table[FD_length];
	uint32_t process_id;
	uint32_t parent_ebp;
	uint32_t parent_esp;
	uint32_t parent_id;
	uint32_t command_arg[max_buffer];
	uint32_t command_flag;
	uint32_t command_len;
	
	
} pcb_t;

operation_t stdin_table;
operation_t stdout_table;
operation_t rtc_table;
operation_t dir_table;
operation_t file_table;

void operation_table_setup();
int32_t invalid_write(int32_t fd, const void* buf,int32_t nbytes);	
int32_t invalid_read(int32_t fd, void* buf,int32_t nbytes);
void file_descriptor_init(descriptor_t* descriptor_table);
    
int32_t halt(uint8_t status);
int32_t execute(const uint8_t* command);
int32_t read(int32_t fd, void* buf, int32_t nbytes);
int32_t write(int32_t fd, const void* buf, int32_t nbytes);
int32_t open(const uint8_t* filename);
int32_t close(int32_t fd);
int32_t getargs(uint8_t* buf, int32_t nbytes);
int32_t vidmap(uint8_t** screen_start);
int32_t set_handler(int32_t signum, void* handler_address);
 int32_t sigreturn(void);
#endif 

