THREADING = 0
ASSEMBLY = 1
SHADERS = 0

AR = ar rcs
CC = gcc
OCP = objcopy

CFLAGS = -O3 -march=native -g2 -std=c1x -Wall -Wextra -pedantic -I../include -D_GNU_SOURCE
ifeq ($(THREADING), 1)
CFLAGS += -DTHREADING -pthread
endif
ifeq ($(ASSEMBLY), 1)
CFLAGS += -DUSE_ASSEMBLY -masm=intel
endif
ifeq ($(SHADERS), 1)
CFLAGS += -DUSE_SHADERS
endif

OCP_FROM_TEXT = -I binary -O elf64-x86-64 -B i386
RM = rm -f
ASM = fasm

# .c: S&S GL source (obviously)
# .glsl: Builtin shaders
# .hpp: Shader include files
# .cpp: Common shader "attachments"
# .asm: Assembly helpers
OBJECTS = $(patsubst %.c,%.o,$(wildcard *.c))

ifeq ($(THREADING), 0)
OBJECTS := $(subst threads.o,,$(OBJECTS))
endif

ifeq ($(ASSEMBLY), 1)
OBJECTS += $(patsubst %.asm,%-asm.o,$(wildcard *.asm))
endif

ifeq ($(SHADERS), 1)
OBJECTS += $(patsubst %.glsl,%.o,$(wildcard *.glsl)) $(patsubst %.hpp,%.o,$(wildcard *.hpp)) $(patsubst %.cpp,%.o,$(wildcard *.cpp))
else
OBJECTS := $(subst shader.o,,$(OBJECTS))
endif

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
	$(RM) *.o libSASGL.a
