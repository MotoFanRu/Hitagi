TARGET(elf32-bigarm)

MEMORY {
  IRAM (RWX) : ORIGIN = 0x03FD0000, LENGTH = 192k
}

ENTRY(start)

SECTIONS {

  .data : {
    *(.startup)
    *(.data)
  } > IRAM

  .bss : {
    *(.bss)
  } > IRAM
}
