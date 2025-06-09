TARGET(elf32-bigarm)

MEMORY {
  IRAM (RWX) : ORIGIN = 0x03FC8014, LENGTH = 224k
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
