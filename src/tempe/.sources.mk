CPP_SRCS += $(wildcard $(dir $(lastword $(MAKEFILE_LIST)))*.cpp)
CPP_SRCS += $(wildcard $(dir $(lastword $(MAKEFILE_LIST)))*/*.cpp)
