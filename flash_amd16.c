#include "flash.h"

/**
 * AMD-like (AMD, Fujitsu, Spansion) flash chips with 16-bit width bus driver.
 *
 * Documentation:
 *  S71WS-NX0.PDF
 *  https://forum.motofan.ru/index.php?act=Attach&type=post&id=280636
 *
 *  LLD_v5.13_to_Flash_Driver_an_01_e.fm.pdf
 *  How to Use Spansion™ LLD v5.13 to Implement a Flash Driver
 */

#define FLASH_AMD_START_PARAMETER_BLOCKS_1    ((volatile FLASH_DATA_WIDTH *) 0x10000000)
#define FLASH_AMD_END_PARAMETER_BLOCKS_1      ((volatile FLASH_DATA_WIDTH *) 0x10020000)
#define FLASH_AMD_START_PARAMETER_BLOCKS_2    ((volatile FLASH_DATA_WIDTH *) 0x11FE0000)
#define FLASH_AMD_END_PARAMETER_BLOCKS_2      ((volatile FLASH_DATA_WIDTH *) 0x12000000)

#define FLASH_AMD_CMD_REGW_1               (0x00000555)
#define FLASH_AMD_CMD_REGW_2               (0x000002AA)

#define FLASH_AMD_DATA_DONE_STATUS         FLASH_COMMAND(0x80)

#define FLASH_AMD_COMMAND_READ             FLASH_COMMAND(0xF0)
#define FLASH_AMD_COMMAND_SETUP_ERASE      FLASH_COMMAND(0x80)
#define FLASH_AMD_COMMAND_SETUP_WRITE      FLASH_COMMAND(0xA0)
#define FLASH_AMD_COMMAND_SETUP_WRITE_BUF  FLASH_COMMAND(0x25)
#define FLASH_AMD_COMMAND_RESET_WRITE_BUF  FLASH_COMMAND(0xF0)
#define FLASH_AMD_COMMAND_CONFIRM          FLASH_COMMAND(0x29)
#define FLASH_AMD_COMMAND_ERASE_SECTOR     FLASH_COMMAND(0x30)
#define FLASH_AMD_COMMAND_ERASE_CHIP       FLASH_COMMAND(0x10)
#define FLASH_AMD_COMMAND_UNLOCK_1         FLASH_COMMAND(0xAA)
#define FLASH_AMD_COMMAND_UNLOCK_2         FLASH_COMMAND(0x55)
#define FLASH_AMD_COMMAND_PART_ID          FLASH_COMMAND(0x90)

/**
 * Functions.
 */

static int flash_wait(volatile u16 *reg_addr_ctl, const u16 data);
static void flash_reset(volatile u16 *reg_addr_ctl);

/**
 * Flash section for the AMD based flash chips.
 */

int flash_init(void) {
	erase_cmdlet = ERASE_NO;

	flash_reset(FLASH_START_ADDRESS);

	return RESULT_OK;
}

static int flash_wait(volatile u16 *reg_addr_ctl, const u16 data) {
	u16 word = *reg_addr_ctl;

	while ((word & FLASH_AMD_DATA_DONE_STATUS) != (data & FLASH_AMD_DATA_DONE_STATUS)) {
		watchdog_service();
		word = *reg_addr_ctl;
	}

	word = *reg_addr_ctl;
	nop(12);

	return (word != data) ? word : RESULT_OK;
}

static void flash_reset(volatile u16 *reg_addr_ctl) {
	*reg_addr_ctl = FLASH_AMD_COMMAND_READ;
	nop(12);
}

int flash_unlock(volatile u16 *reg_addr_ctl) {
	UNUSED(reg_addr_ctl);

	nop(12);

	return RESULT_OK;
}

int flash_erase(volatile u16 *reg_addr_ctl) {
	u32 status;

	*(FLASH_START_ADDRESS + FLASH_AMD_CMD_REGW_1) = FLASH_AMD_COMMAND_UNLOCK_1;
	*(FLASH_START_ADDRESS + FLASH_AMD_CMD_REGW_2) = FLASH_AMD_COMMAND_UNLOCK_2;

	*(FLASH_START_ADDRESS + FLASH_AMD_CMD_REGW_1) = FLASH_AMD_COMMAND_SETUP_ERASE;

	*(FLASH_START_ADDRESS + FLASH_AMD_CMD_REGW_1) = FLASH_AMD_COMMAND_UNLOCK_1;
	*(FLASH_START_ADDRESS + FLASH_AMD_CMD_REGW_2) = FLASH_AMD_COMMAND_UNLOCK_2;

#if 0
	/* Is this will erase entire flash? */
	*(FLASH_START_ADDRESS + 0x00000000) = FLASH_AMD_COMMAND_ERASE_CHIP;
#endif

	*reg_addr_ctl = FLASH_AMD_COMMAND_ERASE_SECTOR;

	status = flash_wait(reg_addr_ctl, 0xFFFF);

	flash_reset(reg_addr_ctl);

	return status;
}

