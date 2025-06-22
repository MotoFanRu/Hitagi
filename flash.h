#ifndef FLASH_H
#define FLASH_H

#include "platform.h"

extern int flash_init(void);
extern void flash_nop(u8 nop_count);
extern void flash_wait(volatile u16 *reg_addr_ctl);
extern void flash_switch_to_read_and_clean_regs(volatile u16 *reg_addr_ctl);
extern void flash_unlock(volatile u16 *reg_addr_ctl);
extern void flash_erase(volatile u16 *reg_addr_ctl);
extern void flash_write_word(volatile u16 *reg_addr_ctl, u16 word);
extern void flash_write_block(volatile u16 *reg_addr_ctl, volatile u16 *buffer, u32 size);
extern void flash_write_buffer(volatile u16 *reg_addr_ctl, const u16 *buffer, u32 size);
extern int flash_geometry(volatile u16 *reg_addr_ctl);

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

/**
 * Intel Flash chips.
 */

#define FLASH_INTEL_START_PARAMETER_BLOCKS   ((volatile FLASH_DATA_WIDTH *) 0x10000000)
#define FLASH_INTEL_END_PARAMETER_BLOCKS     ((volatile FLASH_DATA_WIDTH *) 0x10020000)

#define FLASH_INTEL_COMMAND_ERASE          FLASH_COMMAND(0x20)
#define FLASH_INTEL_COMMAND_WRITE          FLASH_COMMAND(0x40)
#define FLASH_INTEL_COMMAND_CLEAR          FLASH_COMMAND(0x50)
#define FLASH_INTEL_COMMAND_LOCK           FLASH_COMMAND(0x60)
#define FLASH_INTEL_STATUS_READY           FLASH_COMMAND(0x80)
#define FLASH_INTEL_COMMAND_WRITE_PROTECT  FLASH_COMMAND(0xC0)
#define FLASH_INTEL_COMMAND_CONFIRM        FLASH_COMMAND(0xD0)
#define FLASH_INTEL_COMMAND_WRITE_BUFFER   FLASH_COMMAND(0xE8)
#define FLASH_INTEL_COMMAND_READ           FLASH_COMMAND(0xFF)

#endif /* !FLASH_H */
