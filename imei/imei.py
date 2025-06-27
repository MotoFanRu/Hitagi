#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# drop first 0x08 byte (IMEI size).
data = [0x3A, 0x65, 0x78, 0x05, 0x10, 0x65, 0x82, 0x08]
digits = []

# skip the first nibble (filler: low nibble of first byte)
digits.append(str((data[0] & 0xF0) >> 4))  # high nibble of first byte

# bytes 1 to 6: both nibbles
for b in data[1:7]:
	digits.append(str(b & 0x0F))        # low nibble
	digits.append(str((b & 0xF0) >> 4)) # high nibble

# last byte: both nibbles (to complete 15 digits)
digits.append(str(data[7] & 0x0F))        # low nibble
digits.append(str((data[7] & 0xF0) >> 4)) # high nibble

imei = ''.join(digits[:15])
print(imei)  # prints: 356875001562880
