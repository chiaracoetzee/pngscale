# Copyright (c) 2011 Derrick Coetzee, Guillaume Cottenceau, and contributors
# 
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
# 
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
# 
# Based on code distributed by Guillaume Cottenceau and contributors
# under MIT/X11 License at http://zarb.org/~gc/html/libpng.html

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
