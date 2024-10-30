# Check the Stan model file name

ifndef PS_MODEL
$(error PS_MODEL is not set; try make PS_MODEL=your-model.stan)
endif

# Set paths etc

PS_POLYCHORD ?= $(abspath ./PolyChordLite)
PS_POLYCHORD_VERSION = $(shell grep 'version' $(PS_POLYCHORD)/src/polychord/feedback.f90 | grep -oe '[0-9]\+\.[0-9]\+\.[0-9]')
PS_POLYCHORD_LDLIBS = -L$(PS_POLYCHORD)/lib/ -lchord -Wl,-rpath $(PS_POLYCHORD)/lib/
PS_BS_ROOT ?= $(abspath ./bridgestan)
PS_STAN_FILE_NAME = $(abspath $(PS_MODEL))
PS_BASE = $(notdir $(basename $(PS_MODEL)))
PS_SRC = $(abspath ./src)
PS_HEADERS = $(wildcard $(PS_SRC)/polystan/*.hpp)
PS_BUILD = $(abspath ./build/)
PS_BUILD_MODEL = $(PS_BUILD)/$(PS_BASE)
PS_STAN_FUNCTIONS = $(abspath ./stanfunctions)
PS_MPI ?= 1
PS_STAN_FLAGS = --include-paths $(PS_STAN_FUNCTIONS) --warn-pedantic --warn-uninitialized --O1
PS_MACRO = -D PS_STAN_FILE_NAME=$(PS_STAN_FILE_NAME) -D PS_BASE=$(PS_BASE) -D PS_POLYCHORD_VERSION=$(PS_POLYCHORD_VERSION)

# Set compilers, flags and libs

BS_ROOT = $(PS_BS_ROOT)
$(info Including BridgeStan from $(BS_ROOT)/Makefile)
include $(BS_ROOT)/Makefile
override CXXFLAGS += -I$(PS_POLYCHORD)/src/ -I$(BS_ROOT)/..
override STANCFLAGS += $(PS_STAN_FLAGS)

ifdef PS_MPI
override LDLIBS +=-lmpi_cxx -lmpi
override CXXFLAGS += -DUSE_MPI
override CXX = mpic++
endif

.DEFAULT_GOAL := $(PS_BUILD_MODEL)/run

$(PS_MODEL):
	$(error $(PS_MODEL) does not exist)

$(PS_POLYCHORD)/lib/libchord.so:
	make -C $(PS_POLYCHORD)

$(PS_BUILD_MODEL):
	mkdir -p $(PS_BUILD_MODEL)

$(PS_BUILD_MODEL)/$(PS_BASE).hpp: $(PS_MODEL) $(STANC) $(PS_BUILD_MODEL)
	$(STANC) $(STANCFLAGS) --o=$@ $<

$(PS_BUILD_MODEL)/$(PS_BASE).o: $(PS_BUILD_MODEL)/$(PS_BASE).hpp
	$(COMPILE.cpp) -w -x c++ -o $@ $<

$(PS_BUILD_MODEL)/$(PS_BASE).so: $(PS_BUILD_MODEL)/$(PS_BASE).o $(BRIDGE_O) $(LIBSUNDIALS) $(MPI_TARGETS) $(TBB_TARGETS)
	$(LINK.cpp) -shared -lm -o $@ $< $(BRIDGE_O) $(LDLIBS) $(SUNDIALS_TARGETS) $(MPI_TARGETS) $(TBB_TARGETS)

$(PS_BUILD_MODEL)/polystan.o: $(PS_SRC)/polystan.cpp $(PS_HEADERS) $(PS_BUILD_MODEL)
	$(COMPILE.cpp) $(PS_MACRO) -I$(PS_SRC) $< -o $@

$(PS_BUILD_MODEL)/run: $(PS_BUILD_MODEL)/polystan.o $(PS_BUILD_MODEL)/$(PS_BASE).so $(PS_POLYCHORD)/lib/libchord.so
	$(LINK.cpp) -o $@ $< -Wl,-rpath ./ $(PS_BUILD_MODEL)/$(PS_BASE).so $(PS_POLYCHORD_LDLIBS) $(LDLIBS)
