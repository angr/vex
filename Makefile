all:
	EXTRA_CFLAGS=-fPIC make -f Makefile-gcc

clean:
	make -f Makefile-gcc clean
