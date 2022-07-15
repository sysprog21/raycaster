BIN = main

CXXFLAGS = -std=c++11 -O2 -Wall -g

# SDL
CXXFLAGS += `sdl2-config --cflags`
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
	raycaster_fixed.o \
	raycaster_float.o \
	renderer.o \
	main.o
deps := $(OBJS:%.o=.%.o.d)

tools/%.o: tools/%.cpp
	$(VECHO) "  CXX\t$@\n"
	$(Q)$(CXX) -o $@ $(CXXFLAGS) -c -I . $<

precalculator: tools/precalculator.o
	$(Q)$(CXX) -o $@ $^ $(LDFLAGS)

raycaster_tables.h: precalculator
	./precalculator > $@

%.o: %.cpp raycaster_tables.h
	$(VECHO) "  CXX\t$@\n"
	$(Q)$(CXX) -o $@ $(CXXFLAGS) -c -MMD -MF .$@.d $<

$(BIN): $(OBJS)
	$(Q)$(CXX) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) $(BIN) $(OBJS) $(deps)

-include $(deps)
