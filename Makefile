LIBNAME:=tempe
LIBINCLUDES:=src
SEARCHPATHS="./ ../ ../../ /usr/include/ /usr/local/include/"
CXX=clang++
CXXFLAGS +=-std=c++11

selectpath=$(abspath $(firstword $(foreach dir,$(1),$(wildcard $(dir)$(2)))))

LIBLIGHTSPEED=$(call selectpath,$(SEARCHPATHS),lightspeed)
NEEDLIBS:=$(LIBLIGHTSPEED)

include $(LIBLIGHTSPEED)/building/build_lib.mk


all: libtempe.a tempe-cli tempe-test

debug: libtempe.a tempe-cli tempe-test

tempe-cli: libtempe.a utils/tempe-cli.cpp utils/tempe-cli.h
		g++ utils/tempe-cli.cpp $(CXXFLAGS) -ltempe -llightspeed -L. -I$(LIBLIGHTSPEED)/src  -Isrc -L$(LIBLIGHTSPEED) -lpthread -o tempe-cli
		

tempe-test: libtempe.a utils/test.cpp utils/test.h
		g++ utils/test.cpp $(CXXFLAGS) -ltempe -llightspeed -L. -I$(LIBLIGHTSPEED)/src  -Isrc -L$(LIBLIGHTSPEED) -lpthread -o tempe-test
	