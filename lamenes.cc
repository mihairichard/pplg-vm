//#define DEBUG
//#define DISASSAMBLE
//#define PAD_DEBUG
//#define MAPPER_DEBUG

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "lame6502/lame6502.h"
#include "lame6502/disas.h"
#include "lame6502/instructions.h"

#include "macros.h"
#include "lamenes.h"
#include "romloader.h"
#include "ppu.h"
#include "input.h"

#include "system/display.h"
#include "system/sleep.h"

/* included mappers */
#include "mappers/mmc1.h"	// 1
#include "mappers/unrom.h"	// 2
#include "mappers/cnrom.h"	// 3
#include "mappers/mmc3.h"	// 4

/* static rom, avoid loading from file */
#include "mario_nes.h"

char romfn[256];

/* cache the rom in memory to access the data quickly */
unsigned char *romcache;

unsigned char *ppu_memory;
unsigned char *sprite_memory;

unsigned int pad1_data;
int pad1_readcount = 0;

int start_int;
int vblank_int;
int vblank_cycle_timeout;
int scanline_refresh;

int ntsc = 0;
int pal = 1;

int CPU_is_running = 1;
int pause_emulation = 0;

int height;
int width;

int sdl_screen_height;
int sdl_screen_width;

int enable_background = 1;
int enable_sprites = 1;

int fullscreen = 0;
int scale = 1;

int frameskip = 0;
int skipframe = 0;

int sdl_delay = 10;

long romlen;

/*
 * memory read handler
 */
unsigned char memory_read(unsigned int address) {
	/* this is ram or rom so we can return the address */
	if(address < 0x2000 || address > 0x7FFF)
		return memory[address];

	/* the addresses between 0x2000 and 0x5000 are for input/ouput */
	if(address == 0x2002) {
		ppu_status_tmp = ppu_status;

		/* set ppu_status (D7) to 0 (vblank_on) */
		ppu_status &= 0x7F;
		write_memory(0x2002,ppu_status);

		/* set ppu_status (D6) to 0 (sprite_zero) */
		ppu_status &= 0x1F;
		write_memory(0x2002,ppu_status);

		/* reset VRAM Address Register #1 */
		ppu_bgscr_f = 0x00;

		/* reset VRAM Address Register #2 */
		ppu_addr_h = 0x00;

		/* return bits 7-4 of unmodifyed ppu_status with bits 3-0 of the ppu_addr_tmp */
		return (ppu_status_tmp & 0xE0) | (ppu_addr_tmp & 0x1F);
	}

	if(address == 0x2007) {
		tmp = ppu_addr_tmp;
		ppu_addr_tmp = ppu_addr;

		if(increment_32 == 0) {
			ppu_addr++;
		} else {
			ppu_addr += 0x20;
		}

		return ppu_memory[tmp];
	}

	/* pAPU data (sound) */
	if(address == 0x4015) {
		return memory[address];
	}

	/* joypad1 data */
	if(address == 0x4016) {
		switch(pad1_readcount) {
			case 0:
			memory[address] = pad1_A;
			pad1_readcount++;
			break;

			case 1:
			memory[address] = pad1_B;
			pad1_readcount++;
			break;

			case 2:
			memory[address] = pad1_SELECT;
			pad1_readcount++;
			break;

			case 3:
			memory[address] = pad1_START;
			pad1_readcount++;
			break;

			case 4:
			memory[address] = pad1_UP;
			pad1_readcount++;
			break;

			case 5:
			memory[address] = pad1_DOWN;
			pad1_readcount++;
			break;

			case 6:
			memory[address] = pad1_LEFT;
			pad1_readcount++;
			break;

			case 7:
			memory[address] = pad1_RIGHT;
			pad1_readcount = 0;
			break;
		}

		#ifdef PAD_DEBUG
		printf("debug [%d]: pad1_A -> %d, pad1_B -> %d, pad1_SELECT -> %d, pad1_START -> %d, pad1_UP -> %d, pad1_DOWN -> %d, pad1_LEFT -> %d, pad1_RIGHT -> %d\n",
			debug_cnt,pad1_A,pad1_B,pad1_SELECT,pad1_START,pad1_UP,pad1_DOWN,pad1_LEFT,pad1_RIGHT);

		printf("debug [%d]: pad1_readcount = %d, read joydata = %d [hex: %x]\n",debug_cnt,pad1_readcount,memory[address],memory[address]);
		#endif

		return memory[address];
	}

	if(address == 0x4017) {
		return memory[address];
	}

	return memory[address];
}

/*
 * memory write handler
 */
