unsigned char mmc3_cmd;

int mmc3_prg_bank0 = 0;
int mmc3_prg_bank1 = 0;

int mmc3_prg_page = 0;

int mmc3_chr_xor = 0;

int mmc3_irq_counter = 0;
int mmc3_irq_latch;
int mmc3_irq_control0;
int mmc3_irq_control1;
int mmc3_irq_enable = 0;

void
mmc3_reset()
{
	memcpy(memory + 0xa000, romcache + 16, 8192);
}

void
mmc3_switch_prg(unsigned int address, int bank)
{
	int prg_size;
	prg_size = 8192;
	memcpy(memory + address, romcache + 16 + (bank * prg_size), prg_size); 
}

void
mmc3_switch_chr(unsigned int address, int bank, int pagecount)
{
	int prg_size;
	int chr_size;
	int chr_start;

	prg_size = 16384;
	chr_size = 1024;

	chr_start = prg_size * PRG;

	memcpy(ppu_memory + address, romcache + 16 + chr_start + (bank * chr_size), chr_size * pagecount); 
}

void
mmc3_access(unsigned int address,unsigned char data)
{
	switch(address) {
		case 0x8000:
		/* store command */
		mmc3_cmd = data;

		/* check for prg swapping */
		if(data & 0x40) {
			if(mmc3_prg_page != 1) {
				mmc3_switch_prg(0x8000,(PRG * 2) - 2);
				mmc3_switch_prg(0xc000,mmc3_prg_bank0);

				mmc3_prg_page = 1;
			}
		} else {
			if(mmc3_prg_page != 0) {
				mmc3_switch_prg(0xc000,(PRG * 2) - 2);
				mmc3_switch_prg(0x8000,mmc3_prg_bank0);

				mmc3_prg_page = 0;
			}
		}

		/* check for chr swapping */
		if(data & 0x80) {
			mmc3_chr_xor = 1;
		} else {
			mmc3_chr_xor = 0;
		}
		break;

		case 0x8001:

		/* exec command (bit 0-2)*/
		switch(mmc3_cmd & 0x07){
			case 0:
			if(mmc3_chr_xor == 0) {
				mmc3_switch_chr(0x0000, data, 2);
			} else {
				mmc3_switch_chr(0x1000, data, 2);
			}
			break;

			case 1:
			if(mmc3_chr_xor == 0) {
				mmc3_switch_chr(0x0800, data, 2);
			} else {
				mmc3_switch_chr(0x1800, data, 2);
			}
			break;

			case 2:
			if(mmc3_chr_xor == 0) {
				mmc3_switch_chr(0x1000, data, 1);
			} else {
				mmc3_switch_chr(0x0000, data, 1);
			}
			break;

			case 3:
			if(mmc3_chr_xor == 0) {
				mmc3_switch_chr(0x1400, data, 1);
			} else {
				mmc3_switch_chr(0x400, data, 1);
			}
			break;

			case 4:
			if(mmc3_chr_xor == 0) {
				mmc3_switch_chr(0x1800, data, 1);
			} else {
				mmc3_switch_chr(0x800, data, 1);
			}
			break;

			case 5:
			if(mmc3_chr_xor == 0) {
				mmc3_switch_chr(0x1c00, data, 1);
			} else {
				mmc3_switch_chr(0x0c00, data, 1);
			}
			break;

			case 6:
			mmc3_prg_bank0 = data;

			/* check bit 6 of mmc3_cmd (0x40) to select switchable rom page */
			if(mmc3_cmd & 0x40) {
				mmc3_switch_prg(0xc000,data);
			} else {
				mmc3_switch_prg(0x8000,data);
			}
			break;

			case 7:
			mmc3_prg_bank1 = data;
			mmc3_switch_prg(0xa000,data);
			break;
                }
		break;

		case 0xa000:
		/* set horizontal/vertical mirroring */
		if(data & 0x01) {
			/* set to vertical */
			MIRRORING = 1;
		} else {
			/* set to horizontal */
			MIRRORING = 0;
		}
		break;

		case 0xa001:
		if(data)
			SRAM = 1;
		break;

		case 0xc000:
		/* set IRQ counter */
		if(mmc3_irq_enable == 0)
			mmc3_irq_counter = data;
		break;

		case 0xc001:
		/* set IRQ tmp latch */
		mmc3_irq_latch = data;
		break;

		case 0xe000:
		mmc3_irq_counter = mmc3_irq_latch;
		mmc3_irq_enable = 0;
		break;

		case 0xe001:
		mmc3_irq_enable = 1;
		break;
	}
}
