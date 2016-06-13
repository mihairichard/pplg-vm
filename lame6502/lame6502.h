/*
 * lame6502.h - 6502 cpu functions and registers
 */

extern unsigned char *memory;

extern unsigned int program_counter;

extern unsigned char stack_pointer;
extern unsigned char status_register;
extern unsigned char x_reg;
extern unsigned char y_reg;
extern unsigned char accumulator;

extern unsigned int addr;
extern unsigned int tmp, tmp2, tmp3;

extern int zero_flag;
extern int sign_flag;
extern int overflow_flag;
extern int break_flag;
extern int decimal_flag;
extern int interrupt_flag;
extern int carry_flag;

extern int disassemble;
extern int hit_break;
extern int startdebugger;

extern int breakpoint;
extern int stop_at_debug_cnt;
extern int stackdebug;

//unsigned char memory_read(unsigned int address);
//void write_memory(unsigned int address,unsigned char data);

int IRQ(int cycles);
int NMI(int cycles);
void CPU_reset(void);
int CPU_execute(int cycles);
