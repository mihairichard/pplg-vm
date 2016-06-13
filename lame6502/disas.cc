/*
 * disas.c - 6502 disassembler
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "lame6502.h"

int debug_cnt = 0;
int show_debug_cnt = 0;

void
disas(char *name, char *type)
{
	if(debug_cnt > show_debug_cnt) {
		printf("[%d] A:%x, P:%x, X:%x, Y:%x, S:0x0%x, addr:%x\n",
			debug_cnt-1,accumulator,status_register,x_reg,y_reg,stack_pointer+0x100,addr);

		printf("[%d] Z:%d, N:%d, O:%d, B:%d, D:%d, I:%d, C:%d\n",
			debug_cnt-1,zero_flag,sign_flag,overflow_flag,break_flag,decimal_flag,interrupt_flag,carry_flag);
	}

	if(debug_cnt > show_debug_cnt) {
		if(!strcmp(type,"NODATA")) {
			printf("[%d] executing instruction at offset 0x%x: [0x%x - %s]\n",debug_cnt-1,program_counter-1,
				memory[program_counter-1],name);

			return;
		}

		if(!strcmp(type,"ACC")) {
			printf("[%d] executing instruction at offset 0x%x: [0x%x - %s]\tA\n",debug_cnt-1,program_counter-1,
				memory[program_counter-1],name);

			return;
		}

		if(!strcmp(type,"aa")) {
			printf("[%d] executing instruction at offset 0x%x: [0x%x - %s]\t%x (%x + %d)\n",
				debug_cnt-1,program_counter-1,memory[program_counter-1],name,
				(program_counter + (signed char)memory[program_counter]) + 1,program_counter + 1,
				(signed char)memory[program_counter]);
		}

		if(!strcmp(type,"IM")) {
			printf("[%d] executing instruction at offset 0x%x: [0x%x - %s]\t#%x\n",debug_cnt-1,program_counter-1,
				memory[program_counter-1],name,memory[program_counter]);

			return;
		}

		if(!strcmp(type,"ZP")) {
			printf("[%d] executing instruction at offset 0x%x: [0x%x - %s]\t%x [mem value: %x]\n",debug_cnt-1,
				program_counter-1, memory[program_counter-1],name,memory[program_counter],
				memory[memory[program_counter]]);

			return;
		}

		if(!strcmp(type,"ZPIX")) {
			printf("[%d] executing instruction at offset 0x%x: [0x%x - %s]\t%x, X [mem value: %x]\n",debug_cnt-1,
				program_counter-1,memory[program_counter-1],name,memory[program_counter],
				memory[memory[program_counter] + x_reg]);

			return;
		}

		if(!strcmp(type,"ZPIY")) {
			printf("[%d] executing instruction at offset 0x%x: [0x%x - %s]\t%x, Y [mem value: %x]\n",debug_cnt-1,
				program_counter-1,memory[program_counter-1],name,memory[program_counter],
				memory[memory[program_counter] + y_reg]);

			return;
		}

		if(!strcmp(type,"A")) {
			printf("[%d] executing instruction at offset 0x%x: [0x%x - %s]\t%x [mem value: %x]\n",debug_cnt-1,
				program_counter-1,memory[program_counter-1],name,
				(memory[program_counter+1] << 8) | memory[program_counter],
				memory[(memory[program_counter+1] << 8) | memory[program_counter]]);

			return;
		}

		if(!strcmp(type,"AI")) {
			printf("[%d] executing instruction at offset 0x%x: [0x%x - %s]\t(%x)\n",debug_cnt-1,program_counter-1,
				memory[program_counter-1],name,(memory[program_counter+1] << 8) | memory[program_counter]);

			return;
		}

		if(!strcmp(type,"AIX")) {
			printf("[%d] executing instruction at offset 0x%x: [0x%x - %s]\t%x, X [mem value: %x]\n",debug_cnt-1,
				program_counter-1,memory[program_counter-1],name,
				(memory[program_counter+1] << 8) | memory[program_counter],
				memory[((memory[program_counter+1] << 8) | memory[program_counter]) + x_reg]);

			return;
		}

		if(!strcmp(type,"AIY")) {
			printf("[%d] executing instruction at offset 0x%x: [0x%x - %s]\t%x, Y [mem value: %x]\n",debug_cnt-1,
				program_counter-1,memory[program_counter-1],name,
				(memory[program_counter+1] << 8) | memory[program_counter],
				memory[((memory[program_counter+1] << 8) | memory[program_counter]) + y_reg]);
			return;
		}

		if(!strcmp(type,"IDI")) {
			printf("[%d] executing instruction at offset 0x%x: [0x%x - %s]\t(%x, X) [mem location: %x]\n",debug_cnt-1,
				program_counter-1,memory[program_counter-1],name,memory[program_counter],
				(memory[memory[program_counter] + x_reg + 1] << 8) | memory[memory[program_counter] + x_reg]);

			return;
		}

		if(!strcmp(type,"INI")) {
			printf("[%d] executing instruction at offset 0x%x: [0x%x - %s]\t(%x), Y [mem location: %x]\n",debug_cnt-1,
				program_counter-1,memory[program_counter-1],name,memory[program_counter],
				((memory[memory[program_counter] + 1] << 8) | memory[memory[program_counter]]) + y_reg);

			return;
		}
	}

	return;
}
