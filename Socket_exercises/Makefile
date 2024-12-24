# Find all .c files in the current directory
SRCS = $(wildcard *.c)

# Generate a list of executables from .c files
EXES = $(SRCS:.c=)

# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -g

# Build all executables
all: $(EXES)

# Create each executable from its .c file
%: %.c
	$(CC) $(CFLAGS) -o $@ $<

# Clean up all executables
clean:
	rm -f $(EXES)
