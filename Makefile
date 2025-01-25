
CFLAGS=-nostdinc -pie -mbig-endian -march=armv4t -mtune=arm7tdmi-s -O0 -Wall -Wno-implicit-function-declaration
CFLAGS+=-fno-builtin

SOURCES=start.c string.c memcpy.c parse.c handle.c usb.c dump.c nor.c watchdog.c
OBJECTS=$(SOURCES:.c=.o)

NAME=LTE-Hitagi
BINARY=$(NAME).ldr
ELF=$(NAME).elf

all: $(BINARY)

.c.o:
	arm-none-eabi-gcc $(CFLAGS) -o $@ -c $<

$(ELF): $(OBJECTS)
	arm-none-eabi-ld --script link.lds $(OBJECTS) -o $@

$(BINARY): $(ELF)
	arm-none-eabi-objcopy -O binary $< $@

clean:
	rm -f $(BINARY) $(ELF) $(OBJECTS)
