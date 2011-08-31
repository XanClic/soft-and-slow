AR = ar rcs
CC = gcc
OCP = objcopy
CFLAGS = -O3 -mmmx -msse -msse2 -mssse3 -g2 -std=c1x -Wall -Wextra -pedantic -I../include -D_GNU_SOURCE -DUSE_ASSEMBLY -masm=intel
OCP_FROM_TEXT = -I binary -O elf64-x86-64 -B i386
RM = rm -f
ASM = fasm

# .c: S&S GL source (obviously)
# .glsl: Builtin shaders
# .hpp: Shader include files
# .cpp: Common shader "attachments"
# .asm: Assembly helpers
OBJECTS = $(patsubst %.c,%.o,$(wildcard *.c)) $(patsubst %.glsl,%.o,$(wildcard *.glsl)) $(patsubst %.hpp,%.o,$(wildcard *.hpp)) $(patsubst %.cpp,%.o,$(wildcard *.cpp)) $(patsubst %.asm,%-asm.o,$(wildcard *.asm))

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

%.o: %.cpp
	$(OCP) $(OCP_FROM_TEXT) $< $@

%-asm.o: %.asm
	$(ASM) $< $@

clean:
	$(RM) $(OBJECTS) libSASGL.a
