CFLAGS=-g -Wall
CC=gcc
SRCS=tokenizer.c token-shell.c
OBJS=tokenizer.o token-shell.o
LDFLAGS=
LIBS=

all:    token-shell penn-sh

$(SRCS):
	$(CC) $(CFLAGS) -c $*.c

token-shell: $(OBJS)
	$(CC) $(LDFLAGS) $(LIBS) -o token-shell $(OBJS)

penn-sh: tokenizer.o penn-sh.c
	clang -Wall -g $^ -o $@

clean:
	rm -f *.o token-shell penn-sh
