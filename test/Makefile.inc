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

all: test/test

run_tests:
	test/test

test/clean:
	rm -f test/test $(TEST_OBJS)

TEST_OBJS = test/pngcompare.o test/test.o png_utils.o utils.o 

test/test: $(TEST_OBJS) pngscale
	$(CC) $(CFLAGS) $(TEST_OBJS) -o $@ -lpng

test/pngcompare.o: test/pngcompare.c
	$(CC) $(CFLAGS) -c $< -o $@

test/test.o: test/test.c
	$(CC) $(CFLAGS) -c $< -o $@
