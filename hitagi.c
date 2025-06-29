/*
 * About:
 *   Hitagi main module with Motorola Flash Protocol implementation.
 *
 * Author:
 *   EXL, Motorola Inc.
 *
 * License:
 *   MIT
 *
 * Commands:
 *   ADDR        |.ADDR.10000000XX.       |  # Set address for BIN command, XX is checksum.
 *   BIN         |.BIN.DATAXX.            |  # Upload binary to address (IRAM, RAM, Flash for flashing), XX is checksum.
 *   ERASE       |.ERASE.                 |  # Activate read and write mode. See below for more details.
 *   READ        |.READ.10000000,0200.    |  # Read data from address on size.
 *   RQHW        |.RQHW.                  |  # Request hardware info data, bootloader version.
 *   RQRC        |.RQRC.10000000,10000600.|  # Calculate checksum of addresses range.
 *   RQVN        |.RQVN.                  |  # Request version info.
 *   RQSW        |.RQSW.                  |  # Request S/W version.
 *   RQSN        |.RQSN.                  |  # Request serial number of SoC.
 *   RQFI        |.RQFI.                  |  # Request part ID from flash memory chip.
 *   READ_OTP    |.READ_OTP.              |  # Read OTP registers data.
 *   RESTART     |.RESTART.               |  # Restart or power down device.
 *   POWER_DOWN  |.POWER_DOWN.            |  # Power off device.
 *
 * Notes:
 *   1. Flash modes can be switched by calling the method that sets the `ERASE` flag a different number of times.
 *
 *      # 0. Read-only mode. No `ERASE` flag is set.
 *
 *      # 1. Read/Write word mode for the entire flash.
 *      mfp_cmd(er, ew, 'ERASE')
 *
 *      # 2. Read/Write buffer mode for the entire flash.
 *      mfp_cmd(er, ew, 'ERASE')
 *      mfp_cmd(er, ew, 'ERASE')
 *
 *      # 3. Erase-only mode for the entire flash.
 *      mfp_cmd(er, ew, 'ERASE')
 *      mfp_cmd(er, ew, 'ERASE')
 *      mfp_cmd(er, ew, 'ERASE')
 *
 *   2. It is better if the flashed chunk size is a multiple of `0x8000` (parameter blocks) or `0x20000` (main blocks)
 *      for Intel-like and AMD-like flash chips.
 */

#include "platform.h"

#if defined(FTR_NEPTUNE_LTE1) || defined(FTR_NEPTUNE_LTE2)
#include "regs_neptune.h"
#endif

#include "flash.h"

/**
 * Functions.
 */

static void util_u8_to_hexasc(u8 val, u8 *str);
static void util_u16_to_hexasc(u16 val, u8 *str);
static void util_u32_to_hexasc(u32 val, u8 *str);
static u32 util_hexasc_to_u32(const u8 *str, u8 size);
static void util_string_copy(u8 *dst, const u8 *src);
static int util_string_equal(const u8 *str1_ptr, const u8 *str2_ptr);
static int util_map_cmd(const HITAGI_CMD_TABLE_T *table_ptr, u8 table_size, const u8 *cmd);

static void usb_copy_block(const u8 *src, u16 *dst, u8 len);
static int usb_tx(const u8 *src, u8 len);
static u8 usb_rx(u8 *dst);
static int usb_init(void);

static int watchdog_reboot(void);
static int watchdog_shutdown(void);
int watchdog_service(void);
static int watchdog_init(void);

static void hitagi_command_ADDR(const u8 *answer_str, const u8 *data_ptr, const u8 *buffer_next_byte);
static void hitagi_command_BIN(const u8 *answer_str, const u8 *data_ptr, const u8 *buffer_next_byte);
static void hitagi_command_ERASE(const u8 *answer_str, const u8 *data_ptr, const u8 *buffer_next_byte);
static void hitagi_command_READ(const u8 *answer_str, const u8 *data_ptr, const u8 *buffer_next_byte);
static void hitagi_command_RQHW(const u8 *answer_str, const u8 *data_ptr, const u8 *buffer_next_byte);
static void hitagi_command_RQRC(const u8 *answer_str, const u8 *data_ptr, const u8 *buffer_next_byte);
static void hitagi_command_RQVN(const u8 *answer_str, const u8 *data_ptr, const u8 *buffer_next_byte);
static void hitagi_command_RQSW(const u8 *answer_str, const u8 *data_ptr, const u8 *buffer_next_byte);
static void hitagi_command_RQSN(const u8 *answer_str, const u8 *data_ptr, const u8 *buffer_next_byte);
static void hitagi_command_RQFI(const u8 *answer_str, const u8 *data_ptr, const u8 *buffer_next_byte);
static void hitagi_command_READ_OTP(const u8 *answer_str, const u8 *data_ptr, const u8 *buffer_next_byte);
static void hitagi_command_RESTART(const u8 *answer_str, const u8 *data_ptr, const u8 *buffer_next_byte);
static void hitagi_command_POWER_DOWN(const u8 *answer_str, const u8 *data_ptr, const u8 *buffer_next_byte);
static void hitagi_commands(const u8 *cmd, const u8 *data, const u8 *next);
static void hitagi_send_packet(const u8 *cmd, const u8 *data);
static void hitagi_send_bin_packet(const u8 *cmd, const u8 *data, u16 bin_size);
static void hitagi_send_packet_aux(const u8 *cmd, const u8 *data, u16 bin_size);
static void hitagi_send_ack(const u8 *data);
static void hitagi_send_error(u8 error_code);
static void hitagi_read_packets(void);

