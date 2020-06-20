CC=gcc
CFLAGS=-Wall -Werror=vla -Wextra -Wshadow -Wswitch-default
ASAN=-g -fsanitize=address
SEARCH=-I./include
OUT=-o ./bin/$@
COMPILE=$(CC) $(SEARCH) $(OUT) $^ $(CFLAGS) $(ASAN) -lpthread

all: zipper dezipper

zipper: src/compress.c
	$(COMPILE)

dezipper: src/decompress.c
	$(COMPILE)
