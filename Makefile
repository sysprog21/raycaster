CXXFLAGS = -std=c++11 -O2 -Wall -g
CFLAGS = `sdl2-config --cflags`
LDFLAGS = `sdl2-config --libs` -lm

CROSS_COMPILE ?= arm-none-eabi-
BAREMETAL_CC = $(CROSS_COMPILE)gcc
BAREMETAL_AS = $(CROSS_COMPILE)as
BAREMETAL_CFLAGS = -mcpu=arm1176jzf-s -fpic -ffreestanding -std=gnu11 -O2 -Wall -Wextra
BAREMETAL_LDFLAGS = -T linker.ld -ffreestanding -O2 -nostdlib -lgcc

# Control the build verbosity
ifeq ("$(VERBOSE)","1")
    Q :=
    VECHO = @true
else
    Q := @
    VECHO = @printf
endif

GIT_HOOKS := .git/hooks/applied
.PHONY: all clean sdl arm baremetal

all: $(GIT_HOOKS) raycaster_sdl

arm: $(GIT_HOOKS) raycaster_baremetal.elf

$(GIT_HOOKS):
	@scripts/install-git-hooks
	@echo

ARM_OBJS := \
	main_baremetal.o \
	uart.o \
	mem.o \
	string.o \
	stdlib.o \
	mailbox.o \
	timer.o \
	fb.o

SDL_OBJS := \
	main_sdl.o \
	game.o \
	raycaster.o \
	raycaster_fixed.o \
	raycaster_float.o \
	raycaster_data.o \
	renderer.o \
	raycaster_tables.o
BAREMETAL_OBJS := \
	boot.o \
	mmio_asm.o \
	game_baremetal.o \
	raycaster_baremetal.o \
	raycaster_fixed_baremetal.o \
	raycaster_data_baremetal.o \
	renderer_baremetal.o \
	raycaster_tables_baremetal.o

precalculator: tools/precalculator.cpp
	$(VECHO) "  CXX\t$@\n"
	$(Q)$(CXX) -o $@ $(CXXFLAGS) -I . $<

raycaster_tables.c: precalculator
	$(VECHO) "  Precompute\t$@\n"
	./precalculator > $@

$(ARM_OBJS): %.o: %.c
	$(VECHO) "  C\t$@\n"
	$(Q)$(BAREMETAL_CC) -o $@ $(BAREMETAL_CFLAGS) -c $<

%_baremetal.o: %.c
	$(VECHO) "  C\t$@\n"
	$(Q)$(BAREMETAL_CC) -o $@ $(BAREMETAL_CFLAGS) -c $<

%.o: %.c
	$(VECHO) "  C\t$@\n"
	$(Q)$(CC) -o $@ $(CFLAGS) -c $<

%.o: %.S
	$(VECHO) "  ASM\t$@\n"
	$(Q)$(BAREMETAL_AS) -o $@ $(BAREMETAL_ASFLAGS) $<

raycaster_sdl: $(SDL_OBJS)
	$(Q)$(CC) -o raycaster_sdl $^ $(LDFLAGS)

raycaster_baremetal.elf: $(ARM_OBJS) $(BAREMETAL_OBJS)
	$(Q)$(BAREMETAL_CC) -o raycaster_baremetal.elf $^ $(BAREMETAL_LDFLAGS)

sdl: raycaster_sdl
	./raycaster_sdl

baremetal: raycaster_baremetal.elf
	qemu-system-arm -M raspi0 -kernel raycaster_baremetal.elf -serial stdio

clean:
	$(RM) raycaster_sdl raycaster_baremetal.elf precalculator
	$(RM) $(ARM_OBJS) $(SDL_OBJS) $(BAREMETAL_OBJS) raycaster_tables.c
