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

	#ifdef MAPPER_DEBUG
	printf("[%d] prg_switch: reading from %d (offset: %x) into mem address: %x (bank: %d)\n",
		debug_cnt,16 + (bank * prg_size),16 + (bank * prg_size),address,bank);
	#endif

	memcpy(memory + address, romcache + 16 + (bank * prg_size), prg_size);
}

void
unrom_access(unsigned int address,unsigned char data)
{
	#ifdef MAPPER_DEBUG
	printf("[%d] unrom access: 0x%x\n",debug_cnt,address);
	#endif

	if(address > 0x7fff && address < 0x10000) {
		unrom_switch_prg(data);
	}
}
