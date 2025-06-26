#ifndef FLASH_H
#define FLASH_H

#include "platform.h"

extern int flash_init(void);
extern int flash_unlock(volatile u16 *reg_addr_ctl);
extern int flash_erase(volatile u16 *reg_addr_ctl);
extern int flash_write_block(volatile u16 *reg_addr_ctl, volatile u16 *buffer, u32 size);
extern int flash_write_buffer(volatile u16 *reg_addr_ctl, const u16 *buffer, u32 size);
extern int flash_geometry(volatile u16 *reg_addr_ctl);
extern u32 flash_get_part_id(volatile u16 *reg_addr_ctl);

/**
 * Flash section.
 */

#if defined(FTR_FLASH_DATA_WIDTH_32BIT)
	#define FLASH_COMMAND(x) (((FLASH_DATA_WIDTH) (x) | ((FLASH_DATA_WIDTH) (x) << (sizeof(FLASH_DATA_WIDTH) << 2)))
#else
	#define FLASH_COMMAND(x) ((FLASH_DATA_WIDTH) (x))
#endif

#define BUFFER_SIZE_TO_WRITE(s)      FLASH_COMMAND((s) - 1)

#if defined(FTR_FLASH_DATA_WIDTH_32BIT)
	typedef u32 FLASH_DATA_WIDTH;
#else
	typedef u16 FLASH_DATA_WIDTH;
#endif

#define FLASH_START_ADDRESS            ((volatile FLASH_DATA_WIDTH *) 0x10000000)

#endif /* !FLASH_H */
