CC=gcc
#CFLAGS=-Wall -ggdb
CFLAGS=-O3

all: pngscale

clean: test/clean
	rm -f pngscale $(PNGSCALE_OBJS)

PNGSCALE_OBJS = pngscale.o png_utils.o utils.o

pngscale: $(PNGSCALE_OBJS)
	$(CC) $(CFLAGS) $(PNGSCALE_OBJS) -o $@ -lpng

pngscale.o: pngscale.c
	$(CC) $(CFLAGS) -c $< -o $@

png_utils.o: png_utils.c
	$(CC) $(CFLAGS) -c $< -o $@

utils.o: utils.c
	$(CC) $(CFLAGS) -c $< -o $@

include test/Makefile.inc
