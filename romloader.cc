#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lame6502/lame6502.h"

#include "lamenes.h"
#include "romloader.h"

/* pointers to the nes headers */
unsigned char *header;
unsigned char PRG; /* rom mem */
unsigned char CHR; /* vrom mem */

unsigned char MAPPER;

int RCB;
int OS_MIRROR = 0;
int FS_MIRROR;
int TRAINER;
int SRAM;
int MIRRORING;

char title[128];

int
analyze_header(unsigned char* data, unsigned int size)
{
	int i;

	/*
	 * nes header is 15 bytes
	 * nes internal memory (6502 = 65536 bytes (64K))
	 */
	header = (unsigned char *)malloc(15);

	romlen = size; 

	/* read the first 15 bytes of the rom */
	memcpy(header, data, 15);

	printf("[*] analyzing rom header...\n");

	/* ines rom header must be: "NES\n" (HEX: 4E 45 53 1A), else exit */
	if((header[0] != 'N') || (header[1] != 'E') || (header[2] != 'S') || (header[3] != 0x1A)) {
		printf("[!] incorrect rom header\n");
		free(header);
		return(1);
	}

	/* ines rom header 8-15 must be 0x00 or 0xFF or games wil not work */
	for(i = 8; i < 15; i++) {
		if((header[i] != 0x00) && (header[i] != 0xFF)) {
			printf("[!] notice rom header 8-15 is not empty\n");
		}
	}

	printf("[*] detected rom size: %ldkb\n",romlen / 1024);

	/* detect PRG */
	printf("[*] %d x 16kb pages (PRG 0x%x) found!\n",header[4],header[4]);
	PRG = header[4];

	/* detect CHR */
	if(header[5] == 0x00) {
		printf("[*] no CHR (CHR 0x00) found (imbedded)!\n");
	} else {
		printf("[*] %d x 8kb pages (CHR 0x%x) found!\n",header[5],header[5]);
	}

	CHR = header[5];

	/* detect mapper lower bits */
	MAPPER = (header[6] >> 4);
	MAPPER |= (header[7] & 0xf0);

	/* detect mirroring, sram status,
	 * trainer status and four screen mirroring status.
	 */
	RCB = (header[6] - ((header[6] >> 4) << 4));

	switch(RCB) {
		case 0x00:
		/* horizontal mirroring only */
		MIRRORING = 0;
		SRAM = 0;
		TRAINER = 0;
		FS_MIRROR = 0;
		break;

		case 0x01:
		/* vertical mirroring only */
		MIRRORING = 1;
		SRAM = 0;
		TRAINER = 0;
		FS_MIRROR = 0;
		break;

		case 0x02:
		/* horizontal mirroring and sram enabled */
		MIRRORING = 0;
		SRAM = 1;
		TRAINER = 0;
		FS_MIRROR = 0;
		break;

		case 0x03:
		/* vertical mirroring and sram enabled */
		MIRRORING = 1;
		SRAM = 1;
		TRAINER = 0;
		FS_MIRROR = 0;
		break;

		case 0x04:
		/* horizontal mirroring and trainer on */
		MIRRORING = 0;
		SRAM = 0;
		TRAINER = 1;
		FS_MIRROR = 0;
		break;

		case 0x05:
		/* vertical mirroring and trainer on */
		MIRRORING = 1;
		SRAM = 0;
		TRAINER = 1;
		FS_MIRROR = 0;
		break;

		case 0x06:
		/* horizontal mirroring, sram enabled and trainer on */
		MIRRORING = 0;
		SRAM = 1;
		TRAINER = 1;
		FS_MIRROR = 0;
		break;

		case 0x07:
		/* vertical mirroring, sram enabled and trainer on */
		MIRRORING = 1;
		SRAM = 1;
		TRAINER = 1;
		FS_MIRROR = 0;
		break;

		case 0x08:
		/* horizontal mirroring and four screen vram on */
		MIRRORING = 0;
		SRAM = 0;
		TRAINER = 0;
		FS_MIRROR = 1;
		break;

		case 0x09:
		/* vertical mirroring and four screen vram on */
		MIRRORING = 1;
		SRAM = 0;
		TRAINER = 0;
		FS_MIRROR = 1;
		break;

		case 0x0A:
		/* horizontal mirroring, sram enabled and four screen vram on */
		MIRRORING = 0;
		SRAM = 1;
		TRAINER = 0;
		FS_MIRROR = 1;
		break;

		case 0x0B:
		/* vertical mirroring, sram enabled and four screen vram on */
		MIRRORING = 1;
		SRAM = 1;
		TRAINER = 0;
		FS_MIRROR = 1;
		break;

		case 0x0C:
		/* horizontal mirroring, trainer on and four screen vram on */
		MIRRORING = 0;
		SRAM = 0;
		TRAINER = 1;
		FS_MIRROR = 1;
		break;

		case 0x0D:
		/* vertical mirroring, trainer on and four screen vram on */
		MIRRORING = 1;
		SRAM = 0;
		TRAINER = 1;
		FS_MIRROR = 1;
		break;

		case 0x0E:
		/* horizontal mirroring, sram enabled, trainer on and four screen vram on */
		MIRRORING = 0;
		SRAM = 1;
		TRAINER = 1;
		FS_MIRROR = 1;
		break;

		case 0x0F:
		/* vertical mirroring, sram enabled, trainer on and four screen vram on */
		MIRRORING = 1;
		SRAM = 1;
		TRAINER = 1;
		FS_MIRROR = 1;
		break;

		default:
		printf("[!] RCB header corrupt?\n");
		break;
	}

	free(header);

	return(0);  
}

int
load_rom(unsigned char* data, unsigned int size)
{
	memcpy(romcache, data, size);

	printf("[*] mapping rom...\n");

	/* load prg data in memory */
	if(PRG == 0x01) {
		/* map 16kb in mirror mode */ 
		memcpy(memory + 0x8000, romcache + 16, 16384);
		memcpy(memory + 0xC000, romcache + 16, 16384);
	} else {
		/* map 2x 16kb the first one into 8000 and the last one into c000 */
		memcpy(memory + 0x8000, romcache + 16, 16384);
		memcpy(memory + 0xC000, romcache + 16 + ((PRG - 1) * 16384), 16384);
	}

	/* load chr data in ppu memory */
	if(CHR != 0x00) {
		memcpy(ppu_memory, romcache + 16 + (PRG * 16384), 8192);

		/* fetch title from last 128 bytes */
		memcpy(title, romcache + 16 + (PRG * 16384) + 8192, 128);
	}

	return(0);
}
