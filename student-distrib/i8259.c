/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

#define   MASK  0xFF
#define PIC_IRQ_NUM	15
#define MASTER_IRQ_OFFSET 8


/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */
unsigned int flags;
/* Initialize the 8259 PIC */
void i8259_init(void) {
	//maks all the data ports
	//cli_and_save(flags);
	master_mask = MASK;
	slave_mask = MASK;
	outb(master_mask, MASTER_8259_DATA_PORT);
	outb(slave_mask,SLAVE_8259_DATA_PORT);
	
	//write ICW1
	outb(ICW1,MASTER_8259_PORT);
	outb(ICW1,SLAVE_8259_PORT);
	//write ICW2
	outb(ICW2_MASTER ,MASTER_8259_DATA_PORT);
	outb(ICW2_SLAVE,SLAVE_8259_DATA_PORT);
	//write ICW3
	outb(ICW3_MASTER ,MASTER_8259_DATA_PORT);
	outb(ICW3_SLAVE,SLAVE_8259_DATA_PORT);
	//write ICW4
	outb(ICW4,MASTER_8259_DATA_PORT);
	outb(ICW4,SLAVE_8259_DATA_PORT);
	
	//restore_flags(flags);
	//sti();
}

/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num) {
	uint8_t port_mask; //master or slave data port mask to send signal
	uint16_t port; //master or slave data port to send signal
	uint8_t value; //the combination of the pic mask and requiring mask to send out
	//check irq_num validity
	if(irq_num<0||irq_num>PIC_IRQ_NUM){
		//TO DO: should print some warning 
		return;
	}
	//master irq
	if(irq_num<MASTER_IRQ_OFFSET){
		port = MASTER_8259_DATA_PORT;
		port_mask = master_mask;
		master_mask = port_mask & (~(1<<irq_num));
	}
	//slave irq
	else{
		irq_num -= MASTER_IRQ_OFFSET;
		port = SLAVE_8259_DATA_PORT;
		port_mask = slave_mask;
		slave_mask = port_mask & (~(1<<irq_num));
	}
	value = port_mask & (~(1<<irq_num));
	outb(value,port);
}

/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num) {
	uint8_t port_mask; //master or slave data port mask to send signal
	uint16_t port; //master or slave data port to send signal
	uint8_t value; //the combination of the pic mask and requiring mask to send out
	
	//check irq_num validity
	if(irq_num<0||irq_num>PIC_IRQ_NUM){
		//TO DO: should print some warning 
		return;
	}
	//master irq
	if(irq_num<MASTER_IRQ_OFFSET){
		port = MASTER_8259_DATA_PORT;
		port_mask = master_mask;
		master_mask = port_mask | (1<<irq_num);
	}
	//slave irq
	else{
		irq_num -= MASTER_IRQ_OFFSET;
		port = SLAVE_8259_DATA_PORT;
		port_mask = slave_mask;
		slave_mask = port_mask | (1<<irq_num);
	}
	value = port_mask | (1<<irq_num);
	outb(value,port);	
}

/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) {
	uint8_t value; //signal to be sent to master or slave port
	//check irq_num validity
	if(irq_num<0||irq_num>PIC_IRQ_NUM){
		//TO DO: should print some warning 
		return;
	}
	//master irq
	if(irq_num<MASTER_IRQ_OFFSET){
		value = EOI | irq_num;
		outb(value,MASTER_8259_PORT);
	}
	//slave irq
	else{
		irq_num -= MASTER_IRQ_OFFSET;
		value = EOI | irq_num;
		outb(value,SLAVE_8259_PORT);
		outb(EOI| MASTER_TO_SLAVE,MASTER_8259_PORT);
	}
}

