# Define variable CC to be the compiler we want to use
CC = clang

# Define CFLAGS for the flags we will want to use with clang
CLANG = -g -Wall

TARGETS = clean pipes1 pipes2 redirections

.PHONY:

# If no arguments are passed to make, it will attempt the 'penn-shredder' target
default: pipes1 pipes2 redirections

# This runs the 'clean' and 'penn-shredder' targets when 'make all' is run
all: $(TARGETS)

# This will construct the binary 'penn-shredder'
# $^ = names of all the dependent files, deduped and with spaces
# $@ = complete name of the target
pipes1: pipes1.c
	clang -Wall -g $< -o $@

pipes2: pipes2.c
	clang -Wall -g $< -o $@

redirections: redirections.c
	clang -Wall -g $< -o $@

# $(RM) is the platform agnostic way to delete a file (here rm -f)
clean:
	$(RM) alarm pipes1 pipes2 redirections

