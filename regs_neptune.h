/*
 * About:
 *   Motorola/Freescale Neptune LTE-like SoCs (various) register definitions.
 *
 * Author:
 *   EXL, Motorola Inc.
 *
 * License:
 *   MIT
 *
 * Documentation:
 *  Neptune LTE IC Baseband Specification.pdf
 *  https://firmware.center/firmware/Motorola/E398%20%28Polo%29/Service%20Docs/Data%20Sheets/U800_Neptune%20LTE%20IC%20Baseband%20Specification.pdf
 */

#ifndef REGS_NEPTUNE_H
#define REGS_NEPTUNE_H

#include "platform.h"

/**
 * Watchdog section.
 */

/*
 * WATCHDOG_WCR: Watchdog Control Register, $2484_9000, 16-bit.
 *
 * Bits:
 * ; 00: WDZST, Watchdog Low Power.
 *       Determines the operation of the WDOG module during Low power modes. Write once-only.
 *           0 = Continue timer operation.
 *           1 = Suspend watchdog timer.
 * ; 01: WDBG, Watchdog DBUG Enable.
 *       Determines the operation of the WDOG module during DBUG mode. Write once-only.
 *           0 = Continue timer operation.
 *           1 = Suspend watchdog timer.
 * ; 02: WDE, Watchdog Enable.
 *       Enables or disables the WDOG module. enable once-only.
 *           0 = Disable Watchdog.
 *           1 = Enable Watchdog.
 * ; 03: WRE, Wdog_b or wdog_rst_b Enable.
 *       Determines if the watchdog will generate the reset signal or wdog_b upon a watchdog timeout or
 *       clock monitor event. Write once-only.
 *           0 = Generate a reset signal.
 *           1 = Generate wdog_b.
 * ; 04: SRS, Software Reset Signal.
 *       Controls the software assertion of the WDOG-generated reset signal.
 *       Note: This bit automatically resets to "1" after it has been asserted to "0".
 *           0 = Assert system reset signal.
 *           1 = No effect on system.
 * ; 05: WDA, Wdog_b Assertion.
 *       Controls the software assertion of the wdog_b signal.
 *           0 = Assert wdog_b output.
 *           1 = No effect on system.
 * ; 06: WOE, Wdog_b Output Enable.
 *       Determines if the WDOG pin is configured as an output or is input when GPIO gives WDOG ability to control
 *       this pin’s direction after reset.
 *           0 = Pin is tri-stated.
 *           1 = Pin configured as output.
 * ; 07: Reserved. These bits are reserved for future expansion.
 *       Always read as zero and must be written with zeros for future compatibility.
 * ; 08: Reserved. These bits are reserved for future expansion.
 *       Always read as zero and must be written with zeros for future compatibility.
 * ; 09: Timer value (0x3F).
 * ; 10: Timer value (0x3F).
 * ; 11: Timer value (0x3F).
 * ; 12: Timer value (0x3F).
 * ; 13: Timer value (0x3F).
 * ; 14: Timer value (0x3F).
 * ; 15: Timer value (0x3F).
 *   WT[6:0], Bits 15-9, Watchdog Timeout Field.
 *       This 7-bit field contains the time-out value and is loaded into the Watchdog counter after the service
 *       routine has been performed. After reset, WT[6:0] must be written before enabling the Watchdog.
 *           Set to desired timeout value.
 */

#define WATCHDOG_WCR (*(volatile u16 *) 0x24849000)

/*
 * WATCHDOG_WSR: Watchdog Service Register, $2484_9002, 16-bit.
 *
 *   WSR[15:0] Bits 15-0, Watchdog Service Register.
 *       This 15-bit field contains the watchdog service sequence.
 *       Note: Both writes must occur in the order listed prior to the time-out, but any number of instructions
 *       can be executed between the two writes.
 *
 *          The service sequence must be performed as follows:
 *              a. Write $5555 to the Watchdog Service Register (WSR).
 *              b. Write $AAAA to the Watchdog Service Register (WSR).
 */

