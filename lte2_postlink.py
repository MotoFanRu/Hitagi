#!/usr/bin/env python3

# Written using ChatGPT-4.1, GitHub Copilot
# https://github.com/copilot
# 06-Jun-2025

import sys
import os

def main(head_path, loader_path, sign_path, output_path):
	# Step 1: Read input files
	with open(head_path, 'rb') as f:
		head_data = f.read()
	with open(loader_path, 'rb') as f:
		loader_data = f.read()
	with open(sign_path, 'rb') as f:
		sign_data = f.read()

	# Step 2: Concatenate head.bin + loader.bin
	combined = head_data + loader_data

	# Step 3: Prepare output buffer filled with FFs up to offset 0x1F800
	SIGN_OFFSET = 0x1F800
	if len(combined) > SIGN_OFFSET:
		raise ValueError(f'Combined size ({len(combined)}) is greater than sign offset ({SIGN_OFFSET})')
	# Pad with 0xFF up to SIGN_OFFSET
	out_buf = bytearray(combined)
	out_buf.extend(b'\xFF' * (SIGN_OFFSET - len(out_buf)))

	# Step 4: Insert sign.bin at offset 0x1F800
	out_buf.extend(b'\xFF' * (SIGN_OFFSET + len(sign_data) - len(out_buf))) # ensure enough space
	out_buf[SIGN_OFFSET:SIGN_OFFSET+len(sign_data)] = sign_data

	# Step 5: Write to output file
	with open(output_path, 'wb') as f:
		f.write(out_buf)

if __name__ == '__main__':
	if len(sys.argv) != 5:
		print('Usage: python lte2_postlink.py lte2_head.bin lte2_loader.bin lte2_sign.bin output.ldr')
		sys.exit(1)
	head_path = sys.argv[1]
	loader_path = sys.argv[2]
	sign_path = sys.argv[3]
	output_path = sys.argv[4]
	main(head_path, loader_path, sign_path, output_path)
