#ifndef _RTC_H
#define _RTC_H

#ifndef ASM
#include "lib.h"
#include "kb.h"
#include "types.h"
#include "i8259.h"
#include "irq.h"

#define	max_terminal 3

extern int print_flag;
extern void set_reg_C();
extern volatile int rtc_flag[max_terminal];
extern void init_rtc();
extern void rtc_handler();
extern void change_rate(uint8_t rate);

int32_t rtc_open(const uint8_t* filename);
int32_t rtc_close(int32_t fd);

int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);






#endif 
#endif 

