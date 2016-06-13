
LAMENES_SOURCES	= lamenes.cc\
		lame6502/lame6502.cc\
		romloader.cc\
		ppu.cc\
		input.cc\
		mario_nes.cc
NACL_SDK_ROOT ?= $(abspath $(HOME)/pepper)

# Project Build flags
WARNINGS := -Wno-long-long -Wall -pedantic
CXXFLAGS := -pthread -std=c++11 $(WARNINGS)

#
# Compute tool paths
#
GETOS := python $(NACL_SDK_ROOT)/tools/getos.py
OSHELPERS = python $(NACL_SDK_ROOT)/tools/oshelpers.py
OSNAME := $(shell $(GETOS))
RM := $(OSHELPERS) rm

PNACL_TC_PATH := $(abspath $(NACL_SDK_ROOT)/toolchain/$(OSNAME)_pnacl)
PNACL_CXX := $(PNACL_TC_PATH)/bin/pnacl-clang++
PNACL_FINALIZE := $(PNACL_TC_PATH)/bin/pnacl-finalize
CXXFLAGS := $(CXXFLAGS) -I$(NACL_SDK_ROOT)/include -I.
LDFLAGS := -L$(NACL_SDK_ROOT)/lib/pnacl/Release -lppapi_cpp -lppapi


# Declare the ALL target first, to make the 'all' target the default build
all: hello_tutorial.pexe

clean:
	$(RM) hello_tutorial.pexe hello_tutorial.bc

hello_tutorial.bc: hello_tutorial.cc $(LAMENES_SOURCES) 
	$(PNACL_CXX) -o $@ hello_tutorial.cc $(LAMENES_SOURCES) -O2 $(CXXFLAGS) $(LDFLAGS)

hello_tutorial.pexe: hello_tutorial.bc
	$(PNACL_FINALIZE) -o $@ $<

.PHONY: serve
serve: all
	python -m SimpleHTTPServer 8000
	
