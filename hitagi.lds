/*
 * About:
 *   Template for the linker script for RAMDLDs for Motorola P2K phones on the various Neptune SoCs.
 *
 * Notes:
 *   %ORIGIN%: origin entry point address.
 *   %LENGTH%: memory chunk length size.
 *
 * Author:
 *   EXL, 29-Jun-2025
 */

MEMORY {
	PATCH (RWX) : ORIGIN = %ORIGIN%, LENGTH = %LENGTH%
}

/* Generate Big-Endian ARM ELFs. */
OUTPUT_FORMAT("elf32-bigarm")

/* Entry point function. */
ENTRY(_start)

/* Combine all relevant section to one. */
SECTIONS {
	.text : {
		*(.startup .startup.*)
		*(.text .text.*)
		*(.rodata .rodata.*)
		*(.data .data.*)
		*(.bss .bss.*)
		*(COMMON COMMON.*)
	} > PATCH
}
