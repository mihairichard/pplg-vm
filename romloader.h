#pragma once

extern unsigned char PRG;
extern unsigned char CHR;

extern unsigned char MAPPER;

extern int OS_MIRROR;
extern int FS_MIRROR;
extern int TRAINER;
extern int SRAM;
extern int MIRRORING;
extern int VRAM;

int analyze_header(unsigned char* data, unsigned int size);
int load_rom(unsigned char* data, unsigned int size);