void hitagi_start(void);
void __attribute__((naked, section(".startup"))) _start(void);

/**
 * Constants and command table.
 */

static const u8 ack_str[]  = "ACK";
static const u8 err_str[]  = "ERR";
static const u8 bin_str[]  = "BIN";
static const u8 com_str[]  = ","  ;
static const u8 scm_str[]  = ":"  ;

static const HITAGI_CMD_TABLE_T cmd_tbl[] = {
	{ (const u8 *) "ADDR",       (const u8 *) NULL,         hitagi_command_ADDR        },
	{ (const u8 *) "BIN",        (const u8 *) NULL,         hitagi_command_BIN         },
	{ (const u8 *) "ERASE",      (const u8 *) NULL,         hitagi_command_ERASE       },
	{ (const u8 *) "READ",       (const u8 *) "READ",       hitagi_command_READ        },
	{ (const u8 *) "RQHW",       (const u8 *) "RSHW",       hitagi_command_RQHW        },
#if !defined(FTR_COMPACT)
	{ (const u8 *) "RQRC",       (const u8 *) "RSRC",       hitagi_command_RQRC        },
	{ (const u8 *) "RQVN",       (const u8 *) "RSVN",       hitagi_command_RQVN        },
	{ (const u8 *) "RQSW",       (const u8 *) "RSSW",       hitagi_command_RQSW        },
	{ (const u8 *) "RQSN",       (const u8 *) "RSSN",       hitagi_command_RQSN        },
	{ (const u8 *) "RQFI",       (const u8 *) "RSFI",       hitagi_command_RQFI        },
	{ (const u8 *) "READ_OTP",   (const u8 *) "READ_OTP",   hitagi_command_READ_OTP    },
	{ (const u8 *) "RESTART",    (const u8 *) NULL,         hitagi_command_RESTART     },
	{ (const u8 *) "POWER_DOWN", (const u8 *) NULL,         hitagi_command_POWER_DOWN  },
#endif
};

/**
 * Globals and Rx/Tx buffers.
 */

static u16 *received_address_ptr;
static u16  received_packet_size;

static u8 rx_command[MAX_COMMAND_STR_SIZE];

#if !defined(FTR_COMPACT)
static u8 rx_data[USB_MAX_RX_DATA_SIZE];
static u8 tx_data[USB_MAX_TX_DATA_SIZE];
#else
static u8 *rx_data = (u8 *) 0x03FD0000 + 0x10000;
static u8 *tx_data = (u8 *) 0x03FD0000 + 0x10000 + USB_MAX_RX_DATA_SIZE;
#endif

HITAGI_CMDLET_ERASE_T erase_cmdlet;

/**
 * NOP function.
 */

void nop(u32 nop_count) {
	u32 i;
	for (i = 0; i < nop_count; ++i) {
		asm volatile ("nop");
	}
}

/**
 * Util functions.
 */

static void util_u8_to_hexasc(u8 val, u8 *str) {
	u8 i;
	u8 digit;

	for (i = 0; i < 2; ++i) {
		digit = (val >> 4) & 0x0F;
		val <<= 4;
		*str++ = (digit > 9) ? (digit + '7') : (digit + '0');
	}

	*str = NUL;
}

static void util_u16_to_hexasc(u16 val, u8 *str) {
	u8 i;
	u8 digit;

	for (i = 0; i < 4; ++i) {
		digit = (val >> 12) & 0x0F;
		val <<= 4;
		*str++ = (digit > 9) ? (digit + '7') : (digit + '0');
	}

	*str = NUL;
}