#define WATCHDOG_WSR (*(volatile u16 *) 0x24849002)

/**
 * RTC (Real-time clock) section.
 */

/*
 *
 * RTC_PCRAM0:  RTC - PC RamRegister (RTCA), $2484_300C, 16 (rtc_pcram[1:15]), RTC_PCRAM0-RTC_PCRAM15, 32-bit.
 *
 *   PCRAM0[31:0]
 */

#define RTC_PCRAM0 (*(volatile u32 *) 0x2484300C)

/**
 * USB section.
 */

/*
 * USBINT: USB CPU Interrupt Register, $2485_2004, 16-bit.
 *
 * ; 00: RST_int, Reset Interrupt Flag.
 * ; 01: RES_int, Resume Interrupt Flag.
 * ; 02: SUSP_int, Suspend Interrupt Flag.
 * ; 03: EP0_RX_int, Endpoint 0 Receive Interrupt Flag.
 * ; 04: EP0_TX_int, Endpoint 0 Transmit Interrupt Flag.
 * ; 05: EP1_int, Endpoint 1 Interrupt Flag.
 * ; 06: EP2_int, Endpoint 2 Interrupt Flag.
 * ; 07: EP3_int, Endpoint 3 Interrupt Flag.
 * ; 08: EP4_int, Endpoint 4 Interrupt Flag.
 * ; 09: EP10_int, Endpoint 10 Interrupt Flag.
 * ; 10: Reserved.
 * ; 11: Reserved.
 * ; 12: Reserved.
 * ; 13: Reserved.
 * ; 14: Reserved.
 * ; 15: ERR, Error Flag.
 */

#define USB_INT (*(volatile u16 *) 0x24852004)

/*
 * USBE1CR: Endpoint 1 Control Register, $2485_200A, 16-bit.
 *
 * ; 00-05: B, Byte Count.
 * ; 06: ERRF, Error flag.
 * ; 07: OVF, Receive Buffer Overflow.
 * ; 08: SEQ, Sequence.
 * ; 09: EPEN, Endpoint enabled.
 * ; 10: XFREN, Transfer Enable.
 * ; 11: FSTALL, Force STALL Response.
 * ; 12: SET, Set SEQ and EPTYP1 bit.
 * ; 13: IEOFC, Interrupt, ERR, and Overflow Flag Clear.
 * ; 14-15: EPTYP, Endpoint Type.
 */

#define USB_E1_CR (*(volatile u16 *) 0x2485200A)

/*
 * USBE2CR: Endpoint 2 Control Register, $2485_200C, 16-bit.
 *
 * ; 00-05: B, Byte Count.
 * ; 06: ERRF, Error flag.
 * ; 07: OVF, Receive Buffer Overflow.
 * ; 08: SEQ, Sequence.
 * ; 09: EPEN, Endpoint enabled.
 * ; 10: XFREN, Transfer Enable.
 * ; 11: FSTALL, Force STALL Response.
 * ; 12: SET, Set SEQ and EPTYP1 bit.
 * ; 13: IEOFC, Interrupt, ERR, and Overflow Flag Clear.
 * ; 14-15: EPTYP, Endpoint Type.
 */

#define USB_E2_CR (*(volatile u16 *) 0x2485200C)

/*
 * USBCPUCR: USB CPU Control Register, $2485_2014, 16-bit.
 *
 * 00: MEMMAP, CPU Memory Map Select.
 * 01-03: Reserved.
 * 04-05: SPEED_SEL, USB Speed Select.
 * 06-15: Reserved.
 */

#define USB_CPU_CR (*(volatile u16 *) 0x24852014)

/*
 * USB_E1_RX_HW_BUFFER: Endpoint 1 RX Hardware Buffer, $2485_2090, 16/32-bit.
 *
 * Hardware Buffer size is programmed by MEMMAP bit on USB_CPU_CR register.
 */

#define USB_E1_RX_HW_BUFFER ((u16 *) 0x24852090)

