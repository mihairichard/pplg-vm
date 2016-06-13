/*
 * lame6502.c - 6502 cpu functions and registers
 */

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "instructions.h"

#include "../lamenes.h"

/*
 * system ram
 */
unsigned char *memory;

/*
 * 6502 internal registers
 */

/* 16 bits wide */
unsigned int program_counter;

/* 8 bits wide */
unsigned char stack_pointer;
unsigned char status_register;
unsigned char x_reg;		/* index reg */
unsigned char y_reg;		/* index reg */
unsigned char accumulator;	/* core */

unsigned int addr;
unsigned int tmp, tmp2, tmp3;

#ifdef DISASSAMBLE
int disassemble = 1;
#else
int disassemble = 0;
#endif

/* status_register flags */
int zero_flag;		/* this is set if the last operation returned a result of zero */

int sign_flag;		/* 
		 	* (N flag) this is simply a reflection of the highest bit of
		 	* the result of the last operation. A number with it's high bit
		 	* set is considered to be negative (0xff = -1); they are called
		 	* "Signed" numbers
		 	*/

int overflow_flag;	/* this is set if the last operation resulted in a sign change. */

int break_flag;		/* this is set only if the BRK instruction is executed. */

int decimal_flag;	/* if set, all Addition/Subtraction operations will be calculated using
			 * "Binary-coded Decimal"-formatted values (eg., $69 = 69), and will 
			 * return a BCD value. Decimal mode is unavaliable on the NES' 6502, 
			 * which is just as well.
			 */

int interrupt_flag;	/* if set, IRQ interrupts will be disabled */

int carry_flag;		/* this holds the "carry" out of the most significant bit of the last
			 * addition/subtraction/shift/rotate instruction. If an addition
			 * produces a result greater than 255, this is set. If a subtraction
			 * produces a result less than zero, this is cleared (Subtract
			 * operations use the opposite of the Carry (1=0, 0=1)).
			 */

int cycle_count;

/* used in LameNES */
int startdebugger = 0;
int breakpoint = 0;
int hit_break = 0;
int stop_at_debug_cnt = 0;

void
update_status_register()
{
	status_register = ((sign_flag ? 0x80 : 0) | (zero_flag ? 0x02 : 0) | (carry_flag ? 0x01 : 0) |
			(interrupt_flag ? 0x04 : 0) | (decimal_flag ? 0x08 : 0) | (overflow_flag ? 0x40 : 0) |
			(break_flag ? 0x10 : 0) | 0x20);
}


/* 
 * Maskable Interrupt
 * irq vector address = 0xfffe
 */
int
IRQ(int cycles)
{
	PUSH_ST((program_counter & 0xff00) >> 8);
	PUSH_ST(program_counter & 0xff);
	PUSH_ST(GET_SR());
	break_flag = 0;
	interrupt_flag = 1;
	program_counter = (memory[0xffff] << 8) | memory[0xfffe];
	return cycles -= 7;
}

/* Non-Maskable Interrupt
 * nmi vector address = 0xfffa
 */
int
NMI(int cycles)
{
	PUSH_ST((program_counter & 0xff00) >> 8);
	PUSH_ST(program_counter & 0xff);
	PUSH_ST(GET_SR());
	break_flag = 0;
	interrupt_flag = 1;
	program_counter = (memory[0xfffb] << 8) | memory[0xfffa];

	if(program_counter == breakpoint) {
		printf("breakpoint reached!\n");
			hit_break = 1;
			disassemble = 1;
	}

	return cycles -= 7;
}

/*
 * CPU Reset
 * reset vector address (init) = $fffc
 */
void
CPU_reset(void)
{
	status_register = 0x20;

	zero_flag = 1;
	sign_flag = 0;
	overflow_flag = 0;
	break_flag = 0;
	decimal_flag = 0;
	interrupt_flag = 0;
	carry_flag = 0;

	stack_pointer = 0xff;

	program_counter = (memory[0xfffd] << 8) | memory[0xfffc];

	accumulator=x_reg=y_reg=0;
}

/*
 * Timing and execution
 */
int
CPU_execute(int cycles) {
	unsigned char opcode;

	cycle_count = cycles;
	do {
		update_status_register();

		opcode=memory[program_counter++];

		switch(opcode) {
			#include "opcodes.h"
		}

	} while(cycle_count > 0);

	return cycles - cycle_count;
}
