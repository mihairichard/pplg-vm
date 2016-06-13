#pragma once

extern char romfn[256];

extern unsigned char *romcache;

extern unsigned char *ppu_memory;
extern unsigned char *sprite_memory;

extern unsigned int pad1_data;

extern int pal;
extern int ntsc;

extern int CPU_is_running;
extern int pause_emulation;

extern int scale;
extern int skipframe;

extern int height;
extern int width;

extern int sdl_screen_height;
extern int sdl_screen_width;

extern int enable_background;
extern int enable_sprites;

extern long romlen;

unsigned char memory_read(unsigned int address);
void write_memory(unsigned int address,unsigned char data);

int emulator_main();
