BIN = kernel.elf precalculator

CC = arm-none-eabi-gcc
AS = arm-none-eabi-as

CXXFLAGS = -std=c++11 -O2 -Wall -g
CFLAGS = -mcpu=arm1176jzf-s -fpic -ffreestanding -std=gnu11 -O2 -Wall -Wextra
LDFLAGS = -T linker.ld -ffreestanding -O2 -nostdlib

# Control the build verbosity
ifeq ("$(VERBOSE)","1")
    Q :=
    VECHO = @true
else
    Q := @
    VECHO = @printf
endif

GIT_HOOKS := .git/hooks/applied
.PHONY: all clean

all: $(GIT_HOOKS) $(BIN)

$(GIT_HOOKS):
	@scripts/install-git-hooks
	@echo
	
OBJS := \
	boot.o \
	main.o \
	mmio_asm.o \
	uart.o \
	memory.o \
	game.o \
	raycaster.o \
	raycaster_fixed.o \
	raycaster_data.o \
	renderer.o \
	raycaster_tables.o

precalculator: tools/precalculator.cpp
	$(VECHO) "  CXX\t$@\n"
	$(Q)$(CXX) -o $@ $(CXXFLAGS) -I . $<

raycaster_tables.c: precalculator
	./precalculator > $@

%.o: %.c
	$(VECHO) "  C\t$@\n"
	$(Q)$(CC) -o $@ $(CFLAGS) -c $<

%.o: %.S
	$(Q)$(AS) -o $@ $(ASFLAGS) $<

kernel.elf: $(OBJS)
	$(Q)$(CC) -o $@ $^ $(LDFLAGS)

run: kernel.elf
	qemu-system-arm -m 512 -M raspi0 -serial stdio -kernel kernel.elf

clean:
	$(RM) $(BIN) $(OBJS) raycaster_tables.c

-include $(deps)
