#include "../platform.h"
#include "../regs_neptune.h"

void HAPI_IC_ID_unique_id_read(u16 *dest) {
	dest[0] = *(NEPTUNE_UID_REG_ADDR + 7);
	dest[1] = *(NEPTUNE_UID_REG_ADDR + 6);
	dest[2] = *(NEPTUNE_UID_REG_ADDR + 5);
	dest[3] = *(NEPTUNE_UID_REG_ADDR + 4);
	dest[4] = *(NEPTUNE_UID_REG_ADDR + 3);
	dest[5] = *(NEPTUNE_UID_REG_ADDR + 2);
	dest[6] = *(NEPTUNE_UID_REG_ADDR + 1);
	dest[7] = *(NEPTUNE_UID_REG_ADDR + 0);
}

void encrypt_decrypt_data_with_uid(u16 *buff_ptr) {
	u16 uid[8];
	u8 index;

	HAPI_IC_ID_unique_id_read(uid);

	for (index = 0; index < 4; index++) {
		*buff_ptr++ ^= uid[index];
	}
}

static void hitagi_command_READ_IMEI(const u8 *answer_str, const u8 *data_ptr, const u8 *buffer_next_byte) {
	u16 index, src_index;
	u16 reg_iter;
	u8 response[MAX_READ_RESPONSE_SIZE];

	UNUSED(data_ptr);
	UNUSED(buffer_next_byte);

	reg_iter = 0;

	typedef union {
		u8 byte_array[4 * 2];
		u16 word_array[4];
	} PROT_REG_DATA_T;

	PROT_REG_DATA_T p;

	volatile u16 *otp_ptr = (volatile u16 *) 0x10240000;
	volatile u16 *user_otp_ptr = otp_ptr + (volatile u16) 0x85;

	*otp_ptr = (u16) 0x90;

	for(index = 0; index < 4; index++) {
		p.byte_array[reg_iter++] = (u8) ((*user_otp_ptr >> 8) & 0x00FF);
		p.byte_array[reg_iter++] = (u8) (*user_otp_ptr & 0x00FF);
		user_otp_ptr++;
	}

	*otp_ptr = (u16) 0xFF;

	encrypt_decrypt_data_with_uid(p.word_array);

	response[0] = 0x08;
	response[1] = 0x0A;

	for (index = 1, src_index = 0; src_index < 7; src_index++) {
		response[index] = (response[index] & 0x0F) | (p.byte_array[src_index] << 4);
		index++;
		response[index] = (response[index] & 0xF0) | (p.byte_array[src_index] >> 4);
	}

	/* Ensure that the 15th digit (checksum digit) is 0. */
	response[9 - 1] &= 0x0F;
	response[9] = 0;

	hitagi_send_packet(answer_str, response);
}