int flash_write_block(volatile u16 *reg_addr_ctl, volatile u16 *buffer, u32 size) {
	u32 status;

	volatile u16 *src = buffer;
	volatile u16 *dst = reg_addr_ctl;
	volatile u16 *end = dst + (size / 2);

	status = RESULT_OK;

	while (dst < end) {
		u16 word = *src;
		if (word != 0xFFFF) {
			/* Write word seq. */
			*(FLASH_START_ADDRESS + FLASH_AMD_CMD_REGW_1) = FLASH_AMD_COMMAND_UNLOCK_1;
			*(FLASH_START_ADDRESS + FLASH_AMD_CMD_REGW_2) = FLASH_AMD_COMMAND_UNLOCK_2;

			*(FLASH_START_ADDRESS + FLASH_AMD_CMD_REGW_1) = FLASH_AMD_COMMAND_SETUP_WRITE;
			nop(12);

			*dst = word;

			/* Wait Loops. */
			watchdog_service();
			status = flash_wait(dst, word);
			if (status != RESULT_OK) {
				flash_reset(reg_addr_ctl);
				return status;
			}
		}
		dst++;
		src++;
	}

	flash_reset(reg_addr_ctl);

	return RESULT_OK;
}

/* EXL, 25-Jun-2025: Looks like this one works only with 0x40 usb write buffer size in the Flash Terminal. */
int flash_write_buffer(volatile u16 *reg_addr_ctl, const u16 *buffer, u32 size) {
	u16 i;
	u32 length;
	u32 status;
	u32 size_index;
	volatile u16 *src = (volatile u16 *) buffer;
	volatile u16 *dst = reg_addr_ctl;

	status = RESULT_OK;
	size_index = size / 2;

	do {
		length = (size_index <= 32) ? size_index : 32;

		*(FLASH_START_ADDRESS + FLASH_AMD_CMD_REGW_1) = FLASH_AMD_COMMAND_UNLOCK_1;
		*(FLASH_START_ADDRESS + FLASH_AMD_CMD_REGW_2) = FLASH_AMD_COMMAND_UNLOCK_2;

		*dst = FLASH_AMD_COMMAND_SETUP_WRITE_BUF;

		*dst = BUFFER_SIZE_TO_WRITE(length);

		for (i = 0; i < length; ++i) {
			*dst = *src;

			dst++;
			src++;
		}

		*dst = FLASH_AMD_COMMAND_CONFIRM;
		nop(16);

		watchdog_service();

#if 0
		/* Finalization. */
		*(FLASH_START_ADDRESS + FLASH_AMD_CMD_REGW_1) = FLASH_AMD_COMMAND_UNLOCK_1;
		*(FLASH_START_ADDRESS + FLASH_AMD_CMD_REGW_2) = FLASH_AMD_COMMAND_UNLOCK_2;

		*(FLASH_START_ADDRESS + FLASH_AMD_CMD_REGW_1) = FLASH_AMD_COMMAND_RESET_WRITE_BUF;

		flash_reset(reg_addr_ctl);
#endif

		size_index -= length;
	} while (size_index > 0);

	flash_reset(reg_addr_ctl);

	return status;
}

int flash_geometry(volatile u16 *reg_addr_ctl) {
	u32 block_size;
	u32 addr = (u32) reg_addr_ctl;

	if (
		((addr >= (u32) FLASH_AMD_START_PARAMETER_BLOCKS_1) && (addr < (u32) FLASH_AMD_END_PARAMETER_BLOCKS_1)) ||
		((addr >= (u32) FLASH_AMD_START_PARAMETER_BLOCKS_2) && (addr < (u32) FLASH_AMD_END_PARAMETER_BLOCKS_2))
	) {
		block_size = 0x8000;  /* 0x8000x8 parameter blocks in the start and end of flash. */
	} else {
		block_size = 0x20000; /* 0x20000x254+ main blocks. */
	}

	/*
	 * (addr % block_size) but without __aeabi_uidivmod() libgcc routine.
	 */

	return (addr & (block_size - 1));
}

u32 flash_get_part_id(volatile u16 *reg_addr_ctl) {
	u32 flash_part_id;

	flash_part_id = 0;

	*(reg_addr_ctl + FLASH_AMD_CMD_REGW_1) = FLASH_AMD_COMMAND_UNLOCK_1;
	*(reg_addr_ctl + FLASH_AMD_CMD_REGW_2) = FLASH_AMD_COMMAND_UNLOCK_2;
	nop(12);

	*(reg_addr_ctl + FLASH_AMD_CMD_REGW_1) = FLASH_AMD_COMMAND_PART_ID;
	nop(12);

	flash_part_id  = (u32) (*(reg_addr_ctl + 0x0001) & 0x000000FF) << 16;
	flash_part_id |= (u32) (*(reg_addr_ctl + 0x000E) & 0x000000FF) <<  8;
	flash_part_id |= (u32) (*(reg_addr_ctl + 0x000F) & 0x000000FF) <<  0;

	flash_reset(reg_addr_ctl);

	return flash_part_id;
}
