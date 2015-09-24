################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/tempe/SourceReader.cpp \
../src/tempe/basicOps.cpp \
../src/tempe/compiler.cpp \
../src/tempe/dumpCode.cpp \
../src/tempe/eval.cpp \
../src/tempe/exceptions.cpp \
../src/tempe/fileCompiler.cpp \
../src/tempe/functionVar.cpp \
../src/tempe/functions.cpp \
../src/tempe/linkValue.cpp \
../src/tempe/tempeOps.cpp \
../src/tempe/varTable.cpp 

OBJS += \
./src/tempe/SourceReader.o \
./src/tempe/basicOps.o \
./src/tempe/compiler.o \
./src/tempe/dumpCode.o \
./src/tempe/eval.o \
./src/tempe/exceptions.o \
./src/tempe/fileCompiler.o \
./src/tempe/functionVar.o \
./src/tempe/functions.o \
./src/tempe/linkValue.o \
./src/tempe/tempeOps.o \
./src/tempe/varTable.o 

CPP_DEPS += \
./src/tempe/SourceReader.d \
./src/tempe/basicOps.d \
./src/tempe/compiler.d \
./src/tempe/dumpCode.d \
./src/tempe/eval.d \
./src/tempe/exceptions.d \
./src/tempe/fileCompiler.d \
./src/tempe/functionVar.d \
./src/tempe/functions.d \
./src/tempe/linkValue.d \
./src/tempe/tempeOps.d \
./src/tempe/varTable.d 


# Each subdirectory must supply rules for building sources it contributes
src/tempe/%.o: ../src/tempe/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	$(GPP) -I../../lightspeed/src -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


