# Check the Stan model file name

ifndef MAKECMDGOALS
$(error No Stan model set; try make examples/bernoulli)
endif

# Set PolyChord vars

PS_POLYCHORD ?= $(abspath ./PolyChordLite)
PS_POLYCHORD_VERSION = $(shell grep 'version' $(PS_POLYCHORD)/src/polychord/feedback.f90 | grep -oe '[0-9]\+\.[0-9]\+\.[0-9]')
PS_POLYCHORD_LDLIBS := -L$(PS_POLYCHORD)/lib/ -lchord -Wl,-rpath $(PS_POLYCHORD)/lib/

# Set PolyStan vars

PS_BUILD ?= $(abspath ./build)
PS_SRC := $(abspath ./src)
PS_HEADERS := $(wildcard $(PS_SRC)/polystan/*.hpp)
PS_STAN_FUNCTIONS := $(abspath ./stanfunctions)

# Include BridgeStan

BS_ROOT ?= $(abspath ./bridgestan)
-include $(BS_ROOT)/Makefile

# Set model-specific vars

PS_EXE := $(MAKECMDGOALS)
PS_STAN_FILE_NAME := $(abspath $(MAKECMDGOALS)).stan
PS_STAN_MODEL_NAME := $(notdir $(basename $(PS_STAN_FILE_NAME)))

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

$(PS_STAN_FILE_NAME):
	$(error Stan model $(PS_STAN_FILE_NAME) does not exist)

$(PS_POLYCHORD)/src:
	$(error Code $(PS_POLYCHORD) does not exist - try git submodule update --init --recursive)

$(BS_ROOT)/src:
	$(error Code $(BS_ROOT) does not exist - try git submodule update --init --recursive)

$(PS_POLYCHORD)/lib/libchord.so: $(PS_POLYCHORD)/src
	make -C $(PS_POLYCHORD)

$(PS_BUILD):
	mkdir -p $(PS_BUILD)

$(PS_BUILD)/$(PS_STAN_MODEL_NAME).hpp: $(STANC) $(PS_STAN_FILE_NAME) | $(PS_BUILD)
	$(STANC) $(STANCFLAGS) --o=$@ $(PS_STAN_FILE_NAME)

$(PS_BUILD)/$(PS_STAN_MODEL_NAME).o: $(PS_BUILD)/$(PS_STAN_MODEL_NAME).hpp
	$(COMPILE.cpp) -w -x c++ -o $@ $<

$(PS_BUILD)/polystan.o: $(PS_SRC)/polystan.cpp $(PS_POLYCHORD)/lib/libchord.so | $(PS_BUILD)
	$(COMPILE.cpp) -D PS_POLYCHORD_VERSION=$(PS_POLYCHORD_VERSION) -I$(PS_SRC) $< -o $@

$(PS_BUILD)/$(PS_STAN_MODEL_NAME)_metadata.o: $(PS_SRC)/metadata.cpp $(PS_STAN_FILE_NAME)
	$(COMPILE.cpp) -D PS_STAN_FILE_NAME=$(PS_STAN_FILE_NAME) -D PS_STAN_MODEL_NAME=$(PS_STAN_MODEL_NAME) -I$(PS_SRC) $< -o $@

$(PS_EXE): $(BS_ROOT)/src $(PS_BUILD)/polystan.o $(PS_BUILD)/$(PS_STAN_MODEL_NAME).o $(PS_BUILD)/$(PS_STAN_MODEL_NAME)_metadata.o $(PS_POLYCHORD)/lib/libchord.so $(BRIDGE_O) $(TBB_TARGETS)
	$(LINK.cpp) -o $@ $(PS_BUILD)/polystan.o $(PS_BUILD)/$(PS_STAN_MODEL_NAME).o $(PS_BUILD)/$(PS_STAN_MODEL_NAME)_metadata.o $(BRIDGE_O) $(PS_POLYCHORD_LDLIBS) $(LDLIBS)