all: test/test

test/clean:
	rm -f test/test $(TEST_OBJS)

TEST_OBJS = test/pngcompare.o test/test.o png_utils.o utils.o 

test/test: $(TEST_OBJS) pngscale
	$(CC) $(CFLAGS) $(TEST_OBJS) -o $@ -lpng

test/pngcompare.o: test/pngcompare.c
	$(CC) $(CFLAGS) -c $< -o $@

test/test.o: test/test.c
	$(CC) $(CFLAGS) -c $< -o $@