static void util_u32_to_hexasc(u32 val, u8 *str) {
	u8 i;
	u8 digit;

	for (i = 0; i < 8; ++i) {
		digit = (val >> 28) & 0x0F;
		val <<= 4;
		*str++ = (digit > 9) ? (digit + '7') : (digit + '0');
	}

	*str = NUL;
}

static u32 util_hexasc_to_u32(const u8 *str, u8 size) {
	u8 digit;
	u32 val = 0;

	while (size--) {
		digit = *str++;
		val <<= 4; /* Shift previous digit over. */
		val += (digit >= 'A') ? (digit - '7') : (digit - '0');
	}

	return val;
}

static void util_string_copy(u8 *dst, const u8 *src) {
	/* The do-while loop will copy the NUL terminator! */
	do {
		*dst++ = *src;
	} while (*src++);
}

static int util_string_equal(const u8 *str1_ptr, const u8 *str2_ptr) {
	int match = 0;

	while (*str1_ptr && *str2_ptr && (*str1_ptr == *str2_ptr)) {
		str1_ptr++;
		str2_ptr++;
	}

	if (*str1_ptr == *str2_ptr) {
		match = 1;
	}

	return match;
}

static int util_map_cmd(const HITAGI_CMD_TABLE_T *table_ptr, u8 table_size, const u8 *cmd) {
	u8 i;
	for (i = 0; i < table_size; ++i) {
		if (util_string_equal(table_ptr[i].cmd, cmd)) {
			return i;
		}
	}
	return -1;
}

/**
 * USB section.
 */

static void usb_copy_block(const u8 *src, u16 *dst, u8 len) {
	u8 i;
	u8 half_len;
	u16 data;

	half_len = len / 2;

	for (i = 0; i < half_len; ++i) {
		data = (u16) (*src++ << 8);
		*dst++ = (data | *src++);
	}

	if ((len % 2) != 0) {
		data = (u16) (*src << 8);
		*dst = data;
	}
}

static int usb_tx(const u8 *src, u8 len) {
	/*
	 * (1 << 6):  EP2_int
	 * (1 << 10): XFREN
	 * (1 << 13): IEOFC
	 */
	if ((USB_INT & (1 << 6)) || !(USB_E2_CR & (1 << 10))) {
		usb_copy_block(src, USB_E2_TX_HW_BUFFER, len);

		USB_E2_CR = (USB_E2_CR & ~0x3F) | (len & 0x3F);
		USB_E2_CR |= (1 << 13);
		USB_E2_CR |= (1 << 10);

		return RESULT_OK;
	}

	return RESULT_FAIL;
}

static u8 usb_rx(u8 *dst) {
	u8 i;
	u8 rx_bytes;

	rx_bytes = 0;

	/*
	 * (1 << 5):  EP1_int
	 * (1 << 6):  ERRF
	 * (1 << 13): IEOFC
	 * (1 << 11): FSTALL
	 */
	if (USB_INT & (1 << 5)) {
		if (!(USB_E1_CR & (1 << 6))) {
			u8 *p_dst = dst;
			u8 *p_src = (u8 *) USB_E1_RX_HW_BUFFER;

			rx_bytes = USB_E1_CR & 0x3F;

			for (i = 0; i < rx_bytes; ++i) {
				*p_dst++ = *p_src++;
			}

			USB_E1_CR |= (1 << 13);
		} else {
			USB_E1_CR |= (1 << 11);
		}
	}

	return rx_bytes;
}

static int usb_init(void) {
	/*
	 * USB_MEMMAP_EP1_EP2_16BYTES = 0x0000
	 * USB_MEMMAP_EP1_EP2_32BYTES = 0x0001
	 */
	USB_CPU_CR = 0x0001;

	return RESULT_OK;
}

/**
 * Watchdog section.
 */

static int watchdog_reboot(void) {
	/*
	 * PU_MAIN_SOFTWARE_RESET_PU = 0x0A
	 */
	if (RTC_PCRAM0 < 0x0A) {
		RTC_PCRAM0 = 0x0A;
	}

	/*
	 * NOT_SW_RESET = 0x0010
	 * WATCHDOG_WCR &= ~(NOT_SW_RESET)
	 */
	WATCHDOG_WCR &= ~(0x0010);

	/* Forever! */
	while("MotoFan.Ru is cool!");

	return RESULT_OK;
}

static int watchdog_shutdown(void) {
	/*
	 * WD_NOT_ASSERTED = 0x0020
	 * WATCHDOG_WCR &= ~(WD_NOT_ASSERTED)
	 */
	WATCHDOG_WCR &= ~(0x0020);

	/* Forever! */
	while("MotoFan.Ru is cool!");

	return RESULT_OK;
}

