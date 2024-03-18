
CC := clang
CFLAGS := -Wall -Wextra -Wpedantic -std=c11 -Wconversion
CFLAGS += -g -ggdb3

bdig: bdig.c
	$(CC) $(CFLAGS) -fsanitize=address $^ -o $@


