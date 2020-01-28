#include "rtc.h"

#define data_port 	0x71
#define rw_port		0x70
#define reg_A		0x8A
#define reg_B 		0x8B
#define reg_C_nmi	0x8C
#define reg_C		0x0C
#define rtc_irq 	8
#define rtc_master	2
#define def_freq	0xF
#define max_freq	0x0F
#define lower4bit	0xF0
#define bit6		0x40
#define RTC_RAM		0x20
#define rtc_flag_idx0 0
#define rtc_flag_idx1 1
#define rtc_flag_idx2 2
#define max_rate 	15
#define min_rate 	2
#define double_ 	2
/*
 * set_reg_C
 *  Description: Select register C to make the interrupt can happen again
 *  Input: none
 *  Output: none
 * 	Reference : https://wiki.osdev.org/RTC#Programming_the_RTC "Interrupt and Register C"
 */
void set_reg_C() {
	outb(reg_C, rw_port);	// select register C
	inb(data_port);			// just throw away contents
}

volatile int rtc_flag[max_terminal] = {0,0,0};
/* init_rtc
 * Description: Initialize rtc with periodic interrupt of default freq
 * Input: none
 * Output: none
 * Reference: https://wiki.osdev.org/RTC#Programming_the_RTC "Turning on IRQ8"
 */
void init_rtc(){
	cli();
	unsigned char prev;
	outb(reg_A,rw_port);
	outb(RTC_RAM,data_port);
	
	outb(reg_B, rw_port);	// writes a byte from register B to a port 0x70 
	prev = inb(data_port);	// read the current value 
	outb(reg_B, rw_port);
	outb(bit6 | prev, data_port); //write the prev value with 0x71
	change_rate(def_freq);
	enable_irq(rtc_master);	//enable the master that owns IRQ8
	enable_irq(rtc_irq);	//enable IRQ8
		
	sti();
	
	
}

/* change_rate()
 * Description: Initialize rtc with periodic interrupt of default freq
 * Input: none
 * Output: none
 * Reference: https://wiki.osdev.org/RTC#Programming_the_RTC "Changing Interrupt Rate"
 */

void change_rate(uint8_t rate){
	cli();
	char prev;

	rate &= max_freq;	// 2 < rate < 15
	
	outb(reg_A, rw_port);	// set index to register A, disable NMI
	prev = inb(data_port);	// get init value
	outb(reg_A, rw_port);	// reset index
	// write the prev value with 0x40, and turns on Bit 6 of Reg B
	outb( (prev & lower4bit) | rate, data_port);	
	
	sti();	
}

int print_flag;		//flag for printing
/* rtc_handler()
 * Description: Initialize rtc with periodic interrupt of default freq
 * Input: none
 * Output: none
 */
void rtc_handler(){
	cli();
	
	outb(reg_C_nmi, rw_port);	
	inb(data_port);
	uint8_t one = '1';
	if(print_flag == 1){
	putc_with_line_change(one);	// print according to freq
	}
	rtc_flag[rtc_flag_idx0] = 1;
	rtc_flag[rtc_flag_idx1] = 1;
	rtc_flag[rtc_flag_idx2] = 1;
	set_reg_C();
	send_eoi(rtc_irq);		//end of interrupt
	sti();
}

/* rtc_open
 * Description: Initialize RTC to default freq
 * Input: const uint8_t* filename
 * Output: none
 * Return value: 0 when success
 */
int32_t rtc_open(const uint8_t* filename){
	change_rate(def_freq);		//init rate to default 
	return 0;
}

/* rtc_close
 * Description: Do nothing if RTC is not virtual
 * Input: int32_t fd
 * Output: none
 * Return value: 0 when success
 */
int32_t rtc_close(int32_t fd){
	return 0;
}

/* rtc_read
 * Description: Do nothing if RTC is not virtual
 * Input: int32_t fd, void* buf, int32_t nbytes
 * Output: none
 * Return value: 0 when success, -1 when fail
 */
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes){
	if(buf == NULL){
		return -1;		//invalid
	}
	
	rtc_flag[0] = 0;
	while(rtc_flag[0] != 1);
	
	return 0;	
}

/* rtc_write
 * Description: enable frequency change of RTC
 * Input: int32_t fd, const void* buf, int32_t nbytes
 * Output: none
 * Return value: 0 when success, -1 when fail
 */
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes){
	int32_t* buffer;
	buffer = (int32_t*)buf;
	if(buffer == NULL)
		return -1;		//unable to write
	int32_t freq = buffer[0];		//get frequency from buf
	int32_t rate_setting = max_rate;	// set rate to 15
	int32_t rate = min_rate;

	/* converting freq to rate */
	while(rate != freq){
		rate_setting -= 1;
		rate = rate * double_;			// double the rate
	}
	change_rate(rate_setting);	
	
	return nbytes;
}



