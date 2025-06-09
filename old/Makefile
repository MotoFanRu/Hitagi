all:
	make -f Makefile.lte1
	rm -Rf *.elf *.o *.tmp
	make -f Makefile.lte2
	rm -Rf *.elf *.o *.tmp

clean:
	make -f Makefile.lte1 clean
	make -f Makefile.lte2 clean
