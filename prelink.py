#!/usr/bin/env python3

# Written using ChatGPT-4.1, GitHub Copilot
# https://github.com/copilot
# 29-Jun-2025

import sys

def main():
	if len(sys.argv) != 5:
		print('Usage: python prelink.py <template.lds> <output.ld> <ORIGIN> <LENGTH>')
		sys.exit(1)

	in_file  = sys.argv[1]
	out_file = sys.argv[2]
	origin   = sys.argv[3]
	length   = sys.argv[4]

	with open(in_file, 'r', encoding='utf-8') as f:
		content = f.read()

	content = content.replace('%ORIGIN%', origin)
	content = content.replace('%LENGTH%', length)

	with open(out_file, 'w', encoding='utf-8') as f:
		f.write(content)

	print(f'Patched {in_file} -> {out_file} with ORIGIN={origin}, LENGTH={length}')

if __name__ == '__main__':
	main()
