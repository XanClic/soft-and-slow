CC = gcc
LINK = gcc
RM = rm -f
CFLAGS = -rdynamic -O3 -Wall -Wextra -std=c1x -I../../include
LDFLAGS = -rdynamic -L../../lib -lSDL -lSASGL

.PHONY: all clean

all: gears

gears: gears.o
	$(LINK) $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) gears gears.o
