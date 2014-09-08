all:
	EXTRA_CFLAGS="-fPIC -w" make -f Makefile-gcc

clean:
	make -f Makefile-gcc clean