/*
 * USB_E2_TX_HW_BUFFER: Endpoint 2 TX Hardware Buffer, $2485_20B0, 16/32-bit.
 *
 * Hardware Buffer size is programmed by MEMMAP bit on USB_CPU_CR register.
 */

#define USB_E2_TX_HW_BUFFER ((u16 *) 0x248520B0)

/*
 * USB_MAX_RX_DATA_SIZE: Max RX size.
 */

#define USB_MAX_RX_DATA_SIZE (8192 + 128)

/*
 * USB_MAX_TX_DATA_SIZE: Max TX size.
 */

#define USB_MAX_TX_DATA_SIZE (2048)

/*
 * USB_MAX_PACKET_SIZE: Max USB packet size.
 *
 * Can be 16 bytes or 32 bytes, see MEMMAP bit on USB_CPU_CR register.
 */

#if defined(FTR_NEPTUNE_LTE1)
#define USB_MAX_PACKET_SIZE (16)
#elif defined(FTR_NEPTUNE_LTE2)
#define USB_MAX_PACKET_SIZE (32)
#else
#error "Unknown Neptune SoC flavor!"
#endif

/*
 * USB_DATA_ARRAY_SIZE: Size of data buffer for USB engine.
 */

#define USB_DATA_ARRAY_SIZE (32)

/**
 * UID Section.
 */

/*
 * NEPTUNE_UID_REG: Unique Identifier Register, $2485_0000...$2485_0010, 16-bit.
 *
 * The Unique Identifier Register (UID) contains a 128-bit read-only field.
 * Each part is programmed with a unique security identification code during the manufacturing process.
 * Programming is done with laser fuses. Bits 127-124 are used as security module enables.
 * These signals are readable so that the software can determine the chip configuration.
 *
 * Note: These bits are read 1 when the associated module is enabled (ie the associated fuse is blown).
 * Bits 115-112 are used to indicate the usage of the chip to the software.
 *
 * Bits:
 *   UID[127]: iim_hacc_en (1 = enabled, fuse blown).
 *   UID[126]: iim_msu_en  (1 = enabled, fuse blown).
 *   UID[125]: iim_srom_en (1 = enabled, fuse blown). Reserved. Not used in Neptune LTE.
 *   UID[124]: iim_scc_en  (1 = enabled, fuse blown).
 *
 *   UID[115:112]:
 *     0001: Development Part (SE Neptune LTE).
 *     0010: Production Part (S Neptune LTE).
 *     1000: Non-Secure Part (NS Neptune LTE).
 */

#define NEPTUNE_UID_REG_ADDR ((volatile u16 *) 0x24850000)

/*
 * NEPTUNE_REV_REG: Hardware Revision Register, $2485_0010, 16-bit.
 *
 * Bits:
 *
 *   PR[4:0]: Bits 15-11. Product Revision. Specifies the Neptune product type (ROM, RAM, Production RAM, etc.).
 *     00000: Reserved.
 *     10001: Neptune LTS ROM.
 *     10010: Neptune LTE ROM.
 *     10011: Neptune ULS ROM.
 *     10100: Neptune LTE2 ROM.
 *     10101: Neptune LTE2 IROM0400 ROM (not sure).
 *     10111: Neptune LTE2 IROM0400 ROM (not sure).
 *
 *   PVT[2:0]: Bits 10-8. Product Vendor / Technology. Specifies the Neptune vendor and silicon technology
 *             (SPS HIP6W, SPS HIP7, SPS HIP8 etc.)
 *     000: Unknown.
 *     001: Unknown.
 *     010: SPS Hip7.
 *     011: SPS Hip8.
 *
 *   SR[7:0]: Bits 7-0. SIlicon Revision. Increments with each silicon change.
 *
 *     0000_0000: Initial Revision (Pass 1).
 *     0000_0001: Pass 2.
 *     ...
 */

#define NEPTUNE_REV_REG_ADDR ((volatile u16 *) 0x24850010)

#endif /* !REGS_NEPTUNE_H */
