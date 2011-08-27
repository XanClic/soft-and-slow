AR = ar rcs
CC = gcc
CFLAGS = -O3 -g2 -std=c1x -Wall -Wextra -pedantic -I../include -D_GNU_SOURCE
RM = rm -f

OBJECTS = $(patsubst %.c,%.o,$(wildcard *.c))

.PHONY: all clean

all: libSASGL.a

libSASGL.a: $(OBJECTS)
	$(AR) $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJECTS) libSASGL.a
