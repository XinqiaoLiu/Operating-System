#ifndef EXCEPTION_H
#define EXCEPTION_H

#include "lib.h"

void division_by_zero();
void single_step();
void non_maskable();
void breakpoint();
void overflow_trap();
void bound_range_exceeded();
void invalid_opcode();
void coprocessor_not_available();
void double_fault_exception();
void coprocessor_segment_overrun();
void invalid_task_state_segment();
void segment_not_present();
void stack_exception();
void general_protection_exception();
void page_fault();
void reserved();
void math_fault();
void alignment_check();
void machine_check();
void floating_point();
void virtualization();
void control_protection();

#endif

