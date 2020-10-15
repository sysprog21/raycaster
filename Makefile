BIN = main

CXXFLAGS = -std=c++11 -O2 -Wall -g

# SDL
CXXFLAGS += -I /usr/local/include/SDL2 # macOS
CXXFLAGS += -I /usr/include/SDL2 # GNU/Linux
LDFLAGS = -L /usr/local/lib/ -lSDL2

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

%.o: %.cpp
	$(VECHO) "  CXX\t$@\n"
	$(Q)$(CXX) -o $@ $(CXXFLAGS) -c -MMD -MF .$@.d $<

$(BIN): $(OBJS)
	$(Q)$(CXX)  -o $@ $^ $(LDFLAGS)

clean:
	$(RM) $(BIN) $(OBJS) $(deps)

-include $(deps)
