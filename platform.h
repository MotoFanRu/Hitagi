#ifndef PLATFORM_H
#define PLATFORM_H

/**
 * Common stuff.
 */

#ifndef NULL
#define NULL                           ((void *) 0)
#endif

#ifndef RESULT_OK
#define RESULT_OK                      (0)
#endif

#ifndef RESULT_FAIL
#define RESULT_FAIL                    (1)
#endif

typedef char                           s8;
typedef unsigned char                  u8;
typedef short                          s16;
typedef unsigned short                 u16;
typedef int                            s32;
typedef unsigned int                   u32;
typedef long long                      s64;
typedef unsigned long long             u64;

/**
 * Base things.
 */

#define BIT_CLEAR                      (0)
#define BIT_SET                        (1)

#define UNUSED(x)                      ((void) x)

/**
 * Motorola Flash Protocol things.
 */

#define NUL                            (0x00)
#define STX                            (0x02)
#define ETX                            (0x03)
#define RS                             (0x1E)

#define MAX_DATA_FIELD_SIZE            (2)
#define MAX_COMMAND_STR_SIZE           (12)
#define MAX_ACK_RESPONSE_SIZE          (32)
#define MAX_RESP_DATA_SIZE             (64)

#define SHIFT_MSB                      (8)
#define BIN_DATA_SIZE_MSB              (0)
#define SHIFT_LSB                      (0)
#define BIN_DATA_SIZE_LSB              (1)

#define MIN_BIN_PACKET_SIZE            (8)
#define MAX_BIN_PACKET_SIZE            (8192)
#define MAX_READ_RESPONSE_SIZE         (0x500)
#define EVEN_NUMBER                    (2)

#define ERR_INVALID_PACKET_SIZE        (0x80 | 0x04)
#define ERR_UNKNOWN_COMMAND            (0x80 | 0x05)
#define ERR_DATA_INVALID               (0x80 | 0x0B)

#define CMD_32_SIZE                    (8)
#define CMD_16_SIZE                    (4)
#define CMD_8_SIZE                     (2)

typedef void (*HITAGI_CMD_HANDLER_T) (const u8 *data, const u8 *next);

typedef struct {
	const u8 *cmd;
	HITAGI_CMD_HANDLER_T cmd_func;
} HITAGI_CMD_TABLE_T;

typedef enum {
	ERASE_NO,
	ERASE_WRITE_BLOCK,
	ERASE_WRITE_BUFFER,
	ERASE_ONLY
} HITAGI_CMDLET_ERASE_T;

extern HITAGI_CMDLET_ERASE_T erase_cmdlet;

/**
 * Functions.
 */

extern int watchdog_service(void);

#endif /* !PLATFORM_H */
