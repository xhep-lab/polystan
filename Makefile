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

DEBUG ?= 0
ifeq ($(DEBUG), 0)
STAN_NO_RANGE_CHECKS ?= 1
STAN_CPP_OPTIMS ?= 1
TBB_CXXFLAGS ?= -w -Ofast -march=native -flto=auto
endif

-include $(BS_ROOT)/Makefile

# Set model-specific vars

PS_EXE := $(MAKECMDGOALS)$(EXE)
PS_STAN_FILE_NAME := $(abspath $(MAKECMDGOALS)).stan
PS_STAN_MODEL_NAME := $(notdir $(basename $(PS_STAN_FILE_NAME)))

# Preliminary checks

ifndef MAKECMDGOALS
$(error No Stan program set; try make examples/bernoulli)
endif

ifeq (,$(wildcard $(PS_STAN_FILE_NAME)))
$(warning Stan model $(PS_STAN_FILE_NAME) does not exist)
endif

# Set build flags & optimizations

override CXXFLAGS += -I$(PS_POLYCHORD)/src/ -I$(BS_ROOT)/.. -Wno-deprecated-declarations
override STANCFLAGS += --include-paths $(PS_STAN_FUNCTIONS)

MPI ?= $(shell mpirun 2> /dev/null && echo 1 || echo 0)
ifeq ($(MPI), 1)
override LDLIBS += -lmpi
override CXXFLAGS += -DUSE_MPI
override CXX = mpic++
endif

ifeq ($(DEBUG), 1)
override CXXFLAGS += -g
else
override CXXFLAGS += -Ofast -march=native -flto=auto
FFLAGS += -march=native -flto=auto
endif

export MPI FFLAGS DEBUG

# Define real targets

$(PS_POLYCHORD)/src:
	$(error Code $(PS_POLYCHORD) does not exist - try git submodule update --init --recursive)

$(BS_ROOT)/src:
	$(error Code $(BS_ROOT) does not exist - try git submodule update --init --recursive)

$(PS_POLYCHORD)/lib/libchord.so: $(PS_POLYCHORD)/src
	$(info Building PolyChord library)
	$(MAKE) -C $(PS_POLYCHORD)

$(PS_BUILD):
	mkdir -p $(PS_BUILD)

$(PS_BUILD)/$(PS_STAN_MODEL_NAME).hpp: $(PS_STAN_FILE_NAME) $(STANC) | $(PS_BUILD)
	$(info Transpiling model into C++)
	$(STANC) $(STANCFLAGS) --o=$@ $(PS_STAN_FILE_NAME)

$(PS_BUILD)/$(PS_STAN_MODEL_NAME).o: $(PS_BUILD)/$(PS_STAN_MODEL_NAME).hpp
	$(info Compiling model)
	$(COMPILE.cpp) -x c++ -o $@ $<

$(PS_BUILD)/polystan.o: $(PS_SRC)/polystan.cpp $(PS_HEADERS) $(PS_POLYCHORD)/lib/libchord.so | $(PS_BUILD)
	$(info Compiling PolyStan inferface)
	$(COMPILE.cpp) -D PS_POLYCHORD_VERSION=$(PS_POLYCHORD_VERSION) -I$(PS_SRC) $< -o $@

$(PS_BUILD)/$(PS_STAN_MODEL_NAME)_metadata.o: $(PS_SRC)/metadata.cpp $(PS_STAN_FILE_NAME)
	$(info Compiling metadata)
	$(COMPILE.cpp) -D PS_STAN_FILE_NAME=$(PS_STAN_FILE_NAME) -D PS_STAN_MODEL_NAME=$(PS_STAN_MODEL_NAME) -I$(PS_SRC) $< -o $@

%: %.stan $(BS_ROOT)/src $(PS_BUILD)/polystan.o $(PS_BUILD)/$(PS_STAN_MODEL_NAME).o $(PS_BUILD)/$(PS_STAN_MODEL_NAME)_metadata.o $(PS_POLYCHORD)/lib/libchord.so $(BRIDGE_O) $(TBB_TARGETS)
	$(info Building executable)
	$(LINK.cpp) -o $(PS_EXE) $(PS_BUILD)/polystan.o $(PS_BUILD)/$(PS_STAN_MODEL_NAME).o $(PS_BUILD)/$(PS_STAN_MODEL_NAME)_metadata.o $(BRIDGE_O) $(PS_POLYCHORD_LDLIBS) $(LDLIBS)

# Define phony targets

.PHONY: INFO
INFO:
	$(info Compiling BridgeStan and TBB dependencies)

.PHONY: clean-polystan
clean-polystan:
	$(RM) $(PS_BUILD)/*.o
	$(RM) $(PS_BUILD)/*.hpp

.PHONY: clean-polychord
clean-polychord:
	$(MAKE) -C $(PS_POLYCHORD) veryclean

clean: clean-polystan clean-polychord

.PHONY: format-polystan
format-polystan:
	clang-format -i src/*.cpp src/polystan/*.hpp || $(BS_FORMAT_IGNOREABLE)
	isort *.py || $(BS_FORMAT_IGNOREABLE)
	black *.py || $(BS_FORMAT_IGNOREABLE)

format: format-polystan

.PHONY: polystan-update
polystan-update:
	git submodule update --init --recursive

.PHONY: test-polystan
test-polystan:
	pytest ./test/