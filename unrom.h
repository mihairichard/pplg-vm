/*
 * unrom.h - NES Mapper 2: UNROM
 */

void
unrom_switch_prg(int bank)
{
	int prg_size;
	unsigned int address;

	address = 0x8000;
	prg_size = 16384;

	memcpy(memory + address, romcache + 16 + (bank * prg_size), prg_size);
}

void
unrom_access(unsigned int address,unsigned char data)
{
	if(address > 0x7fff && address < 0x10000) {
		unrom_switch_prg(data);
	}
}
