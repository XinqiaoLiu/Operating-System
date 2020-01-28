#ifndef _KB_H
#define _KB_H
#include "types.h"
#define KB_BUFFER_LEN	128
#define RIGHT_SHIFT_P	0x36
#define LEFT_SHIFT_P	0x2A
#define RIGHT_SHIFT_R	0xB6 
#define LEFT_SHIFT_R	0xAA
#define CAPSLOCK_P		0x3A
#define CTRL_L_P		0x1D
#define CTRL_L_R		0x9D			
#define L_P				0x26
#define BACKSPACE       0x0E
#define ENTER			0x1C
#define CURSOR_START    12
#define CURSOR_END      13
#define SCANCODE_ROW	4
#define SCANCODE_COL	84
#define KB_IRQ_NUM		1
#define CURSOR_START_REG 0x0A
#define CRTC_D_REG_ONE	0x3D4
#define CRTC_D_REG_TWO	0x3D5
#define CURSOR_REG_ONE  0xC0
#define CURSOR_REG_TWO	0x0B
#define CURSOR_REG_THREE	0xE0
#define KB_DATA_IN	    0x60
#define REGULAR_LAYER		0
#define	SHIFT_LAYER			1
#define CAP_LAYER			2
#define SHIFT_CAP_LAYER		3
#define B_PER_CHAR			4
int kb_buffer_cnt;
unsigned char kb_buffer[KB_BUFFER_LEN];
void kb_init();
void kb_handler();
void enable_cursor(uint8_t cursor_start, uint8_t cursor_end);
void enter_echo();

#define T_BUFFER_LEN     128
#define T_BABBLING_LEN	   28
int t_buffer_cnt;
unsigned char t_buffer[T_BUFFER_LEN];
int32_t terminal_open();
int32_t terminal_close();
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);


#endif

