#ifndef _PAGING_H
#define _PAGING_H
#define FOUR_KB 	1024
#define ENTRY_SIZE	4	
#include "types.h"
#define virtual_user_program_end	0x08400000
extern uint32_t page_directory[FOUR_KB] __attribute__((aligned(FOUR_KB*ENTRY_SIZE)));
extern uint32_t page_table[FOUR_KB] __attribute__((aligned(FOUR_KB*ENTRY_SIZE)));

void paging_init();
void map_new_program(uint32_t virtual, uint32_t physical);
void map_video_mem();
#endif

