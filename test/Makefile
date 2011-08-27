CC = gcc
LINK = gcc
RM = rm -f
CFLAGS = -O3 -g2 -std=c1x -Wall -Wextra -pedantic -I../include
LDFLAGS = -g2 -rdynamic -L../lib -lSASGL -lm -ldl

OBJECTS = $(patsubst %.c,%.o,$(wildcard *.c))

.PHONY: all clean

all: test

test: $(OBJECTS)
	$(LINK) $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJECTS) test
