/*
 * cnrom.h - NES Mapper 3: CNROM
 */

void
cnrom_switch_chr(int bank)
{
	int prg_size;
	int chr_size;

	int chr_start;

	unsigned int address;

	address = 0x0000;

	prg_size = 16384;
	chr_size = 8192;

	chr_start = prg_size * PRG;

	#ifdef MAPPER_DEBUG
	printf("[%d] chr_switch: reading from %d (offset: %x) into ppu mem address: 0x%x (bank: %d)\n",
		debug_cnt,16 + chr_start + (bank * chr_size),16 + chr_start + (bank * chr_size),address,bank);
	#endif

	memcpy(ppu_memory + address, romcache + 16 + chr_start + (bank * chr_size), chr_size);
}

void
cnrom_access(unsigned int address,unsigned char data)
{
	#ifdef MAPPER_DEBUG
	printf("[%d] cnrom access: 0x%x\n",debug_cnt,address);
	#endif

	if(address > 0x7fff && address < 0x10000) {
		cnrom_switch_chr(data & (CHR - 1));
	}
}
