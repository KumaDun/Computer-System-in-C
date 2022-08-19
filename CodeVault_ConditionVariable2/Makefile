# Define variable CC to be the compiler we want to use
CC = clang

# Define CFLAGS for the flags we will want to use with clang
CLANG = -g -Wall

TARGETS = clean condition_variable_broadcast

.PHONY: 

# If no arguments are passed to make, it will attempt the 'penn-shredder' target
default: condition_variable_broadcast

# This runs the 'clean' and 'penn-shredder' targets when 'make all' is run
all: $(TARGETS)

# This will construct the binary 'penn-shredder'
# $^ = names of all the dependent files, deduped and with spaces
# $@ = complete name of the target
condition_variable_broadcast: condition_variable_broadcast.c
	clang -Wall -g $< -o $@ -lpthread


# $(RM) is the platform agnostic way to delete a file (here rm -f)
clean:
	$(RM) condition_variable_broadcast