void
write_memory(unsigned int address,unsigned char data)
{
	/* PPU Status */
	if(address == 0x2002) {
		memory[address] = data;
		return;
	}

	/* PPU Video Memory area */
	if(address > 0x1fff && address < 0x4000) {
		write_ppu_memory(address,data);
		return;
	}

	/* Sprite DMA Register */
	if(address == 0x4014) {
		write_ppu_memory(address,data);
		return;
	}

	/* Joypad 1 */
	if(address == 0x4016) {
		memory[address] = 0x40;

		return;
	}

	/* Joypad 2 */
	if(address == 0x4017) {
		memory[address] = 0x48;
		return;
	}

	/* pAPU Sound Registers */
	if(address > 0x3fff && address < 0x4016) {
		//write_sound(address,data); not emulated yet!
		memory[address] = data;
		return;
	}

	/* SRAM Registers */
	if(address > 0x5fff && address < 0x8000) {
		memory[address] = data;
		return;
	}

	/* RAM registers */
	if(address < 0x2000) {
		memory[address] = data;
		memory[address+2048] = data; // mirror of 0-800
		memory[address+4096] = data; // mirror of 0-800
		memory[address+6144] = data; // mirror of 0-800
		return;
	}

	if(MAPPER == 1) {
		mmc1_access(address,data);
		return;
	}

	if(MAPPER == 2) {
		unrom_access(address,data);
		return;
	}

	if(MAPPER == 3) {
		cnrom_access(address,data);
		return;
	}

	if(MAPPER == 4) {
		mmc3_access(address,data);
		return;
	}

	memory[address] = data;
}

void
start_emulation()
{
	int counter = 0;

	int scanline = 0;


	while(CPU_is_running) {
//printf("[%d] 1 counter = %d / pvi = %d (counter - PAL_VBLANK_INT = %d)\n",debug_cnt,counter,PAL_VBLANK_INT + 341,(counter - (PAL_VBLANK_INT + 341)));
		CPU_execute(start_int);

		/* set ppu_status D7 to 1 and enter vblank */
		ppu_status |= 0x80;
		write_memory(0x2002,ppu_status);

		counter += CPU_execute(12); // needed for some roms

		#ifdef DEBUG
		if(debug_cnt > show_debug_cnt) {
			printf("debug [%d] --- entering VBLANK! ---\n",debug_cnt);
			printf("ppu_status: %x\n",ppu_status);
		}
		#endif

		if(exec_nmi_on_vblank) {
			#ifdef DEBUG
			if(debug_cnt > show_debug_cnt) {
				printf("[%d] vblank = on\n",debug_cnt);
			}
			#endif

			counter += NMI(counter);
		}

		counter += CPU_execute(vblank_cycle_timeout);

		#ifdef DEBUG
		if(debug_cnt > show_debug_cnt) {
			printf("[%d] --- leaving VBLANK! ---\n",debug_cnt);
		}
		#endif

		/* vblank ends (ppu_status D7) is set to 0, sprite_zero (ppu_status D6) is set to 0 */
		ppu_status &= 0x3F;
	
		/* and write to mem */
		write_memory(0x2002,ppu_status);

		loopyV = loopyT;

		if(skipframe > frameskip)
			skipframe = 0;

		for(scanline = 0; scanline < 240; scanline++) {
			if(!sprite_zero) {
				check_sprite_hit(scanline);
			}

			render_background(scanline);

			counter += CPU_execute(scanline_refresh);

			if(mmc3_irq_enable == 1) {
				if(scanline == mmc3_irq_counter) {
					//printf("[%d] mmc3_irq_counter = %d\n",debug_cnt,mmc3_irq_counter);
					IRQ(counter);
					mmc3_irq_counter--;

					//break;
				}
			}
		}

		render_sprites();

		#ifdef DEBUG
		if(debug_cnt > show_debug_cnt) {
			printf("[%d]: *** UPDATING SCREEN ***\n",debug_cnt);
		}
		#endif

		if(skipframe == 0)
			display_update();

		sleep_ms(sdl_delay);

		skipframe++;
	}
}

void
reset_emulation()
{
	printf("[*] resetting emulation...\n");

	if(load_rom(get_mario_rom(), get_mario_rom_size())) {
		free(sprite_memory);
		free(ppu_memory);
		free(memory);
		free(romcache);
		exit(1);
	}

	if(MAPPER == 4)
		mmc3_reset();

	CPU_reset();

	reset_input();

	start_emulation();
}

void
quit_emulation()
{
	/* free all memory */
	free(sprite_memory);
	free(ppu_memory);
	free(memory);
	free(romcache);

	printf("[!] quiting LameNES!\n\n");

	exit(0);
}

