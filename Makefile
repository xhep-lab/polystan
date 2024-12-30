# Check the Stan model file name

ifndef MAKECMDGOALS
$(error No Stan file set; try make examples/bernoulli.stan)
endif

PS_STAN_FILE_NAME := $(abspath $(MAKECMDGOALS))

# Set PolyChord vars

PS_POLYCHORD ?= $(abspath ./PolyChordLite)
PS_POLYCHORD_VERSION := $(shell grep 'version' $(PS_POLYCHORD)/src/polychord/feedback.f90 | grep -oe '[0-9]\+\.[0-9]\+\.[0-9]')
PS_POLYCHORD_LDLIBS := -L$(PS_POLYCHORD)/lib/ -lchord -Wl,-rpath $(PS_POLYCHORD)/lib/

# Set model-specific vars

PS_EXE := $(basename $(PS_STAN_FILE_NAME))
PS_STAN_MODEL_NAME := $(notdir $(PS_EXE))

# Set PolyStan vars

TEMP_BUILD_DIR := $(shell mktemp -d)
PS_BUILD ?= $(TEMP_BUILD_DIR)
PS_SRC := $(abspath ./src)
PS_HEADERS := $(wildcard $(PS_SRC)/polystan/*.hpp)
PS_STAN_FUNCTIONS := $(abspath ./stanfunctions)

# Include BridgeStan

BS_ROOT ?= $(abspath ./bridgestan)
$(info Including BridgeStan from $(BS_ROOT)/Makefile)
include $(BS_ROOT)/Makefile

# Set build flags

PS_MPI ?= 1
PS_STAN_FLAGS := --include-paths $(PS_STAN_FUNCTIONS) --warn-pedantic --warn-uninitialized --O1

override CXXFLAGS += -I$(PS_POLYCHORD)/src/ -I$(BS_ROOT)/..
override STANCFLAGS += $(PS_STAN_FLAGS)

ifdef PS_MPI
override LDLIBS += -lmpi
override CXXFLAGS += -DUSE_MPI
override CXX = mpic++
endif

# Define targets

.PHONY: $(MAKECMDGOALS)

$(MAKECMDGOALS): $(PS_STAN_FILE_NAME) $(PS_EXE)
	$(info Built Stan model $(PS_STAN_FILE_NAME) to $(PS_EXE) in $(PS_BUILD))

$(PS_STAN_FILE_NAME):
	$(error Stan model $(PS_STAN_FILE_NAME) does not exist)

$(PS_POLYCHORD)/lib/libchord.so:
	make -C $(PS_POLYCHORD)

$(PS_BUILD)/$(PS_STAN_MODEL_NAME).hpp: $(STANC)
	$(STANC) $(STANCFLAGS) --o=$@ $(PS_STAN_FILE_NAME)

$(PS_BUILD)/$(PS_STAN_MODEL_NAME).o: $(PS_BUILD)/$(PS_STAN_MODEL_NAME).hpp
	$(COMPILE.cpp) -w -x c++ -o $@ $<

$(PS_SRC)/polystan.o: $(PS_SRC)/polystan.cpp $(PS_POLYCHORD)/lib/libchord.so
	$(COMPILE.cpp) -D PS_POLYCHORD_VERSION=$(PS_POLYCHORD_VERSION) -I$(PS_SRC) $< -o $@

$(PS_BUILD)/metadata.o: $(PS_SRC)/metadata.cpp
	$(COMPILE.cpp) -D PS_STAN_FILE_NAME=$(PS_STAN_FILE_NAME) -D PS_STAN_MODEL_NAME=$(PS_STAN_MODEL_NAME) -I$(PS_SRC) $< -o $@

$(PS_EXE): $(PS_SRC)/polystan.o $(PS_BUILD)/$(PS_STAN_MODEL_NAME).o $(PS_BUILD)/metadata.o $(PS_POLYCHORD)/lib/libchord.so $(BRIDGE_O)
	$(LINK.cpp) -o $@ $< $(PS_BUILD)/$(PS_STAN_MODEL_NAME).o $(PS_BUILD)/metadata.o $(BRIDGE_O) $(PS_POLYCHORD_LDLIBS) $(LDLIBS)