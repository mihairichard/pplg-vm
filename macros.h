#pragma once

/*
 * &0x80 = D7
 * &0x40 = D6
 * &0x20 = D5
 * &0x10 = D4
 * &0x08 = D3
 * &0x04 = D2
 * &0x02 = D1
 * &0x01 = D0
 */

/* memory[0x2000] */
#define exec_nmi_on_vblank	(ppu_control1 & 0x80) /* 1 = Generate VBlank NMI */
#define sprite_16		(ppu_control1 & 0x20) /* 1 = Sprites 8x16/8x8 */
#define background_addr_hi	(ppu_control1 & 0x10) /* 1 = BG pattern adr $0000/$1000 */
#define sprite_addr_hi		(ppu_control1 & 0x08) /* 1 = Sprite pattern adr $0000/$1000 */
#define increment_32		(ppu_control1 & 0x04) /* 1 = auto increment 1/32 */

/* memory[0x2001] */
#define sprite_on		(ppu_control2 & 0x10) /* 1 = Show sprite */
#define background_on		(ppu_control2 & 0x08) /* 1 = Show background */
#define sprite_clipping_off	(ppu_control2 & 0x04) /* 1 = 1 = No clipping */
#define background_clipping_off	(ppu_control2 & 0x02) /* 1 = 1 = No clipping */
#define monochrome_on		(ppu_control2 & 0x01) /* 1 = Display monochrome */

/* memory[0x2002] */
#define vblank_on		(ppu_status & 0x80) /* 1 = In VBlank */
#define sprite_zero		(ppu_status & 0x40) /* 1 = PPU has hit Sprite #0 */
#define scanline_sprite_count	(ppu_status & 0x20) /* 1 = More than 8 sprites on current scanline */
#define vram_write_flag		(ppu_status & 0x10) /* 1 = Writes to VRAM are ignored */
