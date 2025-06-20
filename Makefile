# Simple Makefile for ARM bare-metal project
# Assumes: linker.ld, startup.c, main.c (add more .c files as needed)

# Toolchain prefix (change if using a different toolchain)
CROSS_COMPILE ?= arm-none-eabi-

CC      := $(CROSS_COMPILE)gcc
LD      := $(CROSS_COMPILE)ld
OBJCOPY := $(CROSS_COMPILE)objcopy
OBJDUMP := $(CROSS_COMPILE)objdump
SIZE    := $(CROSS_COMPILE)size

# Source and objects
SRCS = hitagi.c
OBJS = $(SRCS:.c=.o)

# Output files
TARGET = hitagi
ELF    = $(TARGET).elf
BIN    = $(TARGET).ldr

# Flags
DEFINES = -DFTR_NEPTUNE_LTE1
CFLAGS  = $(DEFINES) 
CFLAGS += -Wall -Wextra -pedantic 
CFLAGS += -nostdlib -nostdinc 
CFLAGS += -O2 -marm -mbig-endian -march=armv4t -mtune=arm7tdmi-s
CFLAGS += -ffreestanding -fPIE
LDFLAGS = -pie -nostdlib
LIBS    = -T hitagi.ld -L. -lgcc_gba_m

.PHONY: all clean

all: $(BIN)

$(BIN): $(ELF)
	$(OBJCOPY) -O binary $< $@
	$(SIZE) $(ELF)
	$(OBJDUMP) -d $(ELF)

$(ELF): $(OBJS) hitagi.ld
	$(CC) -o $@ $(OBJS) $(LDFLAGS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(ELF) $(BIN) $(MAP)
