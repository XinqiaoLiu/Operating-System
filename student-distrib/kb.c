//xliu152

#include "kb.h"
#include "i8259.h"
#include "lib.h"
//#include "system_call.h"


int flag; //indicate the scan code layer
int shift_cnt;//the number of shift being pressed
int ctrl_flag; //1 is on, 0 is off

volatile int enter_flag = 0;

unsigned char scan_code[SCANCODE_ROW][SCANCODE_COL] = {{0,0,'1','2',
								'3','4','5','6',
								'7','8','9','0',
								'-','=',0,0,
								'q','w','e','r',
								't','y','u','i',
								'o','p','[',']',
								'\n',0,'a','s',
								'd','f','g','h',
								'j','k','l',';',
								'\'','`',0,'\\',
								'z','x','c','v',
								'b','n','m',',',
								'.','/',0,'*',
								0,' ',0,0,
								0,0,0,0,
								0,0,0,0,
								0,0,0,'7',
								'8','9','-','4',
								'5','6','+','1',
								'2','3','0','.'
								},
								{0,0,'!','@',
								'#','$','%','^',
								'&','*','(',')',
								'_','+',0,0,
								'Q','W','E','R',
								'T','Y','U','I',
								'O','P','{','}',
								'\n',0,'A','S',
								'D','F','G','H',
								'J','K','L',':',
								'"','~',0,'|',
								'Z','X','C','V',
								'B','N','M','<',
								'>','?',0,'*',
								0,' ',0,0,
								0,0,0,0,
								0,0,0,0,
								0,0,0,0, 
                               	0,0,'-',0,							
								0,0,'+',0,
								0,0,0,0
								},
								{0,0,'1','2',
								'3','4','5','6',
								'7','8','9','0',
								'-','=',0,0,
								'Q','W','E','R',
								'T','Y','U','I',
								'O','P','[',']',
								'\n',0,'A','S',
								'D','F','G','H',
								'J','K','L',';',
								'\'','`',0,'\\',
								'Z','X','C','V',
								'B','N','M',',',
								'.','/',0,'*',
								0,' ',0,0,
								0,0,0,0,
								0,0,0,0,
								0,0,0,'7',
								'8','9','-','4',
								'5','6','+','1',
								'2','3','0','.'
								},
								{0,0,'!','@',
								'#','$','%','^',
								'&','*','(',')',
								'_','+',0,0,
								'q','w','e','r',
								't','y','u','i',
								'o','p','{','}',
								'\n',0,'a','s',
								'd','f','g','h',
								'j','k','l',':',
								'"','~',0,'|',
								'z','x','c','v',
								'b','n','m','<',
								'>','?',0,'*',
								0,' ',0,0,
								0,0,0,0,
								0,0,0,0,
								0,0,0,0,
								0,0,'-',0,
								0,0,'+',0,
								0,0,0,0}};
/* void kb_init(void);
 * Inputs: void
 * Return Value: none
 * Function: initialize the keyboard: set up irq enable; clean input buffer and counter; clean scancode flag and shift counter */
void kb_init(){
	int i;
	//enable int
	enable_irq(KB_IRQ_NUM);
	//ckean kb buffer
	kb_buffer_cnt = 0;
	for(i=0;i<KB_BUFFER_LEN;i++){
		kb_buffer[i] = '\0';
	}
	//default to be regular scan code
	flag = REGULAR_LAYER;
	shift_cnt = 0;
	//enable cursor
	enable_cursor(CURSOR_START,CURSOR_END);
	
}

/* void enable_cursor(uint8_t cursor_start, uint8_t cursor_end);
 * Inputs: void
 * Return Value: none
 * Function: enable the cursor, called by kb_init 
 * code from osdev*/
void enable_cursor(uint8_t cursor_start, uint8_t cursor_end)
{
	outb(CURSOR_START_REG,CRTC_D_REG_ONE);
	outb((inb(CRTC_D_REG_TWO) & CURSOR_REG_ONE) | cursor_start,CRTC_D_REG_TWO);
 
	outb(CURSOR_REG_TWO,CRTC_D_REG_ONE);
	outb((inb(CRTC_D_REG_TWO) & CURSOR_REG_THREE) | cursor_end,CRTC_D_REG_TWO);
}

/* void kb_irq_handler(void);
 * Inputs: void
 * Return Value: none
 * Function: called when keyboard gives input, print the regular letters and deal with functionality when texting*/
