//xliu152
#include "paging.h"
#define VIDEO_ADDR          0xB8000
#define VIDEO_PAGE_TABLE	0xB8
#define CLEAR_ENTRY		    0x2
#define FOUR_MB             0x400000
#define VIDEO_PAGE_DIR_IDX  0
#define KERNEL_PAGE_DIR_IDX	1
#define SIZE_PPT_OFFSET	    7
#define PRESENT_PPT_OFFSET  0
#define pdt_shift			22
#define user_shift			2
#define pd_shift			12
#define pd_clear			0x3ff
uint32_t page_directory[FOUR_KB] __attribute__((aligned(FOUR_KB*ENTRY_SIZE)));
uint32_t page_table[FOUR_KB] __attribute__((aligned(FOUR_KB*ENTRY_SIZE)));
uint32_t video_page_table[FOUR_KB] __attribute__((aligned(FOUR_KB*ENTRY_SIZE)));

/*
31                         11          9                                  0
page table 4kb aligned addr| availabel | G | S | 0 | A | D | W | U | R | P


*/

/* paging_init
 * DESCRIPTION: Initialize the paging function
 * INPUTS: none
 * OUTPUTS: none
 * RETURN VALUE: none
 */
void paging_init(){
	int i;
	//initialize the page directory entries
	//initialize the page table entry
	for(i = 0; i < FOUR_KB ; i++){
		page_directory[i] = CLEAR_ENTRY;
		page_table[i] = CLEAR_ENTRY;
	}
	
	//present the first page directory entry for vidoe memory
    page_directory[VIDEO_PAGE_DIR_IDX] |= 1<<PRESENT_PPT_OFFSET;
	
	//set page table addtr for video memory
	page_directory[VIDEO_PAGE_DIR_IDX] |= (uint32_t)page_table;
	
	
	//present the second page directory entry for kernel
	page_directory[KERNEL_PAGE_DIR_IDX] |= 1<<PRESENT_PPT_OFFSET;
	//set page size for kernel : 1 means 4MB
	page_directory[KERNEL_PAGE_DIR_IDX] |= 1<<SIZE_PPT_OFFSET;
	//set page addr for kernel, since it is identity mapping, just the addr of the kernel
	page_directory[KERNEL_PAGE_DIR_IDX] |= FOUR_MB;
	
	//set page for video memory present
    page_table[VIDEO_PAGE_TABLE] |= 1<<PRESENT_PPT_OFFSET; 
	page_table[VIDEO_PAGE_TABLE] |= VIDEO_ADDR;
	//turn on paging
	// does the order matter?
	asm volatile(
	"movl $page_directory, %%eax;"
	"movl %%eax, %%cr3; "
	"movl %%cr4, %%eax; "
    "orl  $0x00000010, %%eax; "
    "movl %%eax, %%cr4; "
	"movl %%cr0, %%eax; "
	"orl  $0x80000000, %%eax; "
    "movl %%eax, %%cr0; "
	:
	:
	:"eax"
	);
	
}

/* map_new_program
 * DESCRIPTION: turn on paging for the new program
 * INPUTS: uint32_t virtual, uint32_t physical
 * OUTPUTS: none
 * RETURN VALUE: none
 */
void map_new_program(uint32_t virtual, uint32_t physical){
	//parse the vm 
	uint32_t pdt_idx  = virtual >> pdt_shift;
	
	//present the PDE for new program
	page_directory[pdt_idx] = CLEAR_ENTRY;
	page_directory[pdt_idx] |= 1<<PRESENT_PPT_OFFSET;
	//set the user bit 
	page_directory[pdt_idx] |= 1<<user_shift;
	//set page size for new program : 1 means 4MB
	page_directory[pdt_idx] |= 1<<SIZE_PPT_OFFSET;
	page_directory[pdt_idx] |= physical; //8mb for now
	
	//turn on paging for the new program
	
}


/* map_video_mem
 * DESCRIPTION: set page for vidmap present
 * INPUTS: none
 * OUTPUTS: none
 * RETURN VALUE: none
 */
void map_video_mem(){
	uint32_t screen_start = virtual_user_program_end;
	//parse the vm 
	uint32_t pdt_idx  = screen_start >> pdt_shift;
	uint32_t pt_idx = (screen_start >> pd_shift)&pd_clear;
	//present the PDE for video memory
	page_directory[pdt_idx] = CLEAR_ENTRY;
	page_directory[pdt_idx] |= 1<<PRESENT_PPT_OFFSET;
	//set the user bit 
	page_directory[pdt_idx] |= 1<<user_shift;
	//set page table addr for video memory
	page_directory[pdt_idx] |= (uint32_t)video_page_table;
	//set page for video memory present
	video_page_table[pt_idx] = CLEAR_ENTRY;
    video_page_table[pt_idx] |= 1<<PRESENT_PPT_OFFSET; 
	//set the user bit 
	video_page_table[pt_idx] |= 1<<user_shift;
	video_page_table[pt_idx] |= VIDEO_ADDR;
	
	//flush TLB
	asm volatile(
	"movl %%cr3, %%eax; "
	"movl %%eax, %%cr3; "
	:
	:
	:"eax"
	);
}







