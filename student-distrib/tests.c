#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "rtc.h"
#include "file_system.h"
#include "system_call.h"

#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

// add more tests here
//test paging
int paging_test(){
	int b;
	int* a = NULL;
	b = *a;
	return PASS;
}
/* Checkpoint 2 tests */
int open_rtc(){
	rtc_open(0);	
	return PASS;
}

int write_rtc(){
	//int rate;
	//rate = 2;  
	//rate = 5;
	//rate = 8;
	//rate = 10;
	//rate = 16;	//not valid	   
	//rtc_write(0,(int*)&rate,4);
	print_flag = 1; 
	return PASS;
}

int read_rtc(){
	rtc_read(0,0,0);
	print_flag = 1;
	return PASS;
}


int print_by_name(){
	screen_clear();
	//print_file_by_name((uint8_t*)"frame0.txt");
	//print_file_by_name((uint8_t*)"frame1.txt");
	print_file_by_name((uint8_t*)"verylargetextwithverylongname.txt");
	return PASS;
}

int list_file_name(){
	screen_clear();
	list_directory();
	return PASS;
}

int print_file_idx(){
	screen_clear();
	int index = 10;
	print_file_by_idx(index);
	return PASS;
}

/* Checkpoint 3 tests */
int exe_test(){
	screen_clear();
	execute((const uint8_t*)"shell");
	return PASS;
}
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */

/* Test suite entry point */
void launch_tests(){
	//TEST_OUTPUT("idt_test", idt_test());
	// launch your tests here
	//TEST_OUTPUT("rtc_open", open_rtc());
	//TEST_OUTPUT("rtc_write", write_rtc());
	//TEST_OUTPUT("rtc_read", read_rtc());
	//TEST_OUTPUT("paging_test", paging_test());
	//TEST_OUTPUT("print_by_name", print_by_name());
	//TEST_OUTPUT("list_file_name", list_file_name());
	//TEST_OUTPUT("print_file_idx", print_file_idx());
	TEST_OUTPUT("exe_test", exe_test());
	//list_directory();
	
}
