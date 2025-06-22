#include "platform.h"
#include "flash.h"


/**
 * Flash section for the Intel based flash chips.
 */

int flash_init(void) {
	erase_cmdlet = ERASE_NO;

	flash_switch_to_read_and_clean_regs(FLASH_START_ADDRESS);

	return RESULT_OK;
}

void flash_nop(u8 nop_count) {
	u8 i;
	for (i = 0; i < nop_count; ++i) {
		asm volatile ("nop");
	}
}

void flash_wait(volatile u16 *reg_addr_ctl) {
	while ((*reg_addr_ctl & FLASH_INTEL_STATUS_READY) != FLASH_INTEL_STATUS_READY) {
		flash_nop(8);
	}
}

void flash_switch_to_read_and_clean_regs(volatile u16 *reg_addr_ctl) {
	*reg_addr_ctl = FLASH_INTEL_COMMAND_CLEAR;
	flash_nop(12);

	*reg_addr_ctl = FLASH_INTEL_COMMAND_READ;
	flash_nop(12);
}

void flash_unlock(volatile u16 *reg_addr_ctl) {
	*reg_addr_ctl = FLASH_INTEL_COMMAND_LOCK;
	flash_nop(12);

	*reg_addr_ctl = FLASH_INTEL_COMMAND_CONFIRM;
	flash_nop(12);
}

void flash_erase(volatile u16 *reg_addr_ctl) {
	*reg_addr_ctl = FLASH_INTEL_COMMAND_ERASE;
	flash_nop(12);

	*reg_addr_ctl = FLASH_INTEL_COMMAND_CONFIRM;
	flash_nop(12);

	flash_wait(reg_addr_ctl);
}

void flash_write_word(volatile u16 *reg_addr_ctl, u16 word) {
	*reg_addr_ctl = FLASH_INTEL_COMMAND_WRITE;
	flash_nop(12);

	*reg_addr_ctl = word;
	flash_nop(12);
}

void flash_write_block(volatile u16 *reg_addr_ctl, volatile u16 *buffer, u32 size) {
	volatile u16 *src = buffer;
	volatile u16 *dst = reg_addr_ctl;
	volatile u16 *end = dst + (size / 2);

	while (dst < end) {
		u16 word = *src;
		if (word != 0xFFFF) {
			flash_write_word(dst, word);
			flash_wait(dst);
			watchdog_service();
		}
		dst++;
		src++;
	}

	flash_switch_to_read_and_clean_regs(dst);
}

void flash_write_buffer(volatile u16 *reg_addr_ctl, const u16 *buffer, u32 size) {
	u16 i;
	u32 length;
	u32 size_index;
	volatile u16 *src = (volatile u16 *) buffer;
	volatile u16 *dst = reg_addr_ctl;

	size_index = size / 2;

	do {
		length = (size_index <= 32) ? size_index : 32;

		do
		{
			*dst = FLASH_INTEL_COMMAND_WRITE_BUFFER;

			if ((*dst & 0x30) != 0) {
				*dst = FLASH_INTEL_COMMAND_CLEAR;
			}
		} while ((*dst & FLASH_INTEL_STATUS_READY) != FLASH_INTEL_STATUS_READY);

		*dst = BUFFER_SIZE_TO_WRITE(length);

		for (i = 0; i < length; ++i) {
			*dst = *src;

			dst++;
			src++;
		}

		*reg_addr_ctl = FLASH_INTEL_COMMAND_CONFIRM;

		while ((*reg_addr_ctl & 0xBC) == 0);

		watchdog_service();

		size_index -= length;
	} while (size_index > 0);

	flash_switch_to_read_and_clean_regs(reg_addr_ctl);
}

int flash_geometry(volatile u16 *reg_addr_ctl) {
	u32 block_size;
	u32 addr = (u32) reg_addr_ctl;

	if ((addr >= ((u32) FLASH_INTEL_START_PARAMETER_BLOCKS)) && (addr < ((u32) FLASH_INTEL_END_PARAMETER_BLOCKS))) {
		block_size = 0x8000;  /* 0x8000x4 parameter blocks. */
	} else {
		block_size = 0x20000; /* 0x20000x255+ main blocks. */
	}

	/*
	 * (addr % block_size) but without __aeabi_uidivmod() libgcc routine.
	 */

	return (addr & (block_size - 1));
}