int watchdog_service(void) {
	WATCHDOG_WSR = 0x5555;
	WATCHDOG_WSR = 0xAAAA;

	return RESULT_OK;
}

static int watchdog_init(void) {
	/*
	 * (THIRTYTWO_SEC_TIMEOUT | WD_OUTPUT_EN | WD_NOT_ASSERTED | NOT_SW_RESET | WD_ENABLE | WD_DEBUG)
	 *
	 * hex((63 << 9) | 0x0040 | 0x0020 | 0x0010 | 0x0004 | 0x0002)
	 * '0x7e76'
	 */
	WATCHDOG_WCR = 0x7E76;

	watchdog_service();

	return RESULT_OK;
}

/**
 * Hitagi section.
 */

static void hitagi_command_ADDR(const u8 *answer_str, const u8 *data_ptr, const u8 *buffer_next_byte) {
	u32 addr;
	u8 response[MAX_RESP_DATA_SIZE];

	UNUSED(answer_str);
	UNUSED(buffer_next_byte);

	/* Converted the received address from ASCII string to number. */
	addr = util_hexasc_to_u32(&data_ptr[0], CMD_32_SIZE);

	received_address_ptr = (u16 *) addr;
	util_string_copy(&response[0], data_ptr);

	hitagi_send_ack(response);
}

static void hitagi_command_BIN(const u8 *answer_str, const u8 *data_ptr, const u8 *buffer_next_byte) {
	u32 i;
	u8 *rx_ptr;
	u8 *data_aligned_ptr;
	u8 nr_shift_right;
	u8 bytes_received;
	u32 bytes_received_total;

	UNUSED(answer_str);

	const u8 *source_ptr;

	/* Point to MSB of data size field. */
	source_ptr = data_ptr;

	/* Compute number of data bytes in this block and update global variable. */
	received_packet_size = ((source_ptr[BIN_DATA_SIZE_MSB] << SHIFT_MSB) + source_ptr[BIN_DATA_SIZE_LSB]);

	/* Advance to first data byte. */
	source_ptr += MAX_DATA_FIELD_SIZE;

	rx_ptr = (u8 *) buffer_next_byte;
	bytes_received_total = buffer_next_byte - source_ptr;

	while (bytes_received_total < received_packet_size) {
		bytes_received = usb_rx(rx_ptr);
		bytes_received_total += bytes_received;
		rx_ptr += bytes_received;
	}

	/* ACK the BIN command so the host can build up a new command/data packet. While we decrypt and copy it. */
	hitagi_send_ack(NULL);

	/*
	 * Realign data.
	 * Force data alignment to MCORE WORD (UINT32) boundary.
	 * Data is shifted to the right up to 3 bytes.
	 */
	nr_shift_right  = sizeof(u32) - (((u32) source_ptr) % sizeof(u32));
	nr_shift_right %= sizeof(u32); /* Within 0..3. */
	if (nr_shift_right != 0) {
		/* Odd address boundary: shift data right by nr_shift_right byte(s). */
		data_aligned_ptr = (u8 *) source_ptr + received_packet_size - 1;
		for (i = 0; i < received_packet_size; i++, data_aligned_ptr--) {
			data_aligned_ptr[nr_shift_right] = data_aligned_ptr[0];
		}
		/* Point to UINT32 aligned value. */
		source_ptr += nr_shift_right;
		buffer_next_byte += nr_shift_right;
	}

	if (erase_cmdlet == ERASE_NO) {
		/* Copy to RAM. */
		data_aligned_ptr = (u8 *) received_address_ptr;
		for (i = 0; i < received_packet_size; ++i) {
			*data_aligned_ptr++ = *source_ptr++;
		}
	} else {
		flash_unlock((volatile u16 *) received_address_ptr);

		if (flash_geometry((volatile u16 *) received_address_ptr) == RESULT_OK) {
			flash_erase((volatile u16 *) received_address_ptr);
		}

		if (erase_cmdlet != ERASE_ONLY) {
			if (erase_cmdlet == ERASE_WRITE_BLOCK) {
				flash_write_block(
					(volatile u16 *) received_address_ptr,
					(volatile u16 *) source_ptr,
					received_packet_size
				);
			} else if (erase_cmdlet == ERASE_WRITE_BUFFER) {
				flash_write_buffer(
					(volatile u16 *) received_address_ptr,
					(const u16 *) source_ptr,
					received_packet_size
				);
			} else {
				/* Unknown write flash method. */
				return;
			}
		}
	}

	/*
	 * Update the address pointer to track where we are.
	 * This eliminates the need for each BIN packet to be
	 * preceded by and ADDR packet for large section of contiguius memory.
	 */
	received_address_ptr += (received_packet_size / 2);
}