int lamenes_main(){
	int chr_check_result;

	int i;

	/* cpu speed */
	int NTSC_SPEED = 1789725;
	int PAL_SPEED = 1773447;

	/* vblank int */
	int NTSC_VBLANK_INT = NTSC_SPEED / 60;
	int PAL_VBLANK_INT = PAL_SPEED / 50;

	/* scanline refresh (hblank)*/
	int NTSC_SCANLINE_REFRESH = NTSC_VBLANK_INT / 261;
	int PAL_SCANLINE_REFRESH = PAL_VBLANK_INT / 313;

	/* vblank int cycle timeout */
	int NTSC_VBLANK_CYCLE_TIMEOUT = (261-224) * NTSC_VBLANK_INT / 261;
	int PAL_VBLANK_CYCLE_TIMEOUT = (313-240) * PAL_VBLANK_INT / 313;

	/* 64k main memory */
	memory = (unsigned char *)malloc(65536);

	/* 16k video memory */
	ppu_memory = (unsigned char *)malloc(16384);

	/* 256b sprite memory */
	sprite_memory = (unsigned char *)malloc(256);

	if(analyze_header(get_mario_rom(), get_mario_rom_size()) == 1) {
		free(sprite_memory);
		free(ppu_memory);
		free(memory);
		exit(1);
	}

	/* rom cache memory */
	romcache = (unsigned char *)malloc(romlen);

	printf("[*] PRG = %x, CHR = %x, OS_MIRROR = %d, FS_MIRROR = %d, TRAINER = %d"
		", SRAM = %d, MIRRORING = %d\n",
		PRG,CHR,OS_MIRROR, FS_MIRROR,TRAINER,SRAM,MIRRORING);

	printf("[*] mapper: %d found!\n",MAPPER);

	if(load_rom(get_mario_rom(), get_mario_rom_size())) {
		free(sprite_memory);
		free(ppu_memory);
		free(memory);
		free(romcache);
		exit(1);
	}

	if(MAPPER == 4)
		mmc3_reset();

	if(pal == 1) {
		height = 240;
		width = 256;
	}

	if(ntsc == 1) {
		height = 224;
		width = 256;
	}

	sdl_screen_height = height * scale;
	sdl_screen_width = width * scale;

	if(pal == 1) {
		printf("[*] PAL_SPEED: %d\n",PAL_SPEED);
		printf("[*] PAL_VBLANK_INT: %d\n",PAL_VBLANK_INT);
		printf("[*] PAL_SCANLINE_REFRESH: %d\n",PAL_SCANLINE_REFRESH);
		printf("[*] PAL_VBLANK_CYCLE_TIMEOUT: %d\n",PAL_VBLANK_CYCLE_TIMEOUT);
		printf("[*] height * PAL_SCANLINE_REFRESH: %d\n",(height * PAL_SCANLINE_REFRESH) + PAL_VBLANK_CYCLE_TIMEOUT + 341);
	}

	if(ntsc == 1) {
		printf("[*] NTSC_SPEED: %d\n",NTSC_SPEED);
		printf("[*] NTSC_VBLANK_INT: %d\n",NTSC_VBLANK_INT);
		printf("[*] NTSC_SCANLINE_REFRESH: %d\n",NTSC_SCANLINE_REFRESH);
		printf("[*] NTSC_VBLANK_CYCLE_TIMEOUT: %d\n",NTSC_VBLANK_CYCLE_TIMEOUT);
		printf("[*] height * NTSC_SCANLINE_REFRESH: %d\n",(height * NTSC_SCANLINE_REFRESH) + NTSC_VBLANK_CYCLE_TIMEOUT + 341);
	}

	printf("[*] setting screen resolution to: %dx%d\n",sdl_screen_width,sdl_screen_height);

	DisplayType display_type = DisplayTypePAL;
	if (ntsc == 1) {
		display_type = DisplayTypeNTSC;
	}

	display_init(sdl_screen_width, sdl_screen_height, display_type, fullscreen == 1);

	printf("[*] resetting cpu...\n");

	/*
	 * first reset the cpu at poweron
	 */
	CPU_reset();

	/*
	 * reset joystick
	 */
	reset_input();

	printf("[*] LameNES starting emulation!\n");

	if(pal == 1) {
		start_int = 341;
		vblank_int = PAL_VBLANK_INT;
		vblank_cycle_timeout = PAL_VBLANK_CYCLE_TIMEOUT;
		scanline_refresh = PAL_SCANLINE_REFRESH;
	}

	if(ntsc == 1) {
		start_int = 325;
		vblank_int = NTSC_VBLANK_INT;
		vblank_cycle_timeout = NTSC_VBLANK_CYCLE_TIMEOUT;
		scanline_refresh = NTSC_SCANLINE_REFRESH;
	}

	while(1) {
		if(!pause_emulation)
			start_emulation();
	}

	/* never reached */
	return(0);
}