void kb_handler(){
	//unsigned char status_reg = 0x20;
	unsigned char output;
	
	//cli();
	
	output = inb(KB_DATA_IN);
	/*if(inb(0x64)==0x02){
		send_eoi(1);
		sti();
	}*/
	//release
	if(output>SCANCODE_COL){
			
		//shift
		if(output == RIGHT_SHIFT_R || output == LEFT_SHIFT_R){
			//if capslock is on
			if(flag==SHIFT_CAP_LAYER){
				//if only one shift was on
				if(shift_cnt==1)
					flag = CAP_LAYER;
				//if two shifts on, no need to change flag
			}
			//if capslock is not on, flag is 1
			else{
				//if only one shift was on
				if(shift_cnt==1)
					flag = REGULAR_LAYER;
				//if two shifts on, no need to change flag
			}
				
			shift_cnt--;
		}
			
		//ctrl
		if(output == CTRL_L_R)
			ctrl_flag = 0;
			
		send_eoi(KB_IRQ_NUM);
		sti();
		return;
		
	}
		
	//pressed
	//shift
	if(output == RIGHT_SHIFT_P || output == LEFT_SHIFT_P){ 
		//if capslock is also on
		if(flag==CAP_LAYER)
			flag = SHIFT_CAP_LAYER;
		//if capslock is not on, flag is 0 or 1
		else
			flag = SHIFT_LAYER;
		shift_cnt++;
	}
	//caplock
	if(output == CAPSLOCK_P){
			
		//if capslock is off ,shift is also on
		if(flag==SHIFT_LAYER)
			flag = SHIFT_CAP_LAYER;
		//if capslock is off,shift is not on, flag is 0 
		else if(flag==REGULAR_LAYER)
			flag = CAP_LAYER;
		//if capslock is on, shift is on
		else if(flag==SHIFT_CAP_LAYER)
			flag = SHIFT_LAYER;
		//if capslock is on, shift is off, flag is 2
		else
			flag =REGULAR_LAYER;
	}
		
	//ctrl + l
	//crtl being pressed, set crtl flag
	if(output == CTRL_L_P)
		ctrl_flag = 1;
	//L is pressed at same time
	if(output == L_P && ctrl_flag==1){
		screen_clear();
		update_cursor();
		kb_buffer_cnt = 0;
		t_buffer_cnt = 0;
		
		send_eoi(KB_IRQ_NUM);
		sti();
		return;
	}
		
	//backspace and update cursor
	if(output == BACKSPACE){
		
		if(kb_buffer_cnt!=0){
			backspace();
			update_cursor();
			kb_buffer_cnt--;
			t_buffer_cnt--;
		}
	}

	//enter
	if(output==ENTER){
		//responding to the terminal input
		enter_flag = 1;
		
		putc_with_line_change(scan_code[flag][output]);
		update_cursor();
		//echo function
		/*
		if(kb_buffer_cnt!=0)
			enter_echo();
		*/
		kb_buffer_cnt = 0;
		t_buffer_cnt = 0;
		send_eoi(KB_IRQ_NUM);
		return;
	}
		
	//nothing should be printed	
	if(scan_code[flag][output]==0){
		send_eoi(KB_IRQ_NUM);
		sti();
		return;
	}
	 //check buffer limit
	if(kb_buffer_cnt>=KB_BUFFER_LEN){
		send_eoi(KB_IRQ_NUM);
		sti();
		return;
	}
	putc_with_line_change(scan_code[flag][output]);
	kb_buffer[kb_buffer_cnt] = scan_code[flag][output];
	kb_buffer_cnt++;
	update_cursor();
	//terminal_read();
	//terminal_write();
	send_eoi(KB_IRQ_NUM);
	//sti();
}
/* int terminal_open(void);
 * Inputs: void
 * Return Value: 0 if success
 * Function: terminal initialize*/
int32_t terminal_open(){
	int i;
	//clean buffer
	//TO DO: no need for this
	for(i=0;i<T_BUFFER_LEN;i++){
		t_buffer[i] = 0;
	}
	t_buffer_cnt = 0;
	
	return 0;
}
/* int terminal_close(void);
 * Inputs: void
 * Return Value: 0 if success
 * Function: close the terminal*/
int32_t terminal_close(){
	return 0;
}
/* int terminal_read(void);
 * Inputs: void
 * Return Value: number of bytes read
 * Function: read the kb buffer and copy to t buffer
int terminal_read(){
	int i; 
	int num_b_read = kb_buffer_cnt-t_buffer_cnt;
	for(i = t_buffer_cnt;i<kb_buffer_cnt;i++){
		t_buffer[i] = kb_buffer[i];
		
	}
	
	return num_b_read*B_PER_CHAR;
}*/
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes){
	int i;
	//if(nbytes>128)
	//	nbytes = 128;
	char* t_buf = (char*)buf;
	while(1){
		if(enter_flag){
			for(i=0;i<nbytes;i++){
				t_buf[i] = kb_buffer[i];
			}
			//clear k buffer
			for(i = 0;i<128;i++){
				kb_buffer[i] = '\0';
			}
			kb_buffer_cnt = 0;
			enter_flag = 0;
			return nbytes;
		}
	}
		
	
}

/* int terminal_write(void);
 * Inputs: void
 * Return Value: number of bytes write
 * Function: print t buffer to terminal screen
int terminal_write(){
	int i;
	
	for(i = t_buffer_cnt;i<kb_buffer_cnt;i++){
		putc_with_line_change(t_buffer[i]);
		update_cursor();
        t_buffer_cnt++;
	}
	return t_buffer_cnt*B_PER_CHAR;
}*/
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes){
	int i;
	//if(nbytes>128)
	//	nbytes = 128;
	
	char* t_buf = (char*)buf;
	for(i = 0;i<nbytes;i++){
		putc_with_line_change(t_buf[i]);
		update_cursor();
		
		}
	return nbytes;
}
/* void enter_echo(void);
 * Inputs: void
 * Return Value: void
 * Function: respond to user typed command , called by typing enter key, echo the command*/	
void enter_echo(){
	int i;
	uint8_t babbling_cmd[T_BABBLING_LEN] = {'w','h','a','t',' ','a','r','e',' ','y','o','u',' ','b','a','b','b','l','i','n','g',' ','a','b','o','u','t',':'};
	//print response
	for(i=0;i<T_BABBLING_LEN;i++){
		putc_with_line_change(babbling_cmd[i]);
	}
	//echo user command
	for(i=0;i<t_buffer_cnt;i++){
		putc_with_line_change(t_buffer[i]);
	}
	//new line
	putc_with_line_change('\n');
	update_cursor();
}