static void hitagi_command_ERASE(const u8 *answer_str, const u8 *data_ptr, const u8 *buffer_next_byte) {
	u8 response[MAX_READ_RESPONSE_SIZE];

	UNUSED(answer_str);
	UNUSED(data_ptr);
	UNUSED(buffer_next_byte);

	erase_cmdlet += 1;

	/* Comment out for compatibility with Motorola Flash Protocol. */
	util_u16_to_hexasc(erase_cmdlet, response);

	hitagi_send_ack(response);
}

static void hitagi_command_READ(const u8 *answer_str, const u8 *data_ptr, const u8 *buffer_next_byte) {
	u8 csum;
	u16 size;
	u32 start_addr;
	u8 response[MAX_READ_RESPONSE_SIZE];

	u8 *data_start_ptr;
	u8 *data_end_ptr;
	u8 *response_ptr;

	UNUSED(buffer_next_byte);

	csum = 0;
	response_ptr = &response[0];

	start_addr = util_hexasc_to_u32(&data_ptr[0], CMD_32_SIZE);
	size = util_hexasc_to_u32(&data_ptr[CMD_32_SIZE + 1], CMD_16_SIZE);

	data_start_ptr = (u8 *) start_addr;
	data_end_ptr = data_start_ptr + size;

	if ((size) < 0x10) {
		hitagi_send_error(ERR_DATA_INVALID);
		return;
	}

	*((u16 *) response_ptr) = size;
	response_ptr += 2;
	csum += (size >> 8) & 0xFF;
	csum += (size >> 0) & 0xFF;

	while (data_start_ptr < data_end_ptr) {
		u8 byte_data = *(data_start_ptr++);
		csum += byte_data;
		*(response_ptr++) = byte_data;

		watchdog_service();
	}

	*response_ptr = csum;

	/*
	 * 2: 16-bit size header.
	 * 1: 8-bit checksum.
	 */
	hitagi_send_bin_packet(answer_str, response, size + 2 + 1);
}

static void hitagi_command_RQHW(const u8 *answer_str, const u8 *data_ptr, const u8 *buffer_next_byte) {
	u8 i;
	u8 *response_ptr;
	u16 bootloader_version;
	u8 response[MAX_READ_RESPONSE_SIZE];

	UNUSED(data_ptr);
	UNUSED(buffer_next_byte);

	for (i = 0; i < 8 * 2; ++i) {
		response[i] = 0x30;
	}

	response_ptr = &response[12];
	bootloader_version = *(FLASH_START_ADDRESS + 0x07); /* 0x0E offset (0x0E / 2) bootloader version on 0x1000000E. */

	util_u16_to_hexasc(bootloader_version, response_ptr);

	hitagi_send_packet(answer_str, response);
}

static void hitagi_command_RQVN(const u8 *answer_str, const u8 *data_ptr, const u8 *buffer_next_byte) {
	u8 i;
	u8 *response_ptr;
	u16 bootloader_version;
	u8 response[MAX_READ_RESPONSE_SIZE];

	UNUSED(data_ptr);
	UNUSED(buffer_next_byte);

	for (i = 0; i < ((8 * 2) * 2) + 1; ++i) {
		response[i] = 0x30;
	}

	bootloader_version = *(FLASH_START_ADDRESS + 0x07); /* 0x0E offset (0x0E / 2) bootloader version on 0x1000000E. */

	response_ptr = &response[12];
	util_u16_to_hexasc(bootloader_version, response_ptr);
	response[16] = *com_str;

	/* Repeat bootloader version again. */
	response_ptr = &response[29];
	util_u16_to_hexasc(bootloader_version, response_ptr);

	hitagi_send_packet(answer_str, response);
}

static void hitagi_command_RQRC(const u8 *answer_str, const u8 *data_ptr, const u8 *buffer_next_byte) {
	u16 csum;
	u8 *data_start_ptr;
	u8 *data_end_ptr;
	u8 *response_ptr;
	u8 response[MAX_RESP_DATA_SIZE];
	u32 start_addr;
	u32 end_addr;

	UNUSED(buffer_next_byte);

	csum = 0;
	response_ptr = &response[0];

	start_addr = util_hexasc_to_u32(&data_ptr[0], CMD_32_SIZE);
	end_addr = util_hexasc_to_u32(&data_ptr[CMD_32_SIZE + 1], CMD_32_SIZE);

	data_start_ptr = (u8 *) start_addr;
	data_end_ptr = (u8 *) end_addr;

	if ((end_addr - start_addr) < 1) {
		hitagi_send_error(ERR_DATA_INVALID);
		return;
	}

	while (data_start_ptr <= data_end_ptr) {
		csum += *data_start_ptr;
		data_start_ptr++;
		watchdog_service();
	}

	util_u16_to_hexasc(csum, response_ptr);

	hitagi_send_packet(answer_str, response);
}

