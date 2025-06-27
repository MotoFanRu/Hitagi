#include "flash.h"

/**
 * Intel-like (Intel, ST, Numonyx, etc.) flash chips with 16-bit width bus driver.
 *
 * Documentation:
 *  StrataFlash_Wireless_Memory_(L30).pdf
 *  https://firmware.center/firmware/Motorola/E398%20%28Polo%29/Service%20Docs/Data%20Sheets/U700_Numonyx_StrataFlash_Wireless_Memory_%28L30%29.pdf
 */

#define FLASH_INTEL_START_PARAMETER_BLOCKS   ((volatile FLASH_DATA_WIDTH *) 0x10000000)
#define FLASH_INTEL_END_PARAMETER_BLOCKS     ((volatile FLASH_DATA_WIDTH *) 0x10020000)

#define FLASH_INTEL_STATUS_READY           FLASH_COMMAND(0x80)

#define FLASH_INTEL_COMMAND_ERASE          FLASH_COMMAND(0x20)
#define FLASH_INTEL_COMMAND_WRITE          FLASH_COMMAND(0x40)
#define FLASH_INTEL_COMMAND_CLEAR          FLASH_COMMAND(0x50)
#define FLASH_INTEL_COMMAND_LOCK           FLASH_COMMAND(0x60)
#define FLASH_INTEL_COMMAND_PART_ID        FLASH_COMMAND(0x90)
#define FLASH_INTEL_COMMAND_WRITE_PROTECT  FLASH_COMMAND(0xC0)
#define FLASH_INTEL_COMMAND_CONFIRM        FLASH_COMMAND(0xD0)
#define FLASH_INTEL_COMMAND_WRITE_BUFFER   FLASH_COMMAND(0xE8)
#define FLASH_INTEL_COMMAND_READ           FLASH_COMMAND(0xFF)

#define FLASH_INTEL_PR_LOCK_REG0           (0x80)
#define FLASH_INTEL_PR_LOCK_REG1           (0x89)

#define FLASH_INTEL_PR__64BIT_SIZE_16BIT   (( 64 >> 3) >> (sizeof(u16) >> 1))
#define FLASH_INTEL_PR_128BIT_SIZE_16BIT   ((128 >> 3) >> (sizeof(u16) >> 1))

/**
 * Functions.
 */

static void flash_wait(volatile u16 *reg_addr_ctl);
static void flash_reset(volatile u16 *reg_addr_ctl);

/**
 * Flash section for the Intel based flash chips.
 */

int flash_init(void) {
	erase_cmdlet = ERASE_NO;

	flash_reset(FLASH_START_ADDRESS);

	return RESULT_OK;
}

static void flash_wait(volatile u16 *reg_addr_ctl) {
	while ((*reg_addr_ctl & FLASH_INTEL_STATUS_READY) != FLASH_INTEL_STATUS_READY) {
		nop(8);
	}
}

static void flash_reset(volatile u16 *reg_addr_ctl) {
	*reg_addr_ctl = FLASH_INTEL_COMMAND_CLEAR;
	nop(12);

	*reg_addr_ctl = FLASH_INTEL_COMMAND_READ;
	nop(12);
}

int flash_unlock(volatile u16 *reg_addr_ctl) {
	*reg_addr_ctl = FLASH_INTEL_COMMAND_LOCK;
	nop(12);

	*reg_addr_ctl = FLASH_INTEL_COMMAND_CONFIRM;
	nop(12);

	return RESULT_OK;
}

int flash_erase(volatile u16 *reg_addr_ctl) {
	*reg_addr_ctl = FLASH_INTEL_COMMAND_ERASE;
	nop(12);

	*reg_addr_ctl = FLASH_INTEL_COMMAND_CONFIRM;
	nop(12);

	flash_wait(reg_addr_ctl);

	return RESULT_OK;
}

