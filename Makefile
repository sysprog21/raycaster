BIN = main

CXXFLAGS = -std=c++11 -O2 -Wall -g
CFLAGS = -O2 -Wall -g
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
	raycaster_fixed.o \
	raycaster_float.o \
	raycaster_data.o \
	renderer.o \
	main.o
deps := $(OBJS:%.o=.%.o.d)

precalculator.o: tools/precalculator.cpp
	$(VECHO) "  CXX\t$@\n"
	$(Q)$(CXX) -o $@ $(CXXFLAGS) -c -I . $<

precalculator: precalculator.o
	$(Q)$(CXX) -o $@ $^ $(LDFLAGS)

raycaster_tables.h: precalculator
	./precalculator > $@

%.o: %.c raycaster_tables.h
	$(VECHO) "  CXX\t$@\n"
	$(Q)$(CC) -o $@ $(CFLAGS) -c -MMD -MF .$@.d $<

$(BIN): $(OBJS)
	$(Q)$(CC) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) raycaster_tables.h
	$(RM) $(BIN) $(OBJS) $(deps)

-include $(deps)
