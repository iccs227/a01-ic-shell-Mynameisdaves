CC = gcc
CFLAGS = -g -w
BINARY = icsh

all: icsh

icsh: icsh.c
	$(CC) -o $(BINARY) $(CFLAGS) $<

.PHONY: clean

clean:
	rm -f $(BINARY)