static void hitagi_command_RQSW(const u8 *answer_str, const u8 *data_ptr, const u8 *buffer_next_byte) {
	u8 *response_ptr;
	u8 response[MAX_READ_RESPONSE_SIZE];

	UNUSED(data_ptr);
	UNUSED(buffer_next_byte);

	response_ptr = &response[0];
	util_string_copy(response_ptr, (const u8 *) "Hitagi SW v1.0");
	response_ptr = &response[14];
	util_string_copy(response_ptr, scm_str); /* ":" is separator there. */
	response_ptr = &response[15];
	util_string_copy(response_ptr, (const u8 *) "Hitagi FLEX v1.0");

	hitagi_send_packet(answer_str, response);
}

static void hitagi_command_RQSN(const u8 *answer_str, const u8 *data_ptr, const u8 *buffer_next_byte) {
	u8 *response_ptr;
	u8 response[MAX_READ_RESPONSE_SIZE];

	volatile u16 *i;
	volatile u16 *start = NEPTUNE_UID_REG_ADDR;
	volatile u16 *end = (NEPTUNE_REV_REG_ADDR - 1);

	response_ptr = &response[0];

	UNUSED(data_ptr);
	UNUSED(buffer_next_byte);

	for (i = end; i >= start; --i) {
		util_u16_to_hexasc(*i, response_ptr);
		response_ptr += 4;
	}

	hitagi_send_packet(answer_str, response);
}

static void hitagi_command_RQFI(const u8 *answer_str, const u8 *data_ptr, const u8 *buffer_next_byte) {
	u32 flash_part_id;
	u8 response[MAX_READ_RESPONSE_SIZE];

	UNUSED(data_ptr);
	UNUSED(buffer_next_byte);

	flash_part_id = flash_get_part_id(FLASH_START_ADDRESS);
	util_u32_to_hexasc(flash_part_id, response);

	hitagi_send_packet(answer_str, response);
}

static void hitagi_command_READ_OTP(const u8 *answer_str, const u8 *data_ptr, const u8 *buffer_next_byte) {
	u16 i;
	u16 size;
	u8 otp_reg_buffer[FLASH_MAX_OTP_SIZE];
	u8 *response_ptr;
	u8 response[MAX_READ_RESPONSE_SIZE];

	UNUSED(data_ptr);
	UNUSED(buffer_next_byte);

	response_ptr = &response[0];

	flash_get_otp_zone(FLASH_START_ADDRESS, otp_reg_buffer, &size);

	for (i = 0; i < size; ++i) {
		util_u8_to_hexasc(otp_reg_buffer[i], response_ptr);
		response_ptr += 2;
	}

	hitagi_send_packet(answer_str, response);
}

static void hitagi_command_RESTART(const u8 *answer_str, const u8 *data_ptr, const u8 *buffer_next_byte) {
	UNUSED(answer_str);
	UNUSED(data_ptr);
	UNUSED(buffer_next_byte);

	hitagi_send_ack(NULL);

	/* Is 1M of NOPs enough? */
	nop(1024 * 1024);

	watchdog_reboot();
}

static void hitagi_command_POWER_DOWN(const u8 *answer_str, const u8 *data_ptr, const u8 *buffer_next_byte) {
	UNUSED(answer_str);
	UNUSED(data_ptr);
	UNUSED(buffer_next_byte);

	hitagi_send_ack(NULL);

	/* Is 1M of NOPs enough? */
	nop(1024 * 1024);

	watchdog_shutdown();
}

static void hitagi_commands(const u8 *cmd, const u8 *data, const u8 *next) {
	int idx = util_map_cmd(&cmd_tbl[0], sizeof(cmd_tbl) / sizeof(cmd_tbl[0]), cmd);
	if (idx >= 0 && cmd_tbl[idx].cmd_func) {
		cmd_tbl[idx].cmd_func(cmd_tbl[idx].answer_str, data, next);
	} else {
		hitagi_send_error(ERR_UNKNOWN_COMMAND);
	}
}

static void hitagi_send_packet(const u8 *cmd, const u8 *data) {
	hitagi_send_packet_aux(cmd, data, 0);
}

static void hitagi_send_bin_packet(const u8 *cmd, const u8 *data, u16 bin_size) {
	hitagi_send_packet_aux(cmd, data, bin_size);
}

