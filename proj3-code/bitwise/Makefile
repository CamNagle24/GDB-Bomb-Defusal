CFLAGS = -Wall -Werror -g
LIBS = -lm
CC = gcc $(CFLAGS) $(LIBS)

.PHONY: all test clean test-setup zip

all: btest btest_server fshow ishow

btest: btest.c bits_impl.h sema.c utils.c bits.s
	$(CC) -o $@ $^

btest_server: btest_server.c sema.c utils.c bits_test.c
	$(CC) -m32 -o $@ $^

fshow: fshow.c
	$(CC) -m32 -o $@ $^

ishow: ishow.c
	$(CC) -o $@ $^

test-setup:
	@chmod u+x cc_check check_bitwise

test: test-setup all
	./check_bitwise

clean:
	rm -f fshow ishow btest btest_server

zip:
	@echo "Error: Do not run make zip in the bitwise subdirectory. Run it in the project's top-level directory."
