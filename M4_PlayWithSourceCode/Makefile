# Define variable CC to be the compiler we want to use
CC = clang

# Define CFLAGS for the flags we will want to use with clang
CLANG = -g -Wall

TARGETS = clean alarm ctrlc_blocked_buggy ctrlc_blocked ctrlc ctrlc1 example functions

.PHONY:

# If no arguments are passed to make, it will attempt the 'penn-shredder' target
default: clean alarm ctrlc_blocked_buggy ctrlc_blocked ctrlc ctrlc1 example functions

# This runs the 'clean' and 'penn-shredder' targets when 'make all' is run
all: $(TARGETS)

# This will construct the binary 'penn-shredder'
# $^ = names of all the dependent files, deduped and with spaces
# $@ = complete name of the target
alarm: alarm.c
	clang -Wall -g $< -o $@

ctrlc_blocked_buggy: ctrlc_blocked_buggy.c
	clang -Wall -g $< -o $@

ctrlc_blocked: ctrlc_blocked.c
	clang -Wall -g $< -o $@

ctrlc: ctrlc.c
	clang -Wall -g $< -o $@	

ctrlc1: ctrlc1.c
	clang -Wall -g $< -o $@

example: example.c
	clang -Wall -g $< -o $@

functions: functions.c
	clang -Wall -g $< -o $@

# $(RM) is the platform agnostic way to delete a file (here rm -f)
clean:
	$(RM) alarm ctrlc_blocked_buggy ctrlc_blocked ctrlc ctrlc1 example functions