static void hitagi_send_packet_aux(const u8 *cmd, const u8 *data, u16 bin_size) {
	u16 i;
	u16 j;
	u16 size;
	u8 *cmd_ptr;
	u8 *response;
	u8 empty_packet[] = { 0x00, 0x00 };

	i = 0;
	j = 0;
	size = bin_size;

	/* Save the original command on argument and set tx_data buffer as a response buffer. */
	cmd_ptr  = (u8 *) cmd;
	response = tx_data;

	/* Attach the starting control/transmition character first. */
	response[i++] = STX;

	/* Place command to the first answer section. */
	while ((*cmd_ptr != NUL) && (i < (USB_MAX_TX_DATA_SIZE - 16))) {
		response[i++] = *(cmd_ptr++);
	}

	/* Check if any data is present. */
	if (bin_size > 0) {
		/* Place separator character because data is present. */
		response[i++] = RS;

		/* Place data to the answer. */
		while ((size > 0) && (i < (USB_MAX_TX_DATA_SIZE - 16))) {
			response[i++] = *(data++);
			size--;
		}
	} else {
		if (data != NULL) {
			/* Place separator character because data is present. */
			response[i++] = RS;

			/* Place data to the answer. */
			while ((*data != NUL) && (i < (USB_MAX_TX_DATA_SIZE - 16))) {
				response[i++] = *(data++);
			}
		}
	}

	/* Place the ending control/transmition character and terminate string. */
	response[i++] = ETX;
	response[i] = NUL;

	/* Send full-fledged answer! */
	while (i > 0) {
		/* Big portion of Tx data. */
		if (i >= USB_MAX_PACKET_SIZE) {
			/* Send out Tx data and wait for it. */
			while (usb_tx(&(response[j]), USB_MAX_PACKET_SIZE) != RESULT_OK);

			/* Increment to the next data portion and decrement the data size. */
			j += USB_MAX_PACKET_SIZE;
			i -= USB_MAX_PACKET_SIZE;

			/* If we're sending data that is % USB_MAX_PACKET_SIZE, we must send an empty USB data packet. */
			if (i == 0) {
				while (usb_tx(&empty_packet[0], 0) != RESULT_OK);
			}
		} else {
			/* Small portion of Tx data. */
			while (usb_tx(&(response[j]), i) != RESULT_OK);

			i = 0;
		}
	}
}

static void hitagi_send_ack(const u8 *data) {
	/* ACK responce data. */
	u8 response[MAX_ACK_RESPONSE_SIZE];
	u8 *response_ptr;
	u8 *command_ptr;
	u8 *end_ptr;

	response_ptr = response;
	command_ptr = rx_command;
	end_ptr = &(response_ptr[MAX_ACK_RESPONSE_SIZE - 1]);

	/* Copy last command to ACK response. */
	while ((*command_ptr != NUL) && (response_ptr != end_ptr)) {
		*(response_ptr++) = *(command_ptr++);
	}

	/* If there is data, add a comma, then the data. */
	if (data) {
		*(response_ptr++) = com_str[0];

		/* Copy data into response. */
		while ((*data != NUL) && (response_ptr != end_ptr)) {
			*(response_ptr++) = *(data++);
		}
	}

	/* Terminate data. */
	*response_ptr = NUL;

	hitagi_send_packet(ack_str, response);
}

static void hitagi_send_error(u8 error_code) {
	u8 error_code_str[2];

	error_code_str[0] = error_code;
	error_code_str[1] = NUL;

	hitagi_send_packet(err_str, error_code_str);
}

