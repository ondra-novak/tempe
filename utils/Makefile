
all:	test tempe-cli

test:  test.cpp test.h
		g++ test.cpp -ggdb -O0 -ltempe -llightspeed -L../Debug -Ilightspeed/src  -I../src -Llightspeed/Debug -lpthread -o test
		
tempe-cli:  tempe-cli.cpp tempe-cli.h
		g++ tempe-cli.cpp -ggdb -O0 -ltempe -llightspeed -L../Debug -Ilightspeed/src  -I../src -Llightspeed/Debug -lpthread -o tempe-cli
		
clean: 
		rm -f test tempe-cli
