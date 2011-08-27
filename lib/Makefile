AR = ar rcs
CC = gcc
OCP = objcopy
CFLAGS = -O3 -g2 -std=c1x -Wall -Wextra -pedantic -I../include -D_GNU_SOURCE
OCP_FROM_TEXT = -I binary -O elf64-x86-64 -B i386
RM = rm -f

OBJECTS = $(patsubst %.c,%.o,$(wildcard *.c)) fixed-vertex-transformation.o fixed-fragment-transformation.o vertex-shader-include.o fragment-shader-include.o

.PHONY: all clean

all: libSASGL.a

libSASGL.a: $(OBJECTS)
	$(AR) $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.glsl
	$(OCP) $(OCP_FROM_TEXT) $< $@

%.o: %.hpp
	$(OCP) $(OCP_FROM_TEXT) $< $@

clean:
	$(RM) $(OBJECTS) libSASGL.a
