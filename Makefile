BIN = main precalculator

CXXFLAGS = -std=c++11 -O2 -Wall -g
CFLAGS = -std=c11 -O2 -Wall -g -D_GNU_SOURCE
LDFLAGS = -lm -ldl

# SDL
CFLAGS += `sdl2-config --cflags`
LDFLAGS += `sdl2-config --libs`

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
	game.o \
	raycaster.o \
	raycaster_data.o \
	renderer.o \
	raycaster_tables.o \
	main.o
deps := $(OBJS:%.o=.%.o.d)

precalculator.o: tools/precalculator.cpp
	$(VECHO) "  CXX\t$@\n"
	$(Q)$(CXX) -o $@ $(CXXFLAGS) -c -I . $<

precalculator: precalculator.o
	$(Q)$(CXX) -o $@ $^ $(LDFLAGS)

raycaster_tables.c: precalculator
	./precalculator > $@

%.o: %.c
	$(VECHO) "  C\t$@\n"
	$(Q)$(CC) -o $@ $(CFLAGS) -c -MMD -MF .$@.d $<

main: $(OBJS)
	$(Q)$(CC) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) $(BIN) $(OBJS) $(deps) raycaster_tables.c

-include $(deps)