int flash_write_block(volatile u16 *reg_addr_ctl, volatile u16 *buffer, u32 size) {
	volatile u16 *src = buffer;
	volatile u16 *dst = reg_addr_ctl;
	volatile u16 *end = dst + (size / 2);

	while (dst < end) {
		u16 word = *src;
		if (word != 0xFFFF) {
			/* Write word seq. */
			*dst = FLASH_INTEL_COMMAND_WRITE;
			nop(12);

			*dst = word;
			nop(12);

			/* Wait Loops. */
			flash_wait(dst);
			watchdog_service();
		}
		dst++;
		src++;
	}

	flash_reset(reg_addr_ctl);

	return RESULT_OK;
}

int flash_write_buffer(volatile u16 *reg_addr_ctl, const u16 *buffer, u32 size) {
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

	flash_reset(reg_addr_ctl);

	return RESULT_OK;
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

u32 flash_get_part_id(volatile u16 *reg_addr_ctl) {
	u32 flash_part_id;

	flash_part_id = 0;

	volatile u16 *vendor_code = reg_addr_ctl;
	volatile u16 *device_code = reg_addr_ctl + 1;

	*vendor_code = FLASH_INTEL_COMMAND_PART_ID;
	nop(12);

	flash_part_id = (*vendor_code << 16) | *device_code;

	flash_reset(reg_addr_ctl);

	return flash_part_id;
}

/*
 * Intel Numonyx™ StrataFlash® Wireless Memory (L30) 28F128L30, 28F256L30
 *
 * OTP space:
 *   64   (8 bytes):   unique factory device identifier bits.
 *   64   (8 bytes):   user-programmable OTP bits.
 *   2048 (256 bytes): additional user-programmable OTP bits.
 */

int flash_get_otp_zone(volatile u16 *reg_addr_ctl, u8 *otp_out_buffer, u16 *size) {
	u16 i;
	u16 factory_reg[4];
	u16 user_reg[4];
	u16 user_add_reg[128];

	volatile u16 *flash = reg_addr_ctl;
	volatile u16 *factory_reg_ptr = &factory_reg[0];
	volatile u16 *user_reg_ptr = &user_reg[0];
	volatile u16 *user_add_reg_ptr = &user_add_reg[0];

	*size = 8 + 8 + 256;

	flash += FLASH_INTEL_PR_LOCK_REG0;
	flash++;

	for (i = 0; i < FLASH_INTEL_PR__64BIT_SIZE_16BIT; ++i, ++flash) {
		*flash = FLASH_INTEL_COMMAND_PART_ID;
		nop(12);

		*(factory_reg_ptr + i) = *flash;
		*(user_reg_ptr + i)    = *(flash + FLASH_INTEL_PR__64BIT_SIZE_16BIT);

		watchdog_service();

		flash_reset(flash);
	}

	flash = reg_addr_ctl;
	flash += FLASH_INTEL_PR_LOCK_REG1;
	flash++;
	flash += (FLASH_INTEL_PR_128BIT_SIZE_16BIT * (1 - 1));

	for (i = 0; i < FLASH_INTEL_PR_128BIT_SIZE_16BIT * 16; ++i, ++flash) {
		*flash = FLASH_INTEL_COMMAND_PART_ID;
		nop(12);

		*(user_add_reg_ptr + i) = *flash;

		watchdog_service();

		flash_reset(flash);
	}

	flash_reset(flash--);

	volatile u8 *factory_reg_ptr_u8 = (volatile u8 *) factory_reg_ptr;
	volatile u8 *user_reg_ptr_u8 = (volatile u8 *) user_reg_ptr;
	volatile u8 *user_add_reg_ptr_u8 = (volatile u8 *) user_add_reg_ptr;

	for (i = 0; i < 8; ++i) {
		otp_out_buffer[i] = factory_reg_ptr_u8[i];
	}

	for (i = 0; i < 8; ++i) {
		otp_out_buffer[8+i] = user_reg_ptr_u8[i];
	}

	for (i = 0; i < 256; ++i) {
		otp_out_buffer[16+i] = user_add_reg_ptr_u8[i];
	}

	return RESULT_OK;
}