static void hitagi_read_packets(void) {
	u8 i;
	u8 bytes_received;
	u8 previous_command_offset;
	u16 accumulated_bytes_received;
	u16 data_bytes_to_read;

	u8 data_array[USB_DATA_ARRAY_SIZE];

	u8 *input_ptr;
	u8 *command_ptr;
	u8 *current_ptr;
	u8 *buffer_next_byte;
	u8 *data_ptr;

	bytes_received = 0;
	previous_command_offset = 0;
	data_bytes_to_read = 0;

	input_ptr = data_array;
	command_ptr = rx_command;
	buffer_next_byte = NULL;

	watchdog_service();

	/* Forever! */
	while ("MotoFan.Ru is rock!") {
		/* Check if there is data coming in on EP1, add to any data left over from previous command. */
		bytes_received = usb_rx((u8 *) (input_ptr + previous_command_offset));

		if (bytes_received != 0) {
			/* Add the previous data to the count. */
			bytes_received += previous_command_offset;

			/* Throw out all data read until an STX is found. */
			current_ptr = input_ptr;
			while ((*(current_ptr++) != STX) && (bytes_received != 0)) {
				bytes_received--;
			}

			/* STX is present, process the rest of the message. */
			if (bytes_received != 0) {
				/* Decrement to throw out STX. */
				bytes_received--;

				/* Copy the remainder of the first read until RS or ETX is found. */
				while ((*current_ptr != ETX) && (*current_ptr != RS)) {
					*(command_ptr++) = *(current_ptr++);

					/* If out of data, then read more. */
					if (bytes_received-- == 0) {
						bytes_received = usb_rx(input_ptr);
						current_ptr = input_ptr;
					}
				}

				/* Done reading in command, terminate it! */
				*command_ptr = NUL;

				/* Check for separator and additional payload data. */
				if (*current_ptr == RS) {
					/* Set up RX data buffer. */
					data_ptr = rx_data;
					accumulated_bytes_received = 0;

					/* Skip RS and go into data. */
					current_ptr++;
					bytes_received--;

					/* Save all read bytes into data buffer. */
					while (bytes_received != 0) {
						bytes_received--;
						*(data_ptr++) = *(current_ptr++);
						accumulated_bytes_received++;
					}

					/* If this is a BIN command. */
					if (util_string_equal(bin_str, rx_command)) {
						/* Retrieve the minimum bytes required so we can get the size of the BIN data. */
						while (accumulated_bytes_received < MAX_DATA_FIELD_SIZE) {
							bytes_received += usb_rx(data_ptr);
							accumulated_bytes_received += bytes_received;
							data_ptr += bytes_received;
						}

						/* Determine the numbers of bytes to read. */
						data_bytes_to_read = ((rx_data[BIN_DATA_SIZE_MSB] << SHIFT_MSB) + (rx_data[BIN_DATA_SIZE_LSB]));

						/* Check for a valid data packet size. */
						if (
							(data_bytes_to_read < MIN_BIN_PACKET_SIZE) ||
							(data_bytes_to_read > MAX_BIN_PACKET_SIZE) ||
							(data_bytes_to_read % EVEN_NUMBER)
						) {
							hitagi_send_error(ERR_INVALID_PACKET_SIZE);
						} else {
							/* Adjust size of bytes to read, one byte for checksum, one byte for ETX. */
							/* And MAX_DATA_FIELD_SIZE for data count. */

							data_bytes_to_read += (MAX_DATA_FIELD_SIZE + 2);

							/* The next "free" location in the buffer is... */
							buffer_next_byte = data_ptr;

							/* Reset the data_ptr to the start of the buffer. */
							data_ptr = rx_data;
						}
					} else {
						/* Is not a BIN command but with DATA field also. */
						previous_command_offset = 0;

						data_ptr = rx_data;

						/* Scan for end of data. */
						while ((accumulated_bytes_received != 0) && (*data_ptr != ETX)) {
							data_ptr++;
							accumulated_bytes_received--;
						}

						/* Check if ETX was found. */
						if (accumulated_bytes_received != 0) {
							/* If ETX found set offset for next time around loop and skip ETX. */
							previous_command_offset = accumulated_bytes_received - 1;
							data_ptr++;
						} else {
							/* ETX not found yet, read more data. */
							while (bytes_received == 0) {
								watchdog_service();
								bytes_received = usb_rx(data_ptr);
							}
							while (*data_ptr != ETX) {
								data_ptr++;
								/* If out of data, read more again. */
								bytes_received--;
								if (!bytes_received) {
									bytes_received = usb_rx(data_ptr);
								}
								watchdog_service();
							}

							/* Set number of characters for next go-around, minus ETX. */
							if (bytes_received != 0) {
								previous_command_offset = bytes_received - 1;
							}

							/* Skip ETX character. */
							data_ptr++;
						}

						/* Copy any extra data back out for next command. */
						input_ptr = data_array;
						for (i = 0; i < previous_command_offset; ++i) {
							*(input_ptr++) = *(data_ptr++);
						}

						/* Add NULL-terminator to data and reset data pointer to beginning of rx_data array. */
						*data_ptr = NUL;
						data_ptr = rx_data;
					}
				} else {
					data_ptr = NULL;
				}

				hitagi_commands(rx_command, data_ptr, buffer_next_byte);

				/* End of command data, reset all pointers. */
				command_ptr = rx_command;
				data_ptr = NULL;
				input_ptr = data_array;
			}
		}

		watchdog_service();
	}
}

void hitagi_start(void) {
	usb_init();
	flash_init();
	watchdog_init();

	hitagi_read_packets();
}

void __attribute__((naked, section(".startup"))) _start(void) {
	asm volatile (
		"bl hitagi_start\n"
		"b .\n"
	);
}
