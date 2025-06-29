# Simple Makefile for ARM bare-metal project
# Assumes: linker.ld, startup.c, main.c (add more .c files as needed)

# Toolchain prefix (change if using a different toolchain)
CROSS_COMPILE ?= arm-none-eabi-

CC      := $(CROSS_COMPILE)gcc
LD      := $(CROSS_COMPILE)ld
OBJCOPY := $(CROSS_COMPILE)objcopy
OBJDUMP := $(CROSS_COMPILE)objdump
SIZE    := $(CROSS_COMPILE)size

# Parameters
FLASH_TYPE ?= intel16
PLATFORM ?= LTE1

DEFINES_LTE1      = -DFTR_NEPTUNE_LTE1
ORIGIN_LTE1       = 0x03FD0010
LENGTH_LTE1       = 0x0002FFF0
SIGN_OFFSET_LTE1  = 0x0000F800

DEFINES_LTE2      = -DFTR_NEPTUNE_LTE2
ORIGIN_LTE2       = 0x03FC8014
LENGTH_LTE2       = 0x00037FEC
SIGN_OFFSET_LTE2  = 0x0000F800

DEFINES_LTE1C     = -DFTR_NEPTUNE_LTE1 -DFTR_COMPACT -Wno-unused-function
ORIGIN_LTE1C      = 0x03FD0010
LENGTH_LTE1C      = 0x0002FFF0
SIGN_OFFSET_LTE1C = 0x00001800

DEFINES_LTE2C     = -DFTR_NEPTUNE_LTE2 -DFTR_COMPACT -Wno-unused-function
ORIGIN_LTE2C      = 0x03FC8014
LENGTH_LTE2C      = 0x00037FEC
SIGN_OFFSET_LTE2C = 0x00001800

# Source and objects
SRCS  = hitagi.c
SRCS += flash_$(FLASH_TYPE).c
OBJS  = $(SRCS:.c=.o)

# Output files
TARGET = hitagi
ELF    = $(TARGET).elf
BIN    = $(TARGET).bin
LDR    = $(TARGET).ldr

# Flags
CFLAGS       = $(DEFINES_$(PLATFORM))
CFLAGS      += -Wall -Wextra -pedantic
CFLAGS      += -nostdlib -nostdinc
CFLAGS      += -O2 -marm -mbig-endian -march=armv4t -mtune=arm7tdmi-s
CFLAGS      += -ffreestanding -fPIE
LDFLAGS      = -pie -nostdlib
LDSCRIPT     = hitagi.ld
LIBS         = -T $(LDSCRIPT)

.PHONY: all clean

all: $(LDR)

$(LDR): $(BIN)
	python postlink.py bin/$(PLATFORM)_head.bin $< bin/$(PLATFORM)_sign.bin $(SIGN_OFFSET_$(PLATFORM)) $@

$(BIN): $(ELF)
	# $(OBJDUMP) -d $(ELF)
	$(SIZE) $(ELF)
	$(OBJCOPY) -O binary $< $@

$(LDSCRIPT): hitagi.lds
	python prelink.py hitagi.lds $(LDSCRIPT) $(ORIGIN_$(PLATFORM)) $(LENGTH_$(PLATFORM))

$(ELF): $(OBJS) $(LDSCRIPT)
	$(CC) -o $@ $(OBJS) $(LDFLAGS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(ELF) $(BIN) $(MAP) $(LDSCRIPT) $(LDR)
