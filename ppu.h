#pragma once

extern unsigned int ppu_control1;
extern unsigned int ppu_control2;
extern unsigned int ppu_addr;
extern unsigned int ppu_addr_h;
extern unsigned int ppu_addr_tmp;
extern unsigned int ppu_status;
extern unsigned int ppu_status_tmp;
extern unsigned int ppu_bgscr_f;

extern int current_scanline;

extern unsigned int sprite_address;

extern unsigned int loopyT;
extern unsigned int loopyV;
extern unsigned int loopyX;

void show_gfxcache();
void write_ppu_memory(unsigned int address,unsigned char data);
void render_background(int scanline);
void render_sprites();
void check_sprite_hit(int scanline